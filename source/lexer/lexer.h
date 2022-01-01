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

#ifndef ACL_LEXER_H
#define ACL_LEXER_H

#include <utility>

#include "iostream"
#include "string"
#include "vector"

#include <sstream>

class Token {
public:
    enum Type {
        OPERATOR, LEFT_PAREN, RIGHT_PAREN,
        LEFT_BRACE, RIGHT_BRACE, DOT, COMMA, COLON,
        LEFT_BRACKET, RIGHT_BRACKET,

        IDENTIFIER, STRING, INT, FLOAT, EQUALS,

        KEYWORD, END_OF_FILE,
    };

    Token(Type type, std::string raw, int line)
            : type(type), raw(std::move(raw)), line(line) {}

    Type type;
    std::string raw;
    int line;
};

// Sim;ple lexer with Identifier, Number, String, and Punctuation tokens.
class Lexer {
public:
    static std::vector<Token> tokenize(std::istream &input);
};


#endif //ACL_LEXER_H
