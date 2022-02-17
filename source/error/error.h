#include <iostream>
#include <vector>
#include <fstream>

namespace err {
    static std::string COLOR_RED = "\x1b[31m";
    static std::string COLOR_GREEN = "\x1b[32m";
    static std::string RESET = "\x1b[0m";

    static const std::vector<std::string> AVAILABLE_ERRORS = {
            "Unknown token",
            "Unknown identifier",
            "Unknown function",
            "Unknown variable",
            "Unknown type",
            "Unexpected token",
            "Unexpected end of file",
            "Unexpected end of statement",
    };

    static void print_line(std::string file, int line, int start) {
        std::ifstream file_stream(file);
        std::string line_buffer;
        int line_number = 0;

        const std::string error_message =
                RESET + file + ":" + std::to_string(line + 1) + ":" + std::to_string(start + 1);

        while (std::getline(file_stream, line_buffer)) {
            if (line_number == line) {
                std::cout << error_message << "\t" << line_buffer << std::endl;
                std::cout << error_message << "\t" << std::string(start, ' ') << "^" << RESET << std::endl;

                break;
            }
            line_number++;
        }
    }

    static void error(std::string file, std::string error_additional, int error_code, int line, int start) {
        const std::string error_message =
                RESET + file + ":" + std::to_string(line + 1) + ":" + std::to_string(start + 1);

        std::cout << error_message << COLOR_RED << " error: " << RESET << AVAILABLE_ERRORS[error_code] << " -> "
                  << error_additional << RESET << std::endl;

        print_line(file, line, start);
    }

    static void error(int error_code, Token *token) {
        error(token->file_name, type_to_str(token->type), error_code, token->line, token->start);
    }

    static void error(int error_code, std::string add, Token *token) {
        error(token->file_name, add, error_code, token->line, token->start);
    }
}