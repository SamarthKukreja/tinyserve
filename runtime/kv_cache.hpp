#pragma once

#include "runtime/tensor.hpp"

#include <cstddef>
#include <vector>

namespace tinyserve {

// Contiguous single-sequence layout:
// [layer][position][kv_head][head_dimension], with separate key/value buffers.
class KvCache {
 public:
  KvCache(std::size_t num_layers, std::size_t num_kv_heads, std::size_t capacity,
          std::size_t head_dimension);

  std::size_t num_layers() const noexcept { return num_layers_; }
  std::size_t num_kv_heads() const noexcept { return num_kv_heads_; }
  std::size_t capacity() const noexcept { return capacity_; }
  std::size_t head_dimension() const noexcept { return head_dimension_; }
  std::size_t layer_size(std::size_t layer) const;
  std::size_t synchronized_size() const;
  std::size_t memory_bytes() const noexcept;

  void append(std::size_t layer, const Tensor& keys, const Tensor& values);
  Tensor keys(std::size_t layer) const;
  Tensor values(std::size_t layer) const;
  void reset() noexcept;

 private:
  std::size_t layer_offset(std::size_t layer, std::size_t position) const;
  Tensor read_layer(const std::vector<float>& storage, std::size_t layer) const;

  std::size_t num_layers_;
  std::size_t num_kv_heads_;
  std::size_t capacity_;
  std::size_t head_dimension_;
  std::size_t values_per_position_;
  std::vector<float> keys_;
  std::vector<float> values_;
  std::vector<std::size_t> layer_sizes_;
};

}  // namespace tinyserve
