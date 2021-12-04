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

#include <iostream>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"

static constexpr const char *k_file_extension = ".acl";

int main() {
    std::string line = "7 + 2 * 2";
    std::stringstream ss;

    ss << line;

    std::vector<Token> tokens = Lexer::tokenize(ss);

    // Parsing
    Parser parser(tokens);

    // Parse the tokens
    auto ast = parser.parse();

    ast->print();

    // Interpret the AST
    Interpreter::interpret(ast);

    return 0;
}
