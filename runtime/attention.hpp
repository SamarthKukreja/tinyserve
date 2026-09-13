#pragma once

#include "runtime/tensor.hpp"

#include <vector>

namespace tinyserve {

std::vector<float> stable_softmax(const std::vector<float>& values);
void apply_causal_mask(Tensor& scores);

// q: [sequence, query_heads, head_dim]
// k/v: [sequence, kv_heads, head_dim]. query_heads must be divisible by kv_heads.
Tensor scaled_dot_product_attention(const Tensor& queries, const Tensor& keys,
                                    const Tensor& values);

// Incremental attention where queries start at an absolute cached position and
// keys/values contain the complete prefix through the final query token.
Tensor scaled_dot_product_attention_cached(const Tensor& queries, const Tensor& keys,
                                           const Tensor& values,
                                           std::size_t query_start_position);

}  // namespace tinyserve
