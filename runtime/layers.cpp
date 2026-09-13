#include "runtime/layers.hpp"

#include "runtime/attention.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tinyserve {

Tensor linear(const Tensor& input, const Tensor& weights, const Tensor* bias) {
  if (weights.rank() != 2) {
    throw std::invalid_argument("linear weights must have shape [output_features, input_features]");
  }
  const auto input_features = input.dimension(input.rank() - 1);
  const auto output_features = weights.dimension(0);
  if (weights.dimension(1) != input_features) {
    throw std::invalid_argument("linear input width does not match weight input features");
  }
  if (bias != nullptr && (bias->rank() != 1 || bias->size() != output_features)) {
    throw std::invalid_argument("linear bias must match weight output features");
  }
  auto output_shape = input.shape();
  output_shape.back() = output_features;
  Tensor output(output_shape);
  const auto rows = input.size() / input_features;
  for (std::size_t row = 0; row < rows; ++row) {
    for (std::size_t output_feature = 0; output_feature < output_features; ++output_feature) {
      float sum = bias == nullptr ? 0.0F : bias->values()[output_feature];
      for (std::size_t input_feature = 0; input_feature < input_features; ++input_feature) {
        sum += input.values()[row * input_features + input_feature] *
               weights.at({output_feature, input_feature});
      }
      output.values()[row * output_features + output_feature] = sum;
    }
  }
  return output;
}

Tensor rms_norm(const Tensor& input, const Tensor& weight, float epsilon) {
  if (weight.rank() != 1 || weight.size() != input.dimension(input.rank() - 1)) {
    throw std::invalid_argument("RMSNorm weight must match the input's final dimension");
  }
  if (!std::isfinite(epsilon) || epsilon < 0.0F) {
    throw std::invalid_argument("RMSNorm epsilon must be finite and non-negative");
  }
  Tensor output(input.shape());
  const auto width = weight.size();
  const auto rows = input.size() / width;
  for (std::size_t row = 0; row < rows; ++row) {
    float sum_of_squares = 0.0F;
    for (std::size_t column = 0; column < width; ++column) {
      const float value = input.values()[row * width + column];
      sum_of_squares += value * value;
    }
    const float squared_rms = sum_of_squares / static_cast<float>(width) + epsilon;
    if (squared_rms == 0.0F) {
      continue;
    }
    const float inverse_rms = 1.0F / std::sqrt(squared_rms);
    for (std::size_t column = 0; column < width; ++column) {
      output.values()[row * width + column] =
          input.values()[row * width + column] * inverse_rms * weight.values()[column];
    }
  }
  return output;
}

void apply_rope(Tensor& queries, Tensor& keys, std::size_t start_position, float rope_theta) {
  if (queries.rank() != 3 || keys.rank() != 3) {
    throw std::invalid_argument("RoPE expects rank-3 query and key tensors");
  }
  if (queries.dimension(0) != keys.dimension(0) ||
      queries.dimension(2) != keys.dimension(2)) {
    throw std::invalid_argument("RoPE query and key sequence/head dimensions are incompatible");
  }
  const auto sequence = queries.dimension(0);
  const auto head_dimension = queries.dimension(2);
  if (head_dimension < 2 || head_dimension % 2 != 0) {
    throw std::invalid_argument("RoPE head dimension must be positive and even");
  }
  if (!std::isfinite(rope_theta) || rope_theta <= 0.0F) {
    throw std::invalid_argument("RoPE theta must be positive and finite");
  }
  if (sequence > std::numeric_limits<std::size_t>::max() - start_position) {
    throw std::overflow_error("RoPE absolute position overflows size_t");
  }

  const auto rotate = [&](Tensor& tensor) {
    const auto half_dimension = head_dimension / 2;
    for (std::size_t position = 0; position < sequence; ++position) {
      const auto absolute_position = start_position + position;
      for (std::size_t head = 0; head < tensor.dimension(1); ++head) {
        for (std::size_t pair = 0; pair < half_dimension; ++pair) {
          const float exponent = static_cast<float>(pair * 2) /
                                 static_cast<float>(head_dimension);
          const float inverse_frequency = 1.0F / std::pow(rope_theta, exponent);
          const float angle = static_cast<float>(absolute_position) * inverse_frequency;
          const float cosine = std::cos(angle);
          const float sine = std::sin(angle);
          const float first = tensor.at({position, head, pair});
          const float second = tensor.at({position, head, pair + half_dimension});
          tensor.at({position, head, pair}) = first * cosine - second * sine;
          tensor.at({position, head, pair + half_dimension}) = first * sine + second * cosine;
        }
      }
    }
  };
  rotate(queries);
  rotate(keys);
}

float silu(float value) { return value / (1.0F + std::exp(-value)); }

Tensor swiglu(const Tensor& gate, const Tensor& up) {
  if (gate.shape() != up.shape()) {
    throw std::invalid_argument("SwiGLU gate and up tensors must have equal shapes");
  }
  Tensor output(gate.shape());
  for (std::size_t index = 0; index < output.size(); ++index) {
    output.values()[index] = silu(gate.values()[index]) * up.values()[index];
  }
  return output;
}

Tensor feed_forward(const Tensor& input, const Tensor& gate_weight, const Tensor& up_weight,
                    const Tensor& down_weight) {
  const auto gate = linear(input, gate_weight);
  const auto up = linear(input, up_weight);
  return linear(swiglu(gate, up), down_weight);
}

Tensor residual_add(const Tensor& residual, const Tensor& update) {
  return elementwise_add(residual, update);
}

Tensor decoder_block(const Tensor& input, const DecoderBlockWeights& weights,
                     const ModelConfig& config, std::size_t start_position) {
  validate_model_config(config);
  if (input.rank() != 2 || input.dimension(1) != config.hidden_size) {
    throw std::invalid_argument("decoder block input must have shape [sequence, hidden_size]");
  }
  const auto sequence = input.dimension(0);
  if (start_position > config.max_sequence_length ||
      sequence > static_cast<std::size_t>(config.max_sequence_length) - start_position) {
    throw std::out_of_range("decoder block positions exceed max_sequence_length");
  }
  const auto head_dimension = config.hidden_size / config.num_heads;
  const auto kv_width = static_cast<std::size_t>(config.num_kv_heads) * head_dimension;

  const auto normalized = rms_norm(input, weights.attention_norm, config.norm_epsilon);
  const auto projected_queries =
      linear(normalized, weights.query_projection,
             weights.query_bias ? &weights.query_bias.value() : nullptr);
  const auto projected_keys =
      linear(normalized, weights.key_projection,
             weights.key_bias ? &weights.key_bias.value() : nullptr);
  const auto projected_values =
      linear(normalized, weights.value_projection,
             weights.value_bias ? &weights.value_bias.value() : nullptr);
  if (projected_queries.dimension(1) != config.hidden_size ||
      projected_keys.dimension(1) != kv_width || projected_values.dimension(1) != kv_width) {
    throw std::invalid_argument("decoder attention projection widths do not match model config");
  }

  Tensor queries({sequence, config.num_heads, head_dimension}, projected_queries.values());
  Tensor keys({sequence, config.num_kv_heads, head_dimension}, projected_keys.values());
  Tensor values({sequence, config.num_kv_heads, head_dimension}, projected_values.values());
  apply_rope(queries, keys, start_position, config.rope_theta);
  const auto attended = scaled_dot_product_attention(queries, keys, values);
  const Tensor attended_flat({sequence, config.hidden_size}, attended.values());
  const auto attention_update = linear(attended_flat, weights.output_projection);
  const auto after_attention = residual_add(input, attention_update);

  const auto post_attention =
      rms_norm(after_attention, weights.post_attention_norm, config.norm_epsilon);
  const auto mlp_update = feed_forward(post_attention, weights.gate_projection,
                                       weights.up_projection, weights.down_projection);
  return residual_add(after_attention, mlp_update);
}

Tensor decoder_block_cached(const Tensor& input, const DecoderBlockWeights& weights,
                            const ModelConfig& config, KvCache& cache,
                            std::size_t layer_index, std::size_t position) {
  validate_model_config(config);
  if (input.shape() != std::vector<std::size_t>({1, config.hidden_size})) {
    throw std::invalid_argument("cached decoder block input must have shape [1, hidden_size]");
  }
  if (position >= config.max_sequence_length || position >= cache.capacity()) {
    throw std::out_of_range("cached decoder position exceeds configured capacity");
  }
  if (cache.layer_size(layer_index) != position) {
    throw std::logic_error("cached decoder layer position does not match cache size");
  }
  const auto head_dimension = config.hidden_size / config.num_heads;
  const auto normalized = rms_norm(input, weights.attention_norm, config.norm_epsilon);
  const auto projected_queries =
      linear(normalized, weights.query_projection,
             weights.query_bias ? &weights.query_bias.value() : nullptr);
  const auto projected_keys =
      linear(normalized, weights.key_projection,
             weights.key_bias ? &weights.key_bias.value() : nullptr);
  const auto projected_values =
      linear(normalized, weights.value_projection,
             weights.value_bias ? &weights.value_bias.value() : nullptr);

  Tensor queries({1, config.num_heads, head_dimension}, projected_queries.values());
  Tensor keys({1, config.num_kv_heads, head_dimension}, projected_keys.values());
  Tensor values({1, config.num_kv_heads, head_dimension}, projected_values.values());
  apply_rope(queries, keys, position, config.rope_theta);
  cache.append(layer_index, keys, values);
  const auto attended = scaled_dot_product_attention_cached(
      queries, cache.keys(layer_index), cache.values(layer_index), position);
  const Tensor attended_flat({1, config.hidden_size}, attended.values());
  const auto after_attention =
      residual_add(input, linear(attended_flat, weights.output_projection));
  const auto post_attention =
      rms_norm(after_attention, weights.post_attention_norm, config.norm_epsilon);
  const auto mlp_update = feed_forward(post_attention, weights.gate_projection,
                                       weights.up_projection, weights.down_projection);
  return residual_add(after_attention, mlp_update);
}

}  // namespace tinyserve
