#include "runtime/kv_cache.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace tinyserve {
namespace {

std::size_t checked_multiply(std::size_t left, std::size_t right) {
  if (left != 0 && right > std::numeric_limits<std::size_t>::max() / left) {
    throw std::overflow_error("KV cache allocation size overflows size_t");
  }
  return left * right;
}

}  // namespace

KvCache::KvCache(std::size_t num_layers, std::size_t num_kv_heads, std::size_t capacity,
                 std::size_t head_dimension)
    : num_layers_(num_layers),
      num_kv_heads_(num_kv_heads),
      capacity_(capacity),
      head_dimension_(head_dimension),
      values_per_position_(checked_multiply(num_kv_heads, head_dimension)),
      layer_sizes_(num_layers, 0) {
  if (num_layers == 0 || num_kv_heads == 0 || capacity == 0 || head_dimension == 0) {
    throw std::invalid_argument("KV cache dimensions and capacity must be positive");
  }
  const auto values_per_layer = checked_multiply(capacity, values_per_position_);
  const auto total_values = checked_multiply(num_layers, values_per_layer);
  keys_.resize(total_values, 0.0F);
  values_.resize(total_values, 0.0F);
}

std::size_t KvCache::layer_size(std::size_t layer) const {
  if (layer >= num_layers_) {
    throw std::out_of_range("KV cache layer is out of range");
  }
  return layer_sizes_[layer];
}

std::size_t KvCache::synchronized_size() const {
  const auto size = layer_sizes_.front();
  if (!std::all_of(layer_sizes_.begin(), layer_sizes_.end(),
                   [size](std::size_t layer_size) { return layer_size == size; })) {
    throw std::logic_error("KV cache layers are not at the same sequence position");
  }
  return size;
}

std::size_t KvCache::memory_bytes() const noexcept {
  return (keys_.size() + values_.size()) * sizeof(float);
}

std::size_t KvCache::layer_offset(std::size_t layer, std::size_t position) const {
  return (layer * capacity_ + position) * values_per_position_;
}

void KvCache::append(std::size_t layer, const Tensor& keys, const Tensor& values) {
  if (layer >= num_layers_) {
    throw std::out_of_range("KV cache layer is out of range");
  }
  const std::vector<std::size_t> expected_shape = {1, num_kv_heads_, head_dimension_};
  if (keys.shape() != expected_shape || values.shape() != expected_shape) {
    throw std::invalid_argument("KV cache append expects key/value shape [1, kv_heads, head_dim]");
  }
  const auto position = layer_sizes_[layer];
  if (position >= capacity_) {
    throw std::out_of_range("KV cache capacity exceeded");
  }
  const auto offset = layer_offset(layer, position);
  std::copy(keys.values().begin(), keys.values().end(), keys_.begin() + static_cast<std::ptrdiff_t>(offset));
  std::copy(values.values().begin(), values.values().end(), values_.begin() + static_cast<std::ptrdiff_t>(offset));
  ++layer_sizes_[layer];
}

Tensor KvCache::read_layer(const std::vector<float>& storage, std::size_t layer) const {
  const auto size = layer_size(layer);
  if (size == 0) {
    throw std::logic_error("cannot retrieve an empty KV cache layer");
  }
  const auto begin = storage.begin() + static_cast<std::ptrdiff_t>(layer_offset(layer, 0));
  const auto count = size * values_per_position_;
  return Tensor({size, num_kv_heads_, head_dimension_},
                std::vector<float>(begin, begin + static_cast<std::ptrdiff_t>(count)));
}

Tensor KvCache::keys(std::size_t layer) const { return read_layer(keys_, layer); }

Tensor KvCache::values(std::size_t layer) const { return read_layer(values_, layer); }

void KvCache::reset() noexcept { std::fill(layer_sizes_.begin(), layer_sizes_.end(), 0); }

}  // namespace tinyserve
