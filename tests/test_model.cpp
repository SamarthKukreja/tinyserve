#include "runtime/model.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
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

}  // namespace

int main(int argc, char** argv) {
  try {
    require(argc == 2, "expected fixture path argument");
    const std::filesystem::path fixture_path = argv[1];
    const auto model = tinyserve::Model::load(fixture_path);
    const tinyserve::TokenIds tokens = {65, 66};
    const auto first = model.forward(tokens);
    const auto second = model.forward(tokens);
    require(first.shape() == std::vector<std::size_t>({2, 256}),
            "full model logits shape is wrong");
    require(first.values() == second.values(), "fixture logits are not deterministic");
    require(std::all_of(first.values().begin(), first.values().end(),
                        [](float value) { return std::isfinite(value); }),
            "fixture logits contain non-finite values");
    require(model.next_token_logits(tokens).size() == 256, "next-token logits shape is wrong");

    bool bad_token_failed = false;
    try {
      static_cast<void>(model.forward({256}));
    } catch (const std::out_of_range&) {
      bad_token_failed = true;
    }
    require(bad_token_failed, "out-of-vocabulary token did not fail");

    std::ifstream input(fixture_path, std::ios::binary);
    std::vector<char> bytes((std::istreambuf_iterator<char>(input)),
                            std::istreambuf_iterator<char>());
    const std::string required_name = "model.norm.weight";
    const auto found = std::search(bytes.begin(), bytes.end(), required_name.begin(), required_name.end());
    require(found != bytes.end(), "could not locate required tensor name in fixture");
    *found = 'x';
    const auto missing_path = fixture_path.parent_path() / "missing-model.tserve";
    {
      std::ofstream output(missing_path, std::ios::binary | std::ios::trunc);
      output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }
    bool missing_failed = false;
    try {
      static_cast<void>(tinyserve::Model::load(missing_path));
    } catch (const std::out_of_range& error) {
      missing_failed = std::string(error.what()).find("model.norm.weight") != std::string::npos;
    }
    std::filesystem::remove(missing_path);
    require(missing_failed, "missing required tensor did not produce a clear error");

    std::cout << "TinyServe model tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TinyServe model tests failed: " << error.what() << '\n';
    return 1;
  }
}
