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

#include "parser.h"
#include <memory>

std::unique_ptr<AstChild> Parser::factor() {
    Token currentToken = this->tokens[this->currentTokenIndex];

    this->currentTokenIndex++;

    switch (currentToken.type) {
        case Token::Type::NUMBER:
            return std::make_unique<NumberLiteralNode>(std::stoi(currentToken.raw));

        case Token::Type::STRING:
            return std::make_unique<StringLiteralNode>(currentToken.raw);

        case Token::Type::LEFT_PAREN: {
            auto expr = this->expression();
            this->expect(Token::Type::RIGHT_PAREN);

            return expr;
        }

        case Token::Type::OPERATOR: {
            // Unary operator
            if (currentToken.raw == "-" || currentToken.raw == "+") {
                auto expr = this->factor();
                return std::make_unique<UnaryExpressionNode>(std::move(expr), currentToken.raw);
            } else {
                throw std::runtime_error(
                        "Unexpected token: " + currentToken.raw + ", " + std::to_string(currentToken.type));
            }
        }

        default:
            break;
    }

    throw std::runtime_error("Unexpected token: " + currentToken.raw);
}

std::unique_ptr<AstChild> Parser::term() {
    auto left = this->factor();
    Token currentToken = this->tokens[this->currentTokenIndex];

    while (this->currentTokenIndex < this->tokens.size() && (currentToken.raw == "*" || currentToken.raw == "/")) {
        this->currentTokenIndex++;
        left = std::make_unique<ExpressionNode>(std::move(left), std::move(this->factor()), currentToken.raw);
        currentToken = this->tokens[this->currentTokenIndex];
    }

    return left;
}

std::unique_ptr<AstChild> Parser::expression() {
    auto left = this->term();

    while (this->currentTokenIndex < this->tokens.size()) {
        Token currentToken = this->tokens[this->currentTokenIndex];

        if (currentToken.type == Token::Type::OPERATOR) {
            this->currentTokenIndex++;

            if (currentToken.raw == "+" || currentToken.raw == "-") {
                auto right = this->term();
                left = std::make_unique<ExpressionNode>(std::move(left), std::move(right), currentToken.raw);
            } else {
                throw std::runtime_error("Unexpected token: " + currentToken.raw + ", we are parsing an expression");
            }
        } else {
            break;
        }
    }

    return left;
}

AbstractSyntaxTree *Parser::parse() {
    auto *ast = new AbstractSyntaxTree();

    while (this->currentTokenIndex < this->tokens.size()) {
        auto token = tokens[this->currentTokenIndex];

        switch (token.type) {
            case Token::Type::STRING:
            case Token::Type::LEFT_PAREN:
            case Token::Type::OPERATOR:
            case Token::Type::NUMBER: {
                // Expression
                auto expr = this->expression();

                // Add to AST
                this->children.push_back(expr.release());
            }
                break;

            default:
                throw std::runtime_error("Unknown token type: " + std::to_string(static_cast<int>(token.type)));
        }
    }

    ast->children = this->children;

    return ast;
}

Token Parser::getCurrentToken() {
    if (this->currentTokenIndex >= this->tokens.size())
        return {Token::Type::END_OF_FILE, "", 0};

    Token token = this->tokens[this->currentTokenIndex];

    this->currentTokenIndex++;

    return token;
}

[[maybe_unused]] Token Parser::peekNextToken() {
    if (this->currentTokenIndex + 1 >= this->tokens.size())
        return {Token::Type::END_OF_FILE, "", 0};

    return this->tokens[this->currentTokenIndex + 1];
}

void Parser::expect(Token::Type type) {
    Token currentToken = this->getCurrentToken();

    if (currentToken.type != type)
        throw std::runtime_error("Unexpected token: " + currentToken.raw + ". We expected: " + std::to_string(type));
}

Parser::Parser(std::vector<Token> tokens) {
    this->tokens = std::move(tokens);
}
