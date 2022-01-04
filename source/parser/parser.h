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

#ifndef ACL_PARSER_H
#define ACL_PARSER_H

#include "iostream"
#include "ast.h"
#include "../lexer/lexer.h"

class Parser {
    std::vector<Token> tokens;
    int currentTokenIndex = 0;
    std::vector<AstChild *> children;

    /**
     * Expressions
     */
    std::unique_ptr<AstChild> factor(bool allowArrayAccess);
    std::unique_ptr<AstChild> term(bool allowArrayAccess);
    std::unique_ptr<AstChild> expression(bool allowArrayAccess = true);

    /**
     * Statements
     */
    std::unique_ptr<AstChild> variableDefinition(bool constant);
    std::unique_ptr<AstChild> ifStatement();
    std::unique_ptr<AstChild> whileStatement();
    std::unique_ptr<AstChild> forStatement();
    std::unique_ptr<AstChild> returnStatement();
    std::unique_ptr<AstChild> importStatement();
    std::unique_ptr<AstChild> functionDefinition();
    std::unique_ptr<AstChild> switchStatement();
    std::unique_ptr<AstChild> classDefinition();

    /**
     * Identifier
     */
    std::unique_ptr<AstChild> identifier(bool allowArrayAccess);

    /**
     * Checking for array access
     */
    std::unique_ptr<AstChild> checkArrayAccess(std::unique_ptr<AstChild> child);

    /**
     * Important functions
     */
    void expect(Token::Type type);
    Token getCurrentToken();
    [[maybe_unused]] Token peekNextToken();

    std::unique_ptr<AstChild> parseChild();
public:
    AbstractSyntaxTree *parse();

    explicit Parser(std::vector<Token> tokens);
};


#endif //ACL_PARSER_H
