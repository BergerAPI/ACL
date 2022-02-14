#include <iostream>
#include <vector>

namespace err {
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

    static void error(std::string file, int error_code, int line, int start) {
        std::cerr << "Error: " << AVAILABLE_ERRORS[error_code] << "; " << file
                  << ":" << (line + 1) << ":"
                  << (start + 1) << std::endl;
    }

    static void error(std::string file, std::string error_additional, int error_code, int line, int start) {
        std::cerr << "Error: " << AVAILABLE_ERRORS[error_code] << " -> '" << error_additional << "'; " << file
                  << ":" << (line + 1) << ":"
                  << (start + 1) << std::endl;
    }

    static void error(int error_code, Token *token) {
        error(token->file_name, type_to_str(token->type), error_code, token->line, token->start);
    }

    static void error(int error_code, std::string add, Token *token) {
        error(token->file_name, add, error_code, token->line, token->start);
    }
}