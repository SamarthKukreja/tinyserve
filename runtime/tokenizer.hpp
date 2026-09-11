#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace tinyserve {

using TokenId = std::uint32_t;
using TokenIds = std::vector<TokenId>;

class Tokenizer {
 public:
  virtual ~Tokenizer() = default;
  virtual TokenIds encode(std::string_view text) const = 0;
  virtual std::string decode(const TokenIds& tokens) const = 0;
};

class ByteTokenizer final : public Tokenizer {
 public:
  TokenIds encode(std::string_view text) const override;
  std::string decode(const TokenIds& tokens) const override;
};

class TokenInput {
 public:
  static TokenInput from_text(std::string text);
  static TokenInput from_token_ids(TokenIds token_ids);

  bool contains_text() const noexcept { return contains_text_; }
  const std::string& text() const noexcept { return text_; }
  const TokenIds& token_ids() const noexcept { return token_ids_; }

 private:
  bool contains_text_ = false;
  std::string text_;
  TokenIds token_ids_;
};

TokenIds resolve_token_input(const TokenInput& input, const Tokenizer& tokenizer);

}  // namespace tinyserve
