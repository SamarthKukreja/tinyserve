#include "runtime/attention.hpp"
#include "runtime/layers.hpp"
#include "runtime/model.hpp"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct ReferenceTensor {
  std::vector<std::size_t> shape;
  std::vector<float> values;
};

struct ReferenceFile {
  float absolute_tolerance = 0.0F;
  float relative_tolerance = 0.0F;
  std::map<std::string, ReferenceTensor> tensors;
};

std::vector<std::string> split(const std::string& value, char delimiter) {
  std::vector<std::string> parts;
  std::stringstream stream(value);
  std::string part;
  while (std::getline(stream, part, delimiter)) {
    parts.push_back(part);
  }
  return parts;
}

ReferenceFile load_reference(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input) {
    throw std::runtime_error("cannot open reference file: " + path.string());
  }
  ReferenceFile reference;
  std::string line;
  while (std::getline(input, line)) {
    const auto equals = line.find('=');
    if (equals == std::string::npos) {
      throw std::runtime_error("malformed reference line");
    }
    const auto key = line.substr(0, equals);
    const auto value = line.substr(equals + 1);
    if (key == "absolute_tolerance") {
      reference.absolute_tolerance = std::stof(value);
    } else if (key == "relative_tolerance") {
      reference.relative_tolerance = std::stof(value);
    } else if (key == "tensor") {
      const auto fields = split(value, '|');
      if (fields.size() != 3) {
        throw std::runtime_error("malformed tensor reference");
      }
      ReferenceTensor tensor;
      for (const auto& dimension : split(fields[1], ',')) {
        tensor.shape.push_back(static_cast<std::size_t>(std::stoull(dimension)));
      }
      for (const auto& number : split(fields[2], ',')) {
        tensor.values.push_back(std::stof(number));
      }
      reference.tensors.emplace(fields[0], std::move(tensor));
    }
  }
  if (!(reference.absolute_tolerance > 0.0F) || !(reference.relative_tolerance > 0.0F)) {
    throw std::runtime_error("reference tolerances are missing or invalid");
  }
  return reference;
}

void compare(const std::string& checkpoint, const tinyserve::Tensor& actual,
             const ReferenceFile& reference) {
  const auto found = reference.tensors.find(checkpoint);
  if (found == reference.tensors.end()) {
    throw std::runtime_error("missing reference checkpoint: " + checkpoint);
  }
  const auto& expected = found->second;
  if (actual.shape() != expected.shape) {
    throw std::runtime_error(checkpoint + ": shape mismatch");
  }
  float maximum_absolute_error = 0.0F;
  float maximum_relative_error = 0.0F;
  for (std::size_t index = 0; index < actual.size(); ++index) {
    const float expected_value = expected.values[index];
    const float actual_value = actual.values()[index];
    if (std::isinf(expected_value) || std::isinf(actual_value)) {
      if (!(std::isinf(expected_value) && std::isinf(actual_value) &&
            std::signbit(expected_value) == std::signbit(actual_value))) {
        throw std::runtime_error(checkpoint + ": infinity mismatch at index " +
                                 std::to_string(index));
      }
      continue;
    }
    const float absolute_error = std::fabs(actual_value - expected_value);
    const float relative_error = absolute_error / std::max(std::fabs(expected_value), 1.0e-12F);
    maximum_absolute_error = std::max(maximum_absolute_error, absolute_error);
    maximum_relative_error = std::max(maximum_relative_error, relative_error);
    const float allowance =
        reference.absolute_tolerance + reference.relative_tolerance * std::fabs(expected_value);
    if (!std::isfinite(actual_value) || absolute_error > allowance) {
      std::ostringstream message;
      message << checkpoint << ": mismatch at index " << index << ", expected "
              << expected_value << ", actual " << actual_value << ", absolute error "
              << absolute_error << ", allowance " << allowance;
      throw std::runtime_error(message.str());
    }
  }
  std::cout << checkpoint << ": max_abs=" << maximum_absolute_error
            << ", max_rel=" << maximum_relative_error << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  try {
    if (argc != 3) {
      throw std::runtime_error("usage: test_reference MODEL REFERENCE");
    }
    const auto reference = load_reference(argv[2]);

    const auto model = tinyserve::Model::load(argv[1]);
    const auto result = model.forward_with_intermediates({65, 66});
    compare("model.embeddings", result.embeddings, reference);
    if (result.layer_outputs.size() != 1) {
      throw std::runtime_error("fixture model must expose exactly one layer output");
    }
    compare("model.layer.0.output", result.layer_outputs[0], reference);
    compare("model.final_hidden", result.final_hidden, reference);
    const auto vocab = static_cast<std::size_t>(model.config().vocab_size);
    const auto begin = result.logits.values().end() - static_cast<std::ptrdiff_t>(vocab);
    compare("model.logits.last", tinyserve::Tensor({vocab}, std::vector<float>(begin, result.logits.values().end())), reference);

    const tinyserve::Tensor rms_input({2, 4}, {1.0F, -2.0F, 3.0F, -4.0F, 0.25F, 0.5F, -0.75F, 1.0F});
    const tinyserve::Tensor rms_weight({4}, {1.0F, 0.5F, 1.5F, 2.0F});
    compare("primitive.rmsnorm", tinyserve::rms_norm(rms_input, rms_weight, 1.0e-5F), reference);

    tinyserve::Tensor rope({2, 1, 4}, {1.0F, 2.0F, 3.0F, 4.0F, -1.0F, 0.5F, 2.0F, -3.0F});
    tinyserve::Tensor unused_keys = rope;
    tinyserve::apply_rope(rope, unused_keys, 3, 10000.0F);
    compare("primitive.rope", rope, reference);

    tinyserve::Tensor mask({3, 3}, {1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F});
    tinyserve::apply_causal_mask(mask);
    compare("primitive.causal_mask", mask, reference);

    const tinyserve::Tensor queries({2, 2, 2}, {1.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F, -1.0F, 0.5F});
    const tinyserve::Tensor keys({2, 1, 2}, {1.0F, 0.0F, 0.0F, 1.0F});
    const tinyserve::Tensor values({2, 1, 2}, {2.0F, -1.0F, 4.0F, 3.0F});
    compare("primitive.attention", tinyserve::scaled_dot_product_attention(queries, keys, values), reference);
    std::cout << "all fixture checkpoints are within documented tolerances\n";
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << "reference test failed: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
