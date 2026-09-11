#include "runtime/weight_loader.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

void require_failure(const std::function<void()>& action, const std::string& expected_text) {
  try {
    action();
  } catch (const std::exception& error) {
    require(std::string(error.what()).find(expected_text) != std::string::npos,
            "error did not contain '" + expected_text + "': " + error.what());
    return;
  }
  throw std::runtime_error("expected failure containing '" + expected_text + "'");
}

}  // namespace

int main(int argc, char** argv) {
  try {
    require(argc == 2, "expected the fixture path as one argument");
    const std::filesystem::path fixture_path = argv[1];
    const auto weights = tinyserve::WeightFile::load(fixture_path);

    require(weights.config().vocab_size == 256, "fixture config vocab size is wrong");
    require(weights.config().num_heads == 2 && weights.config().num_kv_heads == 1,
            "fixture config head counts are wrong");
    require(weights.tensor_count() == 12, "fixture tensor count is wrong");
    require(weights.contains("model.embed_tokens.weight"), "embedding tensor is missing");

    const auto& embedding = weights.require_tensor(
        "model.embed_tokens.weight", {256, 4}, tinyserve::DataType::float32);
    require(embedding.element_count() == 1024, "embedding element count is wrong");
    require(std::fabs(embedding.float32_at(0) - (-1.0F)) < 1.0e-7F,
            "embedding first value is wrong");
    require(std::fabs(embedding.float32_at(16) - 1.0F) < 1.0e-7F,
            "embedding deterministic sequence is wrong");

    require_failure([&] { static_cast<void>(weights.tensor("missing.weight")); },
                    "missing tensor");
    require_failure(
        [&] {
          static_cast<void>(weights.require_tensor("model.embed_tokens.weight", {4, 256},
                                                   tinyserve::DataType::float32));
        },
        "expected [4,256]");

    std::ifstream input(fixture_path, std::ios::binary);
    const std::vector<char> fixture_bytes((std::istreambuf_iterator<char>(input)),
                                          std::istreambuf_iterator<char>());
    require(fixture_bytes.size() > 16, "fixture is unexpectedly small");
    const auto malformed_path = fixture_path.parent_path() / "truncated.tserve";
    {
      std::ofstream output(malformed_path, std::ios::binary | std::ios::trunc);
      output.write(fixture_bytes.data(), static_cast<std::streamsize>(fixture_bytes.size() - 3));
    }
    require_failure([&] { static_cast<void>(tinyserve::WeightFile::load(malformed_path)); },
                    "truncated TinyServe weight file");
    std::filesystem::remove(malformed_path);

    std::cout << "TinyServe weight loader tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TinyServe weight loader tests failed: " << error.what() << '\n';
    return 1;
  }
}
