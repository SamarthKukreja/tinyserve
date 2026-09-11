#pragma once

#include "runtime/model_config.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tinyserve {

struct TensorRecord {
  std::string name;
  DataType dtype = DataType::float32;
  std::vector<std::uint64_t> shape;
  std::vector<std::byte> data;

  std::uint64_t element_count() const;
  float float32_at(std::size_t index) const;
};

class WeightFile {
 public:
  static WeightFile load(const std::filesystem::path& path);

  const ModelConfig& config() const noexcept { return config_; }
  std::string_view config_text() const noexcept { return config_text_; }
  std::size_t tensor_count() const noexcept { return tensors_.size(); }
  bool contains(std::string_view name) const;
  const TensorRecord& tensor(std::string_view name) const;
  const TensorRecord& require_tensor(std::string_view name, const std::vector<std::uint64_t>& shape,
                                     DataType dtype) const;

 private:
  ModelConfig config_;
  std::string config_text_;
  std::unordered_map<std::string, TensorRecord> tensors_;
};

}  // namespace tinyserve
