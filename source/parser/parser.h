#pragma once

#include <iostream>
#include <memory>

#include "../lexer.h"
#include "ast.h"

namespace parser
{
    class Parser
    {
    public:
        std::vector<Token> tokens;
        int current_token_index = 0;
        Token *current_token;

        Parser(std::vector<Token> tokens) : tokens(tokens) {}

        std::unique_ptr<ast::ASTNode> expression();
        std::unique_ptr<ast::ASTNode> term();
        std::unique_ptr<ast::ASTNode> operand();
        std::unique_ptr<ast::ASTNode> identifier();

        Token *expect(Token::Type type);
        std::vector<std::unique_ptr<parser::ast::ASTNode>> build();
    };
};