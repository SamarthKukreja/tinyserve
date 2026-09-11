#include "runtime/weight_loader.hpp"

#include <array>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace tinyserve {
namespace {

constexpr std::array<std::byte, 8> kMagic = {
    std::byte{'T'}, std::byte{'S'}, std::byte{'E'}, std::byte{'R'},
    std::byte{'V'}, std::byte{'E'}, std::byte{'0'}, std::byte{'1'}};
constexpr std::uint32_t kFormatVersion = 1;
constexpr std::uint32_t kMaxConfigBytes = 1024U * 1024U;
constexpr std::uint32_t kMaxTensorCount = 100000U;
constexpr std::uint32_t kMaxNameBytes = 4096U;
constexpr std::uint32_t kMaxRank = 16U;

class Reader {
 public:
  explicit Reader(std::vector<std::byte> bytes) : bytes_(std::move(bytes)) {}

  std::size_t remaining() const noexcept { return bytes_.size() - offset_; }

  std::vector<std::byte> read_bytes(std::size_t count, std::string_view context) {
    if (count > remaining()) {
      throw std::runtime_error("truncated TinyServe weight file while reading " +
                               std::string(context));
    }
    std::vector<std::byte> result(bytes_.begin() + static_cast<std::ptrdiff_t>(offset_),
                                  bytes_.begin() + static_cast<std::ptrdiff_t>(offset_ + count));
    offset_ += count;
    return result;
  }

  std::string read_string(std::size_t count, std::string_view context) {
    const auto bytes = read_bytes(count, context);
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
  }

  std::uint32_t read_u32(std::string_view context) {
    const auto bytes = read_bytes(4, context);
    return static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[0])) |
           (static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[1])) << 8U) |
           (static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[2])) << 16U) |
           (static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[3])) << 24U);
  }

  std::uint64_t read_u64(std::string_view context) {
    const auto bytes = read_bytes(8, context);
    std::uint64_t value = 0;
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      value |= static_cast<std::uint64_t>(std::to_integer<unsigned char>(bytes[index]))
               << (index * 8U);
    }
    return value;
  }

 private:
  std::vector<std::byte> bytes_;
  std::size_t offset_ = 0;
};

std::vector<std::byte> read_file(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary | std::ios::ate);
  if (!input) {
    throw std::runtime_error("cannot open TinyServe weight file: " + path.string());
  }
  const auto size = input.tellg();
  if (size < 0) {
    throw std::runtime_error("cannot determine TinyServe weight file size: " + path.string());
  }
  if (static_cast<unsigned long long>(size) >
      static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max())) {
    throw std::runtime_error("TinyServe weight file is too large for this platform");
  }
  std::vector<std::byte> bytes(static_cast<std::size_t>(size));
  input.seekg(0);
  if (!bytes.empty() &&
      !input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) {
    throw std::runtime_error("failed to read TinyServe weight file: " + path.string());
  }
  return bytes;
}

std::uint64_t checked_element_count(const std::vector<std::uint64_t>& shape) {
  std::uint64_t count = 1;
  for (const auto dimension : shape) {
    if (dimension == 0) {
      throw std::runtime_error("tensor dimensions must be positive");
    }
    if (count > std::numeric_limits<std::uint64_t>::max() / dimension) {
      throw std::runtime_error("tensor element count overflows uint64");
    }
    count *= dimension;
  }
  return count;
}

std::string shape_string(const std::vector<std::uint64_t>& shape) {
  std::ostringstream output;
  output << '[';
  for (std::size_t index = 0; index < shape.size(); ++index) {
    if (index != 0) {
      output << ',';
    }
    output << shape[index];
  }
  output << ']';
  return output.str();
}

}  // namespace

std::uint64_t TensorRecord::element_count() const { return checked_element_count(shape); }

float TensorRecord::float32_at(std::size_t index) const {
  if (dtype != DataType::float32) {
    throw std::logic_error("tensor '" + name + "' is not float32");
  }
  if (index >= element_count()) {
    throw std::out_of_range("float32 tensor index is out of range");
  }
  const std::size_t offset = index * sizeof(float);
  const std::uint32_t bits = static_cast<std::uint32_t>(std::to_integer<unsigned char>(data[offset])) |
                             (static_cast<std::uint32_t>(
                                  std::to_integer<unsigned char>(data[offset + 1]))
                              << 8U) |
                             (static_cast<std::uint32_t>(
                                  std::to_integer<unsigned char>(data[offset + 2]))
                              << 16U) |
                             (static_cast<std::uint32_t>(
                                  std::to_integer<unsigned char>(data[offset + 3]))
                              << 24U);
  float value = 0.0F;
  std::memcpy(&value, &bits, sizeof(value));
  return value;
}

WeightFile WeightFile::load(const std::filesystem::path& path) {
  Reader reader(read_file(path));
  if (reader.read_bytes(kMagic.size(), "file magic") !=
      std::vector<std::byte>(kMagic.begin(), kMagic.end())) {
    throw std::runtime_error("invalid TinyServe weight file magic");
  }
  const auto version = reader.read_u32("format version");
  if (version != kFormatVersion) {
    throw std::runtime_error("unsupported TinyServe weight format version " +
                             std::to_string(version));
  }
  const auto config_size = reader.read_u32("config length");
  const auto tensor_count = reader.read_u32("tensor count");
  if (config_size == 0 || config_size > kMaxConfigBytes) {
    throw std::runtime_error("TinyServe config length is invalid or exceeds the safety limit");
  }
  if (tensor_count > kMaxTensorCount) {
    throw std::runtime_error("TinyServe tensor count exceeds the safety limit");
  }

  WeightFile file;
  file.config_text_ = reader.read_string(config_size, "model config");
  file.config_ = parse_model_config(file.config_text_);
  file.tensors_.reserve(tensor_count);

  for (std::uint32_t tensor_index = 0; tensor_index < tensor_count; ++tensor_index) {
    const auto name_size = reader.read_u32("tensor name length");
    const auto dtype_code = reader.read_u32("tensor dtype");
    const auto rank = reader.read_u32("tensor rank");
    const auto body_size = reader.read_u64("tensor body length");
    if (name_size == 0 || name_size > kMaxNameBytes) {
      throw std::runtime_error("tensor name length is invalid or exceeds the safety limit");
    }
    if (rank == 0 || rank > kMaxRank) {
      throw std::runtime_error("tensor rank is invalid or exceeds the safety limit");
    }

    TensorRecord tensor;
    tensor.name = reader.read_string(name_size, "tensor name");
    try {
      tensor.dtype = static_cast<DataType>(dtype_code);
      static_cast<void>(data_type_size(tensor.dtype));
    } catch (const std::invalid_argument&) {
      throw std::runtime_error("tensor '" + tensor.name + "' has unsupported dtype code " +
                               std::to_string(dtype_code));
    }
    tensor.shape.reserve(rank);
    for (std::uint32_t dimension = 0; dimension < rank; ++dimension) {
      tensor.shape.push_back(reader.read_u64("tensor shape"));
    }
    const auto elements = checked_element_count(tensor.shape);
    const auto item_size = static_cast<std::uint64_t>(data_type_size(tensor.dtype));
    if (elements > std::numeric_limits<std::uint64_t>::max() / item_size ||
        body_size != elements * item_size) {
      throw std::runtime_error("tensor '" + tensor.name +
                               "' body length does not match its dtype and shape");
    }
    if (body_size > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
      throw std::runtime_error("tensor '" + tensor.name + "' is too large for this platform");
    }
    tensor.data = reader.read_bytes(static_cast<std::size_t>(body_size), "tensor body");
    const std::string name = tensor.name;
    if (!file.tensors_.emplace(name, std::move(tensor)).second) {
      throw std::runtime_error("duplicate tensor name '" + name + "'");
    }
  }
  if (reader.remaining() != 0) {
    throw std::runtime_error("TinyServe weight file contains trailing bytes");
  }
  return file;
}

bool WeightFile::contains(std::string_view name) const {
  return tensors_.find(std::string(name)) != tensors_.end();
}

const TensorRecord& WeightFile::tensor(std::string_view name) const {
  const auto found = tensors_.find(std::string(name));
  if (found == tensors_.end()) {
    throw std::out_of_range("missing tensor '" + std::string(name) + "'");
  }
  return found->second;
}

const TensorRecord& WeightFile::require_tensor(std::string_view name,
                                               const std::vector<std::uint64_t>& shape,
                                               DataType dtype) const {
  const auto& found = tensor(name);
  if (found.dtype != dtype) {
    throw std::runtime_error("tensor '" + std::string(name) + "' has dtype " +
                             std::string(data_type_name(found.dtype)) + ", expected " +
                             std::string(data_type_name(dtype)));
  }
  if (found.shape != shape) {
    throw std::runtime_error("tensor '" + std::string(name) + "' has shape " +
                             shape_string(found.shape) + ", expected " + shape_string(shape));
  }
  return found;
}

}  // namespace tinyserve
