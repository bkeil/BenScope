#ifndef __BENSCOPE_PARSING_LEXER_H__
#define __BENSCOPE_PARSING_LEXER_H__

#include <cstdint>
#include <istream>
#include <string_view>

namespace benscope {

union TokenValue {
  std::string_view string_value;
  double double_value;
};

struct Token {
  static constexpr char kEof = 4;

  static constexpr char kDef = 1;
  static constexpr char kExtern = 16;

  static constexpr char kIf = 5;

  static constexpr char kIdentifier = 2;
  static constexpr char kNumber = 3;

  char type;
  TokenValue value;
};

class Lexer {
public:
  explicit Lexer(std::istream *input);

  void Debug();
  void Next();

  const Token &token() const { return token_; }

private:
  static bool IsDelimiter(char c);
  static bool IsWhitespace(char c);


  static constexpr char kNone = 5;
  
  char NextChar();
  void PutBack(char c);
  bool TokenIsNumeric();

  std::istream *input_;
  Token token_{Token::kEof, {0}};
  char lookahead_ = kNone;
  std::string identifier_;
};
} // namespace benscope

#endif // __BENSCOPE_PARSING_LEXER_H__