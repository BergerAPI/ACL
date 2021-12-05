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

#include "lexer.h"

std::vector<Token> Lexer::tokenize(std::istream &input) {
    // Lexing the input for Token::Type
    std::vector<Token> tokens;
    std::vector<std::string> lines;

    for (std::string line; std::getline(input, line);)
        lines.push_back(line);

    int line_index = 0;

    for (std::string line: lines) {
        // Going through the line
        for (int i = 0; i < line.size(); i++) {
            char c = line[i];

            // If the char is a whitespace, skip it
            if (c == ' ' || c == '\t')
                continue;

            // If the char is a comment, skip the rest of the line
            if (c == '#') {
                while (i < line.size() && line[i] != '\n')
                    i++;
                break;
            }

            // If the char is a newline, skip it
            if (c == '\n')
                continue;

            // If the char is a number, create a number token
            if (isdigit(c)) {
                std::string number;

                while (i < line.size() && isdigit(line[i])) {
                    number += line[i];
                    i++;
                }

                i--;

                tokens.emplace_back(Token::Type::NUMBER, number, line_index);
                continue;
            }

            // If the char is an identifier
            if (isalpha(c)) {
                std::string identifier;

                while (i < line.size() && isalpha(line[i])) {
                    identifier += line[i];
                    i++;
                }

                i--;

                // Checking for keywords
                if (identifier == "if" || identifier == "else" || identifier == "while" ||
                    identifier == "function" || identifier == "return" || identifier == "var")
                    tokens.emplace_back(Token::Type::KEYWORD, identifier, line_index);
                else tokens.emplace_back(Token::Type::IDENTIFIER, identifier, line_index);

                continue;
            }

            // If the char is an operator character
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                tokens.emplace_back(Token::Type::OPERATOR, std::string(1, c), line_index);
                continue;
            }

            // If the char is a parenthesis
            if (c == '(') {
                tokens.emplace_back(Token::Type::LEFT_PAREN, std::string(1, c), line_index);
                continue;
            }

            if (c == ')') {
                tokens.emplace_back(Token::Type::RIGHT_PAREN, std::string(1, c), line_index);
                continue;
            }

            if (c == '{') {
                tokens.emplace_back(Token::Type::LEFT_BRACE, std::string(1, c), line_index);
                continue;
            }

            if (c == '}') {
                tokens.emplace_back(Token::Type::RIGHT_BRACE, std::string(1, c), line_index);
                continue;
            }

            if (c == '.') {
                tokens.emplace_back(Token::Type::DOT, std::string(1, c), line_index);
                continue;
            }

            if (c == ',') {
                tokens.emplace_back(Token::Type::COMMA, std::string(1, c), line_index);
                continue;
            }

            if (c == ':') {
                tokens.emplace_back(Token::Type::COLON, std::string(1, c), line_index);
                continue;
            }

            if (c == '=') {
                tokens.emplace_back(Token::Type::EQUALS, std::string(1, c), line_index);
                continue;
            }

            if (c == '"') {
                std::string string;
                i++;

                while (i < line.size() && line[i] != '"') {
                    string += line[i];
                    i++;
                }

                tokens.emplace_back(Token::Type::STRING, string, line_index);
                continue;
            }
        }

        line_index++;
    }

    return tokens;
}
