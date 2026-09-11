#include "runtime/model.hpp"

#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace tinyserve {
namespace {

std::vector<std::uint64_t> shape(std::initializer_list<std::uint64_t> dimensions) {
  return std::vector<std::uint64_t>(dimensions);
}

Tensor tensor_from_record(const TensorRecord& record) {
  if (record.dtype != DataType::float32) {
    throw std::runtime_error("CPU fixture runtime requires float32 tensor '" + record.name + "'");
  }
  std::vector<std::size_t> tensor_shape;
  tensor_shape.reserve(record.shape.size());
  for (const auto dimension : record.shape) {
    if (dimension > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
      throw std::runtime_error("tensor '" + record.name + "' dimension exceeds size_t");
    }
    tensor_shape.push_back(static_cast<std::size_t>(dimension));
  }
  std::vector<float> values;
  values.reserve(static_cast<std::size_t>(record.element_count()));
  for (std::size_t index = 0; index < record.element_count(); ++index) {
    values.push_back(record.float32_at(index));
  }
  return Tensor(std::move(tensor_shape), std::move(values));
}

Tensor required_tensor(const WeightFile& weights, const std::string& name,
                       const std::vector<std::uint64_t>& expected_shape) {
  return tensor_from_record(weights.require_tensor(name, expected_shape, DataType::float32));
}

std::optional<Tensor> optional_tensor(const WeightFile& weights, const std::string& name,
                                      const std::vector<std::uint64_t>& expected_shape) {
  if (!weights.contains(name)) {
    return std::nullopt;
  }
  return required_tensor(weights, name, expected_shape);
}

DecoderBlockWeights load_layer(const WeightFile& weights, std::size_t layer_index,
                               const ModelConfig& config) {
  const std::string prefix = "model.layers." + std::to_string(layer_index) + ".";
  const auto hidden = static_cast<std::uint64_t>(config.hidden_size);
  const auto intermediate = static_cast<std::uint64_t>(config.intermediate_size);
  const auto head_dimension = config.hidden_size / config.num_heads;
  const auto kv_width = static_cast<std::uint64_t>(config.num_kv_heads) * head_dimension;
  return DecoderBlockWeights{
      required_tensor(weights, prefix + "input_layernorm.weight", shape({hidden})),
      required_tensor(weights, prefix + "self_attn.q_proj.weight", shape({hidden, hidden})),
      required_tensor(weights, prefix + "self_attn.k_proj.weight", shape({kv_width, hidden})),
      required_tensor(weights, prefix + "self_attn.v_proj.weight", shape({kv_width, hidden})),
      optional_tensor(weights, prefix + "self_attn.q_proj.bias", shape({hidden})),
      optional_tensor(weights, prefix + "self_attn.k_proj.bias", shape({kv_width})),
      optional_tensor(weights, prefix + "self_attn.v_proj.bias", shape({kv_width})),
      required_tensor(weights, prefix + "self_attn.o_proj.weight", shape({hidden, hidden})),
      required_tensor(weights, prefix + "post_attention_layernorm.weight", shape({hidden})),
      required_tensor(weights, prefix + "mlp.gate_proj.weight", shape({intermediate, hidden})),
      required_tensor(weights, prefix + "mlp.up_proj.weight", shape({intermediate, hidden})),
      required_tensor(weights, prefix + "mlp.down_proj.weight", shape({hidden, intermediate})),
  };
}

}  // namespace

Model::Model(ModelConfig config, Tensor embeddings, std::vector<DecoderBlockWeights> layers,
             Tensor final_norm, Tensor lm_head)
    : config_(config),
      embeddings_(std::move(embeddings)),
      layers_(std::move(layers)),
      final_norm_(std::move(final_norm)),
      lm_head_(std::move(lm_head)) {}

Model Model::load(const std::filesystem::path& path) {
  return from_weights(WeightFile::load(path));
}

Model Model::from_weights(const WeightFile& weights) {
  const auto& config = weights.config();
  validate_model_config(config);
  if (config.dtype != DataType::float32) {
    throw std::runtime_error("CPU fixture model currently supports float32 weights only");
  }
  const auto vocab = static_cast<std::uint64_t>(config.vocab_size);
  const auto hidden = static_cast<std::uint64_t>(config.hidden_size);
  auto embeddings =
      required_tensor(weights, "model.embed_tokens.weight", shape({vocab, hidden}));
  std::vector<DecoderBlockWeights> layers;
  layers.reserve(config.num_layers);
  for (std::size_t layer = 0; layer < config.num_layers; ++layer) {
    layers.push_back(load_layer(weights, layer, config));
  }
  auto final_norm = required_tensor(weights, "model.norm.weight", shape({hidden}));
  auto lm_head = required_tensor(weights, "lm_head.weight", shape({vocab, hidden}));
  return Model(config, std::move(embeddings), std::move(layers), std::move(final_norm),
               std::move(lm_head));
}

Tensor Model::forward(const TokenIds& token_ids) const {
  if (token_ids.empty()) {
    throw std::invalid_argument("model forward requires at least one token ID");
  }
  if (token_ids.size() > config_.max_sequence_length) {
    throw std::out_of_range("token sequence exceeds max_sequence_length");
  }
  Tensor hidden({token_ids.size(), config_.hidden_size});
  for (std::size_t position = 0; position < token_ids.size(); ++position) {
    const auto token = token_ids[position];
    if (token >= config_.vocab_size) {
      throw std::out_of_range("token ID " + std::to_string(token) + " exceeds vocabulary size");
    }
    for (std::size_t dimension = 0; dimension < config_.hidden_size; ++dimension) {
      hidden.at({position, dimension}) = embeddings_.at({token, dimension});
    }
  }
  for (const auto& layer : layers_) {
    hidden = decoder_block(hidden, layer, config_);
  }
  return linear(rms_norm(hidden, final_norm_, config_.norm_epsilon), lm_head_);
}

std::vector<float> Model::next_token_logits(const TokenIds& token_ids) const {
  const auto logits = forward(token_ids);
  const auto vocab = static_cast<std::size_t>(config_.vocab_size);
  const auto offset = (token_ids.size() - 1) * vocab;
  return std::vector<float>(logits.values().begin() + static_cast<std::ptrdiff_t>(offset),
                            logits.values().begin() +
                                static_cast<std::ptrdiff_t>(offset + vocab));
}

}  // namespace tinyserve
