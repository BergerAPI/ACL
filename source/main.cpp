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

#include "main.h"
#include <filesystem>
#include "utils.h"
#include "error.h"

// A list of all parsed files.
std::vector<std::pair<std::string, AbstractSyntaxTree *>> parsed_files;

// The default path to the source directory.
std::string source_path;

int main(int argv, char **args) {
    throwError(ErrorType::WARNING, "test", "test", "test", 2, 2);
    if (argv < 2) {
        std::cout << "Usage: " << args[0] << " <file>" << std::endl;
        return 1;
    }

    // The source path is the path of the first argument, without the file.
    const size_t last_slash_idx = std::string(args[1]).rfind('/');

    if (std::string::npos != last_slash_idx) {
        source_path = std::string(args[1]).substr(0, last_slash_idx);
    }

    auto code = parse_file(std::string(args[1]), true)[0];

    // code->print();

    Interpreter interpreter(code);

    // Interpret the AST
    interpreter.interpret();

    return 0;
}

std::vector<AbstractSyntaxTree *> parse_file(std::string file_path, bool is_main_file) {
    vector<AbstractSyntaxTree *> trees;
    if (!file_path.ends_with(".acl"))
        file_path += ".acl";

    // Checking if we already have the file parsed in out files vector
    for (auto &file: parsed_files) {
        if (file.first == file_path) {
            trees.push_back(file.second);
            return trees;
        }
    }

    // Checking if it's a file from the default
    std::string std_path_raw = std::string(getenv("HOME")) + "/.acl/std/" + file_path;
    std::ifstream std_path(std_path_raw);


    // We have an absolute path, because of the home directory, so we can check if it exists
    if (std_path.good() || std_path.is_open()) {
        std_path.close();
        return parse_file(std_path_raw, true);
    }

    std::ifstream file((is_main_file ? "" : source_path + "/") + file_path);

    if (!file.is_open()) {
        auto splitPath = splitString(file_path, "/");
        auto fileName = splitPath[splitPath.size() - 1];
        if (!fileName.starts_with("*")) {
            throw std::runtime_error("File not found: " + (is_main_file ? "" : source_path + "/") + file_path);
        }
        splitPath.pop_back();
        std::string restPath;
        for (const auto &item: splitPath)
            restPath += item + "/";

        for (const auto &entry: std::filesystem::directory_iterator(source_path + "/" + restPath)) {
            string foundFile = entry.path();
            trees.push_back(parse_file(foundFile, true)[0]);
        }
        return trees;
    }

    //   /test/test/*   /test/test/utils.acl /test/test/main.acl

    std::vector<Token> tokens = Lexer::tokenize(file);

    // Parsing
    Parser parser(tokens);

    // Parse the tokens
    auto ast = parser.parse();

    parsed_files.emplace_back(file_path, ast);

    file.close();
    trees.push_back(ast);
    return trees;
}