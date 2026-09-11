#include "runtime/tokenizer.hpp"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

void require_invalid(const std::function<void()>& action) {
  try {
    action();
  } catch (const std::invalid_argument&) {
    return;
  }
  throw std::runtime_error("expected invalid_argument");
}

}  // namespace

int main() {
  try {
    const tinyserve::ByteTokenizer tokenizer;
    const std::string text = "TinyServe UTF-8: \xE2\x9C\x93";
    const auto encoded = tokenizer.encode(text);
    require(encoded.size() == text.size(), "byte tokenizer must emit one token per byte");
    require(tokenizer.decode(encoded) == text, "byte tokenizer roundtrip failed");

    const auto from_text = tinyserve::resolve_token_input(
        tinyserve::TokenInput::from_text("abc"), tokenizer);
    require(from_text == tinyserve::TokenIds({97, 98, 99}), "text input was not tokenized");

    const tinyserve::TokenIds raw = {7, 3, 7};
    const auto from_ids = tinyserve::resolve_token_input(
        tinyserve::TokenInput::from_token_ids(raw), tokenizer);
    require(from_ids == raw, "raw token IDs must pass through unchanged");

    require_invalid([&] { static_cast<void>(tokenizer.decode({256})); });

    std::cout << "TinyServe tokenizer tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TinyServe tokenizer tests failed: " << error.what() << '\n';
    return 1;
  }
}
