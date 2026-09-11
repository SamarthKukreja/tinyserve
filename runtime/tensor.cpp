#include "runtime/tensor.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace tinyserve {

std::size_t tensor_element_count(const std::vector<std::size_t>& shape) {
  if (shape.empty()) {
    throw std::invalid_argument("tensor shape must have at least one dimension");
  }
  std::size_t count = 1;
  for (const auto dimension : shape) {
    if (dimension == 0) {
      throw std::invalid_argument("tensor dimensions must be positive");
    }
    if (count > std::numeric_limits<std::size_t>::max() / dimension) {
      throw std::overflow_error("tensor element count overflows size_t");
    }
    count *= dimension;
  }
  return count;
}

Tensor::Tensor(std::vector<std::size_t> shape, float fill)
    : shape_(std::move(shape)), values_(tensor_element_count(shape_), fill) {}

Tensor::Tensor(std::vector<std::size_t> shape, std::vector<float> values)
    : shape_(std::move(shape)), values_(std::move(values)) {
  const auto expected = tensor_element_count(shape_);
  if (values_.size() != expected) {
    throw std::invalid_argument("tensor value count does not match its shape");
  }
}

std::size_t Tensor::dimension(std::size_t axis) const {
  if (axis >= shape_.size()) {
    throw std::out_of_range("tensor axis is out of range");
  }
  return shape_[axis];
}

std::size_t Tensor::offset(const std::vector<std::size_t>& indices) const {
  if (indices.size() != shape_.size()) {
    throw std::invalid_argument("tensor index rank does not match tensor rank");
  }
  std::size_t result = 0;
  for (std::size_t axis = 0; axis < shape_.size(); ++axis) {
    if (indices[axis] >= shape_[axis]) {
      throw std::out_of_range("tensor index is out of bounds at axis " + std::to_string(axis));
    }
    result = result * shape_[axis] + indices[axis];
  }
  return result;
}

float& Tensor::at(const std::vector<std::size_t>& indices) { return values_.at(offset(indices)); }

const float& Tensor::at(const std::vector<std::size_t>& indices) const {
  return values_.at(offset(indices));
}

float& Tensor::at(std::initializer_list<std::size_t> indices) {
  return at(std::vector<std::size_t>(indices));
}

const float& Tensor::at(std::initializer_list<std::size_t> indices) const {
  return at(std::vector<std::size_t>(indices));
}

float dot_product(const std::vector<float>& left, const std::vector<float>& right) {
  if (left.size() != right.size()) {
    throw std::invalid_argument("dot product vectors must have the same length");
  }
  float result = 0.0F;
  for (std::size_t index = 0; index < left.size(); ++index) {
    result += left[index] * right[index];
  }
  return result;
}

std::vector<float> matrix_vector_multiply(const Tensor& matrix,
                                          const std::vector<float>& vector) {
  if (matrix.rank() != 2) {
    throw std::invalid_argument("matrix-vector multiply requires a rank-2 matrix");
  }
  const auto rows = matrix.dimension(0);
  const auto columns = matrix.dimension(1);
  if (vector.size() != columns) {
    throw std::invalid_argument("matrix-vector input length does not match matrix columns");
  }
  std::vector<float> result(rows, 0.0F);
  for (std::size_t row = 0; row < rows; ++row) {
    for (std::size_t column = 0; column < columns; ++column) {
      result[row] += matrix.at({row, column}) * vector[column];
    }
  }
  return result;
}

Tensor elementwise_add(const Tensor& left, const Tensor& right) {
  if (left.shape() != right.shape()) {
    throw std::invalid_argument("elementwise add requires equal tensor shapes");
  }
  Tensor output(left.shape());
  for (std::size_t index = 0; index < output.size(); ++index) {
    output.values()[index] = left.values()[index] + right.values()[index];
  }
  return output;
}

void elementwise_add_in_place(Tensor& target, const Tensor& addend) {
  if (target.shape() != addend.shape()) {
    throw std::invalid_argument("in-place add requires equal tensor shapes");
  }
  for (std::size_t index = 0; index < target.size(); ++index) {
    target.values()[index] += addend.values()[index];
  }
}

Tensor multiply_scalar(const Tensor& input, float scalar) {
  Tensor output(input.shape());
  for (std::size_t index = 0; index < output.size(); ++index) {
    output.values()[index] = input.values()[index] * scalar;
  }
  return output;
}

Tensor elementwise_multiply(const Tensor& left, const Tensor& right) {
  if (left.shape() != right.shape()) {
    throw std::invalid_argument("elementwise multiply requires equal tensor shapes");
  }
  Tensor output(left.shape());
  for (std::size_t index = 0; index < output.size(); ++index) {
    output.values()[index] = left.values()[index] * right.values()[index];
  }
  return output;
}

}  // namespace tinyserve
