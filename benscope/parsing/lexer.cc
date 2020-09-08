#include "benscope/parsing/lexer.h"

#include <cassert>
#include <cctype>
#include <iostream>
#include <string>

namespace benscope {

Lexer::Lexer(std::istream *input) : input_(input) { Next(); }

// static
bool Lexer::IsWhitespace(char c) {
  return c == ' ' || (c > '\010' && c < '\016');
}

// static
bool Lexer::IsDelimiter(char c) {
  return c == '(' || c == ')' || c == '\'' || c == '"' || c == '`' ||
         c == ';' || c == '|' || c == '[' || c == ']' || c == '{' || c == '}' ||
         IsWhitespace(c) || c == Token::kEof;
}

void Lexer::Next() {
  char c;
  do {
    c = NextChar();
  } while (IsWhitespace(c));

  if (IsDelimiter(c)) {
    token_.type = c;
    return;
  }

  identifier_.clear();
  do {
    identifier_.push_back(c);
    c = NextChar();
  } while (!IsDelimiter(c));
  PutBack(c);

  if (TokenIsNumeric()) {
    token_.type = Token::kNumber;
    token_.value.double_value = std::strtod(identifier_.c_str(), nullptr);
  } else if (identifier_ == "def") {
    token_.type = Token::kDef;
  } else if (identifier_ == "extern") {
    token_.type = Token::kExtern;
  } else if (identifier_ == "if") {
    token_.type = Token::kIf;
  } else if (identifier_.size() == 1 && !std::isalnum(identifier_[0])) {
    token_.type = identifier_[0];
  } else {
    token_.type = Token::kIdentifier;
    token_.value.string_value = identifier_;
  }
}

char Lexer::NextChar() {
  if (lookahead_ != kNone) {
    char c = lookahead_;
    lookahead_ = kNone;
    return c;
  }
  if (char c; input_->get(c))
    return c;
  return Token::kEof;
}

void Lexer::PutBack(char c) {
  assert(lookahead_ == kNone);
  lookahead_ = c;
}

bool Lexer::TokenIsNumeric() {
  constexpr int kPrefix = 0, kIntPart = 2, kFracPart = 3, kExp = 4;
  int state = kPrefix;
  for (char c : identifier_) {
    switch (std::tolower(c)) {
    case '#':
    case 'b':
    case 'o':
    case 'x':
    case 'i':
      if (state == kPrefix) {
        continue;
      } else {
        return false;
      }
    case 's':
    case 'f':
    case 'l':
      if (state == kIntPart || state == kFracPart) {
        state = kExp;
        continue;
      } else {
        return false;
      }
    case 'd':
    case 'e':
      if (state == kPrefix) {
        continue;
      } else if (state == kIntPart || state == kFracPart) {
        state = kExp;
        continue;
      } else {
        return false;
      }
    case '.':
      if (state == kPrefix || state == kIntPart) {
        state = kFracPart;
        continue;
      } else {
        return false;
      }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (state == kPrefix) {
        state = kIntPart;
      }
      continue;
    default:
      return false;
    }
  }
  return state != kPrefix;
}

void Lexer::Debug() {
  switch (token_.type) {
  case Token::kDef:
    std::cerr << "[def]";
    break;
  case Token::kEof:
    std::cerr << "[eof]";
    break;
  case Token::kExtern:
    std::cerr << "[extern]";
    break;
  case Token::kIdentifier:
    std::cerr << "{id " << token_.value.string_value << "}";
    break;
  case Token::kIf:
    std::cerr << "[if]";
    break;
  case Token::kNumber:
    std::cerr << "{num " << token_.value.double_value << "}";
    break;
  default:
    std::cerr.put('[');
    std::cerr.put(token_.type);
    std::cerr.put(']');
  }
}

} // namespace benscope
