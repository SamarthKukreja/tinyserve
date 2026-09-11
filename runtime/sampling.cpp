#include "runtime/sampling.hpp"

#include "runtime/attention.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace tinyserve {
namespace {

void validate_logits(const std::vector<float>& logits) {
  if (logits.empty()) {
    throw std::invalid_argument("sampling logits must not be empty");
  }
  if (!std::all_of(logits.begin(), logits.end(), [](float value) { return std::isfinite(value); })) {
    throw std::invalid_argument("sampling logits must all be finite");
  }
}

}  // namespace

TokenId greedy_argmax(const std::vector<float>& logits) {
  validate_logits(logits);
  const auto maximum = std::max_element(logits.begin(), logits.end());
  return static_cast<TokenId>(std::distance(logits.begin(), maximum));
}

std::vector<float> sampling_probabilities(const std::vector<float>& logits, float temperature,
                                          std::size_t top_k) {
  validate_logits(logits);
  if (!std::isfinite(temperature) || temperature <= 0.0F) {
    throw std::invalid_argument("sampling temperature must be positive and finite");
  }
  if (top_k > logits.size()) {
    throw std::invalid_argument("top_k cannot exceed the vocabulary size");
  }

  std::vector<std::size_t> candidates(logits.size());
  std::iota(candidates.begin(), candidates.end(), 0);
  std::stable_sort(candidates.begin(), candidates.end(), [&](std::size_t left, std::size_t right) {
    if (logits[left] == logits[right]) {
      return left < right;
    }
    return logits[left] > logits[right];
  });
  if (top_k != 0) {
    candidates.resize(top_k);
  }

  std::vector<float> selected_logits;
  selected_logits.reserve(candidates.size());
  for (const auto candidate : candidates) {
    selected_logits.push_back(logits[candidate] / temperature);
  }
  const auto selected_probabilities = stable_softmax(selected_logits);
  std::vector<float> probabilities(logits.size(), 0.0F);
  for (std::size_t index = 0; index < candidates.size(); ++index) {
    probabilities[candidates[index]] = selected_probabilities[index];
  }
  return probabilities;
}

Sampler::Sampler(SamplingOptions options) : options_(options), random_(options.seed) {
  if (!std::isfinite(options_.temperature) || options_.temperature < 0.0F) {
    throw std::invalid_argument("temperature must be finite and non-negative");
  }
}

TokenId Sampler::select(const std::vector<float>& logits) {
  if (options_.temperature == 0.0F) {
    return greedy_argmax(logits);
  }
  const auto probabilities =
      sampling_probabilities(logits, options_.temperature, options_.top_k);
  std::discrete_distribution<std::size_t> distribution(probabilities.begin(), probabilities.end());
  return static_cast<TokenId>(distribution(random_));
}

}  // namespace tinyserve
