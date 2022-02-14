#include <iostream>
#include <string>
#include <fstream>

#include "lexer.h"
#include "parser/parser.h"
#include "json.hpp"
#include "compiler/compiler.h"

#define RIX_VERSION_MAJOR 0
#define RIX_VERSION_MINOR 0
#define RIX_VERSION_PATCH 1

#define VERSION_STRING std::to_string(RIX_VERSION_MAJOR) + "." + std::to_string(RIX_VERSION_MINOR) + "." + std::to_string(RIX_VERSION_PATCH)

std::vector<std::string> evaluate_args(int argc, char **argv)
{
    // We need to parse all flags, and the main value, which in this case is the file name
    std::vector<std::string> file_name;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            std::cerr << "Flags aren't implemented yet." << std::endl;
            exit(EXIT_FAILURE);
        }
        else
            // We have a file name
            file_name.push_back(argv[i]);
    }

    return file_name;
}

std::ifstream read_file(std::string file_name)
{
    std::ifstream file(file_name);

    if (!file.is_open())
    {
        std::cerr << "Couldn't open file: " << file_name << std::endl;
        exit(EXIT_FAILURE);
    }

    return file;
}

/**
 * @brief Main Entry Point
 */
int main(int argc, char **argv)
{
    std::cout << "Running Rix v" << VERSION_STRING << std::endl;

    // Evaluating the arguments
    std::vector<std::string> file_names = evaluate_args(argc, argv);

    // If we have no file names, we need to print the help
    if (file_names.size() == 0)
    {
        std::cerr << "No file names were provided." << std::endl;
        exit(EXIT_FAILURE);
    }

    for (std::string file_name : file_names)
    {
        std::ifstream file = read_file(file_name);
        std::vector<Token> tokens = Lexer::tokenize(file_name, file);

        // Parsing the tokens
        std::vector<std::unique_ptr<parser::ast::ASTNode>> statements = parser::Parser(tokens).build();

        for (auto &statement : statements)
        {
            std::cout << statement->to_json().dump(4) << std::endl;
        }

        // Compiling
        auto compiler = new Compiler(&statements);
        compiler->build();
    }

    return EXIT_SUCCESS;
}