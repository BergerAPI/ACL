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
#include <fstream>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"

static constexpr const char *k_file_extension = ".acl";

int main(int argv, char** args) {
    if (argv < 2) {
        std::cout << "Usage: " << args[0] << " <file>" << std::endl;
        return 1;
    }

    std::string file_name = args[1];

    if (file_name.find(k_file_extension) == std::string::npos) {
        file_name += k_file_extension;
    }

    std::ifstream file(file_name);

    if (!file.is_open()) {
        std::cout << "File not found: " << file_name << std::endl;
        return 1;
    }

    std::vector<Token> tokens = Lexer::tokenize(file);

    // Parsing
    Parser parser(tokens);

    // Parse the tokens
    auto ast = parser.parse();

    ast->print();

    Interpreter interpreter(ast);

    // Interpret the AST
    interpreter.interpret();

    return 0;
}
