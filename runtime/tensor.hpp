#pragma once

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace tinyserve {

class Tensor {
 public:
  explicit Tensor(std::vector<std::size_t> shape, float fill = 0.0F);
  Tensor(std::vector<std::size_t> shape, std::vector<float> values);

  std::size_t rank() const noexcept { return shape_.size(); }
  std::size_t size() const noexcept { return values_.size(); }
  const std::vector<std::size_t>& shape() const noexcept { return shape_; }
  std::size_t dimension(std::size_t axis) const;

  std::size_t offset(const std::vector<std::size_t>& indices) const;
  float& at(const std::vector<std::size_t>& indices);
  const float& at(const std::vector<std::size_t>& indices) const;
  float& at(std::initializer_list<std::size_t> indices);
  const float& at(std::initializer_list<std::size_t> indices) const;

  std::vector<float>& values() noexcept { return values_; }
  const std::vector<float>& values() const noexcept { return values_; }

 private:
  std::vector<std::size_t> shape_;
  std::vector<float> values_;
};

std::size_t tensor_element_count(const std::vector<std::size_t>& shape);
float dot_product(const std::vector<float>& left, const std::vector<float>& right);
std::vector<float> matrix_vector_multiply(const Tensor& matrix, const std::vector<float>& vector);
Tensor elementwise_add(const Tensor& left, const Tensor& right);
void elementwise_add_in_place(Tensor& target, const Tensor& addend);
Tensor multiply_scalar(const Tensor& input, float scalar);
Tensor elementwise_multiply(const Tensor& left, const Tensor& right);

}  // namespace tinyserve
