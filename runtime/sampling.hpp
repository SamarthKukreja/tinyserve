#pragma once

#include "runtime/tokenizer.hpp"

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace tinyserve {

struct SamplingOptions {
  float temperature = 0.0F;
  std::size_t top_k = 0;
  std::uint64_t seed = 0;
};

TokenId greedy_argmax(const std::vector<float>& logits);
std::vector<float> sampling_probabilities(const std::vector<float>& logits, float temperature,
                                          std::size_t top_k = 0);

class Sampler {
 public:
  explicit Sampler(SamplingOptions options);
  TokenId select(const std::vector<float>& logits);

 private:
  SamplingOptions options_;
  std::mt19937_64 random_;
};

}  // namespace tinyserve
