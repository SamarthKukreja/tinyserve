#include "runtime/attention.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace tinyserve {

std::vector<float> stable_softmax(const std::vector<float>& values) {
  if (values.empty()) {
    throw std::invalid_argument("softmax input must not be empty");
  }
  const float maximum = *std::max_element(values.begin(), values.end());
  if (!std::isfinite(maximum)) {
    throw std::invalid_argument("softmax input must contain a finite maximum");
  }
  std::vector<float> output(values.size(), 0.0F);
  float denominator = 0.0F;
  for (std::size_t index = 0; index < values.size(); ++index) {
    if (values[index] == -std::numeric_limits<float>::infinity()) {
      continue;
    }
    if (!std::isfinite(values[index])) {
      throw std::invalid_argument("softmax input contains an unsupported non-finite value");
    }
    output[index] = std::exp(values[index] - maximum);
    denominator += output[index];
  }
  if (!(denominator > 0.0F) || !std::isfinite(denominator)) {
    throw std::runtime_error("softmax normalization denominator is invalid");
  }
  for (auto& value : output) {
    value /= denominator;
  }
  return output;
}

void apply_causal_mask(Tensor& scores) {
  if (scores.rank() < 2) {
    throw std::invalid_argument("causal mask requires a tensor with at least two dimensions");
  }
  const auto query_count = scores.dimension(scores.rank() - 2);
  const auto key_count = scores.dimension(scores.rank() - 1);
  const auto matrices = scores.size() / (query_count * key_count);
  const float masked = -std::numeric_limits<float>::infinity();
  for (std::size_t matrix = 0; matrix < matrices; ++matrix) {
    const auto base = matrix * query_count * key_count;
    for (std::size_t query = 0; query < query_count; ++query) {
      for (std::size_t key = query + 1; key < key_count; ++key) {
        scores.values()[base + query * key_count + key] = masked;
      }
    }
  }
}

Tensor scaled_dot_product_attention(const Tensor& queries, const Tensor& keys,
                                    const Tensor& values) {
  if (queries.rank() != 3 || keys.rank() != 3 || values.rank() != 3) {
    throw std::invalid_argument("attention expects rank-3 query, key, and value tensors");
  }
  const auto sequence = queries.dimension(0);
  const auto query_heads = queries.dimension(1);
  const auto head_dimension = queries.dimension(2);
  const auto kv_heads = keys.dimension(1);
  if (keys.dimension(0) != sequence || values.dimension(0) != sequence) {
    throw std::invalid_argument("attention query, key, and value sequence lengths must match");
  }
  if (keys.dimension(2) != head_dimension || values.dimension(2) != head_dimension ||
      values.dimension(1) != kv_heads) {
    throw std::invalid_argument("attention key/value head shapes are incompatible with queries");
  }
  if (query_heads % kv_heads != 0) {
    throw std::invalid_argument("attention query head count must be divisible by KV head count");
  }

  Tensor output({sequence, query_heads, head_dimension});
  const auto query_heads_per_kv_head = query_heads / kv_heads;
  const float scale = 1.0F / std::sqrt(static_cast<float>(head_dimension));
  for (std::size_t query_position = 0; query_position < sequence; ++query_position) {
    for (std::size_t query_head = 0; query_head < query_heads; ++query_head) {
      const auto kv_head = query_head / query_heads_per_kv_head;
      std::vector<float> scores(query_position + 1, 0.0F);
      for (std::size_t key_position = 0; key_position <= query_position; ++key_position) {
        for (std::size_t dimension = 0; dimension < head_dimension; ++dimension) {
          scores[key_position] += queries.at({query_position, query_head, dimension}) *
                                  keys.at({key_position, kv_head, dimension});
        }
        scores[key_position] *= scale;
      }
      const auto probabilities = stable_softmax(scores);
      for (std::size_t dimension = 0; dimension < head_dimension; ++dimension) {
        float weighted_value = 0.0F;
        for (std::size_t key_position = 0; key_position <= query_position; ++key_position) {
          weighted_value += probabilities[key_position] *
                            values.at({key_position, kv_head, dimension});
        }
        output.at({query_position, query_head, dimension}) = weighted_value;
      }
    }
  }
  return output;
}

}  // namespace tinyserve
