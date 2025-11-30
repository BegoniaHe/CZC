/**
 * @file token.cpp
 * @brief Token 相关实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/token.hpp"

#include <unordered_map>

namespace czc::lexer {

namespace {

/// 关键字到 TokenType 的映射表
const std::unordered_map<std::string_view, TokenType> kKeywordMap = {
    // 声明关键字
    {"let", TokenType::KW_LET},
    {"var", TokenType::KW_VAR},
    {"fn", TokenType::KW_FN},
    {"struct", TokenType::KW_STRUCT},
    {"enum", TokenType::KW_ENUM},
    {"type", TokenType::KW_TYPE},
    {"impl", TokenType::KW_IMPL},
    {"trait", TokenType::KW_TRAIT},
    {"return", TokenType::KW_RETURN},

    // 控制流关键字
    {"if", TokenType::KW_IF},
    {"else", TokenType::KW_ELSE},
    {"while", TokenType::KW_WHILE},
    {"for", TokenType::KW_FOR},
    {"in", TokenType::KW_IN},
    {"break", TokenType::KW_BREAK},
    {"continue", TokenType::KW_CONTINUE},
    {"match", TokenType::KW_MATCH},

    // 模块关键字
    {"import", TokenType::KW_IMPORT},
    {"as", TokenType::KW_AS},

    // 字面量关键字
    {"true", TokenType::LIT_TRUE},
    {"false", TokenType::LIT_FALSE},
    {"null", TokenType::LIT_NULL},
};

/// TokenType 到名称的映射表
const char *const kTokenTypeNames[] = {
    "IDENTIFIER",

    // Keywords
    "KW_LET",
    "KW_VAR",
    "KW_FN",
    "KW_STRUCT",
    "KW_ENUM",
    "KW_TYPE",
    "KW_IMPL",
    "KW_TRAIT",
    "KW_RETURN",
    "KW_IF",
    "KW_ELSE",
    "KW_WHILE",
    "KW_FOR",
    "KW_IN",
    "KW_BREAK",
    "KW_CONTINUE",
    "KW_MATCH",
    "KW_IMPORT",
    "KW_AS",

    // Comments
    "COMMENT_LINE",
    "COMMENT_BLOCK",
    "COMMENT_DOC",

    // Literals
    "LIT_INT",
    "LIT_FLOAT",
    "LIT_DECIMAL",
    "LIT_STRING",
    "LIT_RAW_STRING",
    "LIT_TEX_STRING",
    "LIT_TRUE",
    "LIT_FALSE",
    "LIT_NULL",

    // Arithmetic Operators
    "OP_PLUS",
    "OP_MINUS",
    "OP_STAR",
    "OP_SLASH",
    "OP_PERCENT",

    // Comparison Operators
    "OP_EQ",
    "OP_NE",
    "OP_LT",
    "OP_LE",
    "OP_GT",
    "OP_GE",

    // Logical Operators
    "OP_LOGICAL_AND",
    "OP_LOGICAL_OR",
    "OP_LOGICAL_NOT",

    // Bitwise Operators
    "OP_BIT_AND",
    "OP_BIT_OR",
    "OP_BIT_XOR",
    "OP_BIT_NOT",
    "OP_BIT_SHL",
    "OP_BIT_SHR",

    // Assignment Operators
    "OP_ASSIGN",
    "OP_PLUS_ASSIGN",
    "OP_MINUS_ASSIGN",
    "OP_STAR_ASSIGN",
    "OP_SLASH_ASSIGN",
    "OP_PERCENT_ASSIGN",
    "OP_AND_ASSIGN",
    "OP_OR_ASSIGN",
    "OP_XOR_ASSIGN",
    "OP_SHL_ASSIGN",
    "OP_SHR_ASSIGN",

    // Range Operators
    "OP_DOT_DOT",
    "OP_DOT_DOT_EQ",

    // Other Operators
    "OP_ARROW",
    "OP_FAT_ARROW",
    "OP_DOT",
    "OP_AT",
    "OP_COLON_COLON",

    // Delimiters
    "DELIM_LPAREN",
    "DELIM_RPAREN",
    "DELIM_LBRACE",
    "DELIM_RBRACE",
    "DELIM_LBRACKET",
    "DELIM_RBRACKET",
    "DELIM_COMMA",
    "DELIM_COLON",
    "DELIM_SEMICOLON",
    "DELIM_UNDERSCORE",

    // Reserved operators
    "OP_HASH",
    "OP_DOLLAR",
    "OP_BACKSLASH",

    // Special Tokens
    "TOKEN_NEWLINE",
    "TOKEN_EOF",
    "TOKEN_WHITESPACE",
    "TOKEN_UNKNOWN",
};

} // anonymous namespace

std::optional<TokenType> lookupKeyword(std::string_view word) {
  auto it = kKeywordMap.find(word);
  if (it != kKeywordMap.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::string_view tokenTypeName(TokenType type) {
  auto index = static_cast<std::size_t>(type);
  constexpr std::size_t kMaxIndex =
      sizeof(kTokenTypeNames) / sizeof(kTokenTypeNames[0]);

  if (index < kMaxIndex) {
    return kTokenTypeNames[index];
  }
  return "UNKNOWN";
}

} // namespace czc::lexer
