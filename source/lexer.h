/*
 * Copyright (c) 2021/2022 BergerAPI.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <utility>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

class Token
{
public:
    enum Type
    {
        OPERATOR,
        LEFT_PAREN,
        RIGHT_PAREN,
        LEFT_BRACE,
        RIGHT_BRACE,
        DOT,
        COMMA,
        COLON,
        ASSIGN,
        LEFT_BRACKET,
        RIGHT_BRACKET,

        IDENTIFIER,
        STRING,
        INT,
        FLOAT,
        EQUALS,

        KEYWORD,
        ARROW
    };

    Token(Type type, std::string raw, std::string file_name, int line, int start, int end)
        : type(type), raw(std::move(raw)), file_name(std::move(file_name)), line(line), start(start), end(end) {}

    Type type;
    std::string raw;
    std::string file_name;
    int line;
    int start;
    int end;
};

static std::string type_to_str(Token::Type type) {
    switch (type) {
        case Token::OPERATOR: return "OPERATOR";
        case Token::LEFT_PAREN: return "LEFT_PAREN";
        case Token::RIGHT_PAREN: return "RIGHT_PAREN";
        case Token::LEFT_BRACE: return "LEFT_BRACE";
        case Token::RIGHT_BRACE: return "RIGHT_BRACE";
        case Token::DOT: return "DOT";
        case Token::COMMA: return "COMMA";
        case Token::COLON: return "COLON";
        case Token::ASSIGN: return "ASSIGN";
        case Token::LEFT_BRACKET: return "LEFT_BRACKET";
        case Token::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case Token::IDENTIFIER: return "IDENTIFIER";
        case Token::STRING: return "STRING";
        case Token::INT: return "INT";
        case Token::FLOAT: return "FLOAT";
        case Token::EQUALS: return "EQUALS";
        case Token::KEYWORD: return "KEYWORD";
        case Token::ARROW: return "ARROW";
        default: return "UNKNOWN -> " + type;
    }
}

// Simple lexer with Identifier, Number, String, and Punctuation tokens.
class Lexer
{
public:
    static std::vector<Token> tokenize(std::string name, std::istream &input);
};