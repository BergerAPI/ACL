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

std::vector<Token> Lexer::tokenize(std::string name, std::istream &input) {
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
            if (c == '/') {
                if (i + 1 < line.size() && line[i + 1] == '/') {
                    while (i < line.size() && line[i] != '\n')
                        i++;
                    break;
                }
            }

            // If the char is a newline, skip it
            if (c == '\n')
                continue;

            // If the char is an integer or float, create an integer/a float token
            if (std::isdigit(c)) {
                std::string number;

                while (std::isdigit(c) || c == '.') {
                    number += c;
                    i++;
                    if (i >= line.size())
                        break;
                    c = line[i];
                }

                i--;

                if (number.find('.') != std::string::npos)
                    tokens.emplace_back(Token::Type::FLOAT, number, name, line_index, i - number.size() + 1, i + 1);
                else
                    tokens.emplace_back(Token::Type::INT, number, name, line_index, i - number.size() + 1, i + 1);

                continue;
            }

            // If the char is an identifier
            if (isalpha(c) || c == '_') {
                std::string identifier;

                while (i < line.size() && (isalpha(line[i]) || std::isdigit(line[i]) || line[i] == '_')) {
                    identifier += line[i];
                    i++;
                }

                i--;

                // Checking for "or" or "and"
                if (identifier == "or" || identifier == "and")
                    tokens.emplace_back(Token::Type::OPERATOR, identifier == "or" ? "||" : "&&", name, line_index,
                                        i - identifier.size() + 1, i + 1);
                else if (identifier == "if" || identifier == "else" || identifier == "while" ||
                         identifier == "func" || identifier == "return" || identifier == "let" ||
                         identifier == "for" || identifier == "break" || identifier == "continue" ||
                         identifier == "import" || identifier == "const" || identifier == "class")
                    tokens.emplace_back(Token::Type::KEYWORD, identifier, name, line_index, i - identifier.size() + 1,
                                        i + 1);
                else
                    tokens.emplace_back(Token::Type::IDENTIFIER, identifier, name, line_index,
                                        i - identifier.size() + 1, i + 1);

                continue;
            }

            // !=, ==, <, >, <=, >=, &&, ||
            if (c == '!') {
                if (i + 1 < line.size() && line[i + 1] == '=') {
                    tokens.emplace_back(Token::Type::OPERATOR, "!=", name, line_index, i, i + 2);
                    i++;
                } else
                    tokens.emplace_back(Token::Type::OPERATOR, "!", name, line_index, i, i + 1);
                continue;
            }

            if (c == '=') {
                if (i + 1 < line.size() && line[i + 1] == '=') {
                    tokens.emplace_back(Token::Type::OPERATOR, "==", name, line_index, i, i + 2);
                    i++;
                } else
                    tokens.emplace_back(Token::Type::EQUALS, "=", name, line_index, i, i + 1);
                continue;
            }

            if (c == '<') {
                if (i + 1 < line.size() && line[i + 1] == '=') {
                    tokens.emplace_back(Token::Type::OPERATOR, "<=", name, line_index, i, i + 2);
                    i++;
                } else
                    tokens.emplace_back(Token::Type::OPERATOR, "<", name, line_index, i, i + 1);
                continue;
            }

            if (c == '>') {
                if (i + 1 < line.size() && line[i + 1] == '=') {
                    tokens.emplace_back(Token::Type::OPERATOR, ">=", name, line_index, i, i + 2);
                    i++;
                } else
                    tokens.emplace_back(Token::Type::OPERATOR, ">", name, line_index, i, i + 1);
                continue;
            }

            if (c == '|') {
                if (i + 1 < line.size() && line[i + 1] == '|') {
                    tokens.emplace_back(Token::Type::OPERATOR, "||", name, line_index, i, i + 2);
                    i++;
                } else
                    tokens.emplace_back(Token::Type::OPERATOR, "|", name, line_index, i, i + 1);

                continue;
            }

            if (c == '&') {
                if (i + 1 < line.size() && line[i + 1] == '&') {
                    tokens.emplace_back(Token::Type::OPERATOR, "&&", name, line_index, i, i + 2);
                    i++;
                } else
                    tokens.emplace_back(Token::Type::OPERATOR, "&", name, line_index, i, i + 1);

                continue;
            }

            // If the char is an operator character
            if (c == '+' || c == '*' || c == '/' || c == '%') {
                tokens.emplace_back(Token::Type::OPERATOR, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            if (c == '-') {
                // Checking for an arrow (->)
                if (i + 1 < line.size() && line[i + 1] == '>') {
                    tokens.emplace_back(Token::Type::ARROW, "->", name, line_index, i, i + 2);
                    i++;
                    continue;
                }

                tokens.emplace_back(Token::Type::OPERATOR, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            // If the char is a parenthesis
            if (c == '(') {
                tokens.emplace_back(Token::Type::LEFT_PAREN, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            if (c == ')') {
                tokens.emplace_back(Token::Type::RIGHT_PAREN, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            if (c == '{') {
                tokens.emplace_back(Token::Type::LEFT_BRACE, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            if (c == '}') {
                tokens.emplace_back(Token::Type::RIGHT_BRACE, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            if (c == '.') {
                tokens.emplace_back(Token::Type::DOT, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            if (c == ',') {
                tokens.emplace_back(Token::Type::COMMA, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            if (c == ':') {
                // Checking for an assigment operator
                if (i + 1 < line.size() && line[i + 1] == '=') {
                    tokens.emplace_back(Token::Type::ASSIGN, ":=", name, line_index, i, i + 2);
                    i++;
                } else
                    tokens.emplace_back(Token::Type::COLON, std::string(1, c), name, line_index, i, i + 1);

                continue;
            }

            if (c == '[') {
                tokens.emplace_back(Token::Type::LEFT_BRACKET, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            if (c == ']') {
                tokens.emplace_back(Token::Type::RIGHT_BRACKET, std::string(1, c), name, line_index, i, i + 1);
                continue;
            }

            if (c == '"') {
                std::string string;
                i++;

                while (i < line.size() && line[i] != '"') {

                    if (line[i] == '\\') {
                        i++;
                        if (i < line.size()) {
                            if (line[i] == 'n')
                                string += '\n';
                            else if (line[i] == 't')
                                string += '\t';
                            else if (line[i] == '"')
                                string += '"';
                            else if (line[i] == '\\')
                                string += '\\';
                            else
                                string += line[i];
                        }
                    } else

                        string += line[i];
                    i++;
                }

                tokens.emplace_back(Token::Type::STRING, string, name, line_index, i - string.size(), i + 1);
                continue;
            }
        }

        line_index++;
    }

    return tokens;
}