#pragma once

#include "runtime/layers.hpp"
#include "runtime/model_config.hpp"
#include "runtime/tensor.hpp"
#include "runtime/tokenizer.hpp"
#include "runtime/weight_loader.hpp"

#include <filesystem>
#include <vector>

namespace tinyserve {

struct ModelForwardResult {
  Tensor embeddings;
  std::vector<Tensor> layer_outputs;
  Tensor final_hidden;
  Tensor logits;
};

class Model {
 public:
  static Model load(const std::filesystem::path& path);
  static Model from_weights(const WeightFile& weights);

  const ModelConfig& config() const noexcept { return config_; }
  Tensor forward(const TokenIds& token_ids) const;
  ModelForwardResult forward_with_intermediates(const TokenIds& token_ids) const;
  std::vector<float> next_token_logits(const TokenIds& token_ids) const;

 private:
  Model(ModelConfig config, Tensor embeddings, std::vector<DecoderBlockWeights> layers,
        Tensor final_norm, Tensor lm_head);

  ModelConfig config_;
  Tensor embeddings_;
  std::vector<DecoderBlockWeights> layers_;
  Tensor final_norm_;
  Tensor lm_head_;
};

}  // namespace tinyserve
