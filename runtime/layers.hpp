#pragma once

#include "runtime/model_config.hpp"
#include "runtime/kv_cache.hpp"
#include "runtime/tensor.hpp"

#include <cstddef>
#include <optional>

namespace tinyserve {

// Applies weights stored as [output_features, input_features] to the final input axis.
Tensor linear(const Tensor& input, const Tensor& weights, const Tensor* bias = nullptr);
Tensor rms_norm(const Tensor& input, const Tensor& weight, float epsilon);

// Queries and keys use [sequence, heads, head_dim]. RoPE uses split-half rotation.
void apply_rope(Tensor& queries, Tensor& keys, std::size_t start_position, float rope_theta);

float silu(float value);
Tensor swiglu(const Tensor& gate, const Tensor& up);
Tensor feed_forward(const Tensor& input, const Tensor& gate_weight, const Tensor& up_weight,
                    const Tensor& down_weight);
Tensor residual_add(const Tensor& residual, const Tensor& update);

struct DecoderBlockWeights {
  Tensor attention_norm;
  Tensor query_projection;
  Tensor key_projection;
  Tensor value_projection;
  std::optional<Tensor> query_bias;
  std::optional<Tensor> key_bias;
  std::optional<Tensor> value_bias;
  Tensor output_projection;
  Tensor post_attention_norm;
  Tensor gate_projection;
  Tensor up_projection;
  Tensor down_projection;
};

// Runs one pre-norm decoder block. It intentionally has no KV cache or full-model loop.
Tensor decoder_block(const Tensor& input, const DecoderBlockWeights& weights,
                     const ModelConfig& config, std::size_t start_position = 0);

// Runs an incremental single-token block and appends this layer's rotated K/V.
Tensor decoder_block_cached(const Tensor& input, const DecoderBlockWeights& weights,
                            const ModelConfig& config, KvCache& cache,
                            std::size_t layer_index, std::size_t position);

}  // namespace tinyserve
