#include "runtime/tokenizer.hpp"

#include <limits>
#include <stdexcept>
#include <utility>

namespace tinyserve {

TokenIds ByteTokenizer::encode(std::string_view text) const {
  TokenIds tokens;
  tokens.reserve(text.size());
  for (const unsigned char value : text) {
    tokens.push_back(static_cast<TokenId>(value));
  }
  return tokens;
}

std::string ByteTokenizer::decode(const TokenIds& tokens) const {
  std::string text;
  text.reserve(tokens.size());
  for (const auto token : tokens) {
    if (token > std::numeric_limits<unsigned char>::max()) {
      throw std::invalid_argument("byte tokenizer token ID must be in [0, 255]");
    }
    text.push_back(static_cast<char>(static_cast<unsigned char>(token)));
  }
  return text;
}

TokenInput TokenInput::from_text(std::string text) {
  TokenInput input;
  input.contains_text_ = true;
  input.text_ = std::move(text);
  return input;
}

TokenInput TokenInput::from_token_ids(TokenIds token_ids) {
  TokenInput input;
  input.token_ids_ = std::move(token_ids);
  return input;
}

TokenIds resolve_token_input(const TokenInput& input, const Tokenizer& tokenizer) {
  if (input.contains_text()) {
    return tokenizer.encode(input.text());
  }
  return input.token_ids();
}

}  // namespace tinyserve
