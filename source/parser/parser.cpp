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

std::unique_ptr<AstChild> Parser::importStatement() {
    this->currentTokenIndex++;

    auto importPath = this->tokens[this->currentTokenIndex].raw;

    this->expect(Token::Type::STRING);

    return std::make_unique<ImportStatementNode>(importPath);
}

std::unique_ptr<AstChild> Parser::returnStatement() {
    this->currentTokenIndex++;

    auto currentToken = this->tokens[this->currentTokenIndex];

    if (currentToken.type == Token::Type::INT || currentToken.type == Token::Type::FLOAT ||
        currentToken.type == Token::Type::STRING || currentToken.type == Token::Type::IDENTIFIER ||
        currentToken.type == Token::Type::LEFT_PAREN) {
        return std::make_unique<ReturnStatementNode>(this->expression());
    }

    return std::make_unique<ReturnStatementNode>();
}


std::unique_ptr<AstChild> Parser::functionDefinition() {
    auto isExternal = this->tokens[this->currentTokenIndex].raw == "external";

    this->currentTokenIndex++;

    if (isExternal)
        this->currentTokenIndex++;

    auto functionName = this->tokens[this->currentTokenIndex].raw;

    this->currentTokenIndex++;

    this->expect(Token::Type::LEFT_PAREN);

    std::vector<std::string> parameters;
    std::vector<std::unique_ptr<AstChild>> body;

    while (this->currentTokenIndex < this->tokens.size() &&
           this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_PAREN) {
        auto currentToken = this->tokens[this->currentTokenIndex];

        if (currentToken.type != Token::Type::IDENTIFIER) {
            throw std::runtime_error("Expected identifier line: " + std::to_string(currentToken.line + 1));
        }

        parameters.push_back(currentToken.raw);

        this->currentTokenIndex++;

        if (this->currentTokenIndex != this->tokens.size())
            if (this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_PAREN)
                this->expect(Token::Type::COMMA);
    }

    this->expect(Token::Type::RIGHT_PAREN);

    if (!isExternal) {
        this->expect(Token::Type::LEFT_BRACE);

        while (this->currentTokenIndex < this->tokens.size() &&
               this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_BRACE) {
            body.emplace_back(this->parseChild());
        }

        this->expect(Token::Type::RIGHT_BRACE);
    }

    if (this->tokens[this->currentTokenIndex].type == Token::Type::LEFT_BRACE)
        throw std::runtime_error("External functions can't have a body. Line: " +
                                 std::to_string(this->tokens[this->currentTokenIndex].line + 1));

    return std::make_unique<FunctionDefinitionNode>(std::move(functionName), std::move(parameters), std::move(body),
                                                    isExternal);
}

std::unique_ptr<AstChild> Parser::forStatement() {
    this->currentTokenIndex++;

    auto initializer = this->tokens[this->currentTokenIndex].raw;

    this->expect(Token::Type::IDENTIFIER);

    if (this->tokens[this->currentTokenIndex].raw != "in") {
        throw std::runtime_error("Expected 'in' after for loop initializer.");
    }

    this->currentTokenIndex++;

    auto iterator = this->expression();

    std::vector<std::unique_ptr<AstChild>> thenStatements;

    this->expect(Token::Type::LEFT_BRACE);

    while (this->currentTokenIndex < this->tokens.size() &&
           this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_BRACE) {
        thenStatements.emplace_back(this->parseChild());
    }

    this->expect(Token::Type::RIGHT_BRACE);

    return std::make_unique<ForStatementNode>(std::move(initializer), std::move(iterator), std::move(thenStatements));
}

std::unique_ptr<AstChild> Parser::whileStatement() {
    this->currentTokenIndex++;

    auto condition = this->expression();
    std::vector<std::unique_ptr<AstChild>> thenStatements;

    this->expect(Token::Type::LEFT_BRACE);

    while (this->currentTokenIndex < this->tokens.size() &&
           this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_BRACE) {
        thenStatements.emplace_back(this->parseChild());
    }

    this->expect(Token::Type::RIGHT_BRACE);

    return std::make_unique<WhileStatementNode>(std::move(condition), std::move(thenStatements));
}

std::unique_ptr<AstChild> Parser::ifStatement() {
    this->currentTokenIndex++;

    auto condition = this->expression();
    std::vector<std::unique_ptr<AstChild>> thenStatements;
    std::vector<std::unique_ptr<AstChild>> elseStatements;

    this->expect(Token::Type::LEFT_BRACE);

    while (this->currentTokenIndex < this->tokens.size() &&
           this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_BRACE) {
        thenStatements.emplace_back(this->parseChild());
    }

    this->expect(Token::Type::RIGHT_BRACE);

    if (this->currentTokenIndex < this->tokens.size() &&
        this->tokens[this->currentTokenIndex].raw == "else") {
        this->currentTokenIndex++;

        if (this->tokens[this->currentTokenIndex].raw == "if") {
            elseStatements.emplace_back(this->ifStatement());
        } else {
            this->expect(Token::Type::LEFT_BRACE);

            while (this->currentTokenIndex < this->tokens.size() &&
                   this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_BRACE) {
                elseStatements.emplace_back(this->parseChild());
            }

            this->expect(Token::Type::RIGHT_BRACE);
        }
    }

    return std::make_unique<IfStatementNode>(std::move(condition), std::move(thenStatements),
                                             std::move(elseStatements));
}

std::unique_ptr<AstChild> Parser::identifier(bool allowArrayAccess) {
    auto currentToken = this->tokens[this->currentTokenIndex];

    this->currentTokenIndex++;

    // Function call
    if (this->tokens[this->currentTokenIndex].type == Token::Type::LEFT_PAREN) {
        this->currentTokenIndex++;

        auto arguments = std::vector<std::unique_ptr<AstChild>>();

        // Arguments are optional and split by a COMMA
        while (this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_PAREN) {
            arguments.push_back(this->expression());

            if (this->tokens[this->currentTokenIndex].type == Token::Type::COMMA) {
                this->currentTokenIndex++;
            }
        }

        if (this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_PAREN) {
            throw std::runtime_error("Expected ')'");
        }

        this->currentTokenIndex++;

        return std::make_unique<FunctionCallNode>(currentToken.raw, std::move(arguments));

        // Variable Assignment
    } else if (this->tokens[this->currentTokenIndex].type == Token::Type::EQUALS) {
        this->currentTokenIndex++;

        auto value = this->expression();

        return std::make_unique<VariableAssignmentNode>(currentToken.raw, std::move(value));
    } else if (this->tokens[this->currentTokenIndex].type == Token::Type::LEFT_BRACKET) {
        if (allowArrayAccess) {
            this->currentTokenIndex--;

            // The last token has to be the array
            auto array = this->expression(false);

            return this->checkArrayAccess(std::move(array));
        }
    }

    auto res = std::make_unique<VariableReferenceNode>(currentToken.raw);
    res->line = currentToken.line;
    return res;
}

std::unique_ptr<AstChild> Parser::variableDefinition(bool constant) {
    this->currentTokenIndex++;

    auto variableName = this->tokens[this->currentTokenIndex].raw;

    this->expect(Token::Type::IDENTIFIER);
    this->expect(Token::Type::EQUALS);

    auto node = std::make_unique<VariableDefinitionNode>(variableName, std::move(this->expression()), constant);
    node->line = this->tokens[this->currentTokenIndex].line;
    return node;
}

std::unique_ptr<AstChild> Parser::factor(bool allowArrayAccess) {
    auto currentToken = this->tokens[this->currentTokenIndex];

    switch (currentToken.type) {
        case Token::Type::INT:
            this->currentTokenIndex++;
            return std::make_unique<IntegerLiteralNode>(std::stoi(currentToken.raw));

        case Token::Type::FLOAT:
            this->currentTokenIndex++;
            return std::make_unique<FloatLiteralNode>(std::stof(currentToken.raw));

        case Token::Type::STRING:
            this->currentTokenIndex++;
            return std::make_unique<StringLiteralNode>(currentToken.raw);

        case Token::Type::LEFT_PAREN: {
            this->currentTokenIndex++;
            auto expr = this->expression();
            this->expect(Token::Type::RIGHT_PAREN);

            return expr;
        }

        case Token::Type::OPERATOR: {
            this->currentTokenIndex++;

            // Unary operator
            if (currentToken.raw == "-" || currentToken.raw == "+") {
                auto expr = this->factor(allowArrayAccess);
                return std::make_unique<UnaryExpressionNode>(std::move(expr), currentToken.raw);
            } else {
                throw std::runtime_error(
                        "Unexpected token: " + currentToken.raw + ", " + std::to_string(currentToken.type) +
                        ", line: " + std::to_string(currentToken.line));
            }
        }

        case Token::Type::LEFT_BRACKET: {
            std::vector<std::unique_ptr<AstChild>> elements;

            this->currentTokenIndex++;

            while (this->currentTokenIndex <= this->tokens.size() &&
                   this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_BRACKET) {
                elements.push_back(this->expression());

                if (this->tokens[this->currentTokenIndex].type == Token::Type::COMMA)
                    this->currentTokenIndex++;
            }

            this->expect(Token::Type::RIGHT_BRACKET);

            auto thing = std::make_unique<ArrayNode>(std::move(elements));

            return checkArrayAccess(std::move(thing));
        }

        default:
            break;
    }

    return this->identifier(allowArrayAccess);
}

std::unique_ptr<AstChild> Parser::term(bool allowArrayAccess) {
    auto left = this->factor(allowArrayAccess);

    while (this->currentTokenIndex < this->tokens.size() &&
           (this->tokens[this->currentTokenIndex].raw == "*" || this->tokens[this->currentTokenIndex].raw == "/")) {
        auto currentToken = this->tokens[this->currentTokenIndex];

        this->expect(Token::Type::OPERATOR);

        left = std::make_unique<ExpressionNode>(std::move(left), std::move(this->factor(allowArrayAccess)), currentToken.raw);
    }

    return left;
}

std::unique_ptr<AstChild> Parser::expression(bool allowArrayAccess) {
    auto left = this->term(allowArrayAccess);

    if (this->currentTokenIndex >= this->tokens.size()) {
        return left;
    }

    std::string currentRaw = this->tokens[this->currentTokenIndex].raw;

    while (currentRaw == "+" || currentRaw == "-" ||
           currentRaw == "==" || currentRaw == "!=" ||
           currentRaw == ">" || currentRaw == "<" ||
           currentRaw == ">=" || currentRaw == "<=" ||
           currentRaw == "%") {
        auto currentToken = this->tokens[this->currentTokenIndex];

        this->expect(Token::Type::OPERATOR);

        left = std::make_unique<ExpressionNode>(std::move(left), std::move(this->term(allowArrayAccess)), currentToken.raw);
        left->line = currentToken.line;

        currentRaw = this->tokens[this->currentTokenIndex].raw;
    }

    // && and ||
    if (this->currentTokenIndex < this->tokens.size() &&
        (currentRaw == "||" || currentRaw == "&&")) {
        this->expect(Token::Type::OPERATOR);

        left = std::make_unique<ExpressionNode>(std::move(left), std::move(this->expression()), currentRaw);
    }

    return left;
}

AbstractSyntaxTree *Parser::parse() {
    auto *ast = new AbstractSyntaxTree();

    while (this->currentTokenIndex < this->tokens.size()) {
        auto child = this->parseChild();

        if (child) {
            this->children.emplace_back(child.release());
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
        throw std::runtime_error(
                "Unexpected token: " + currentToken.raw + ". We expected: " + std::to_string(type) + ", line: " +
                std::to_string(currentToken.line + 1));
}

Parser::Parser(std::vector<Token> tokens) {
    this->tokens = std::move(tokens);
}

std::unique_ptr<AstChild> Parser::parseChild() {
    auto token = this->tokens[this->currentTokenIndex];

    switch (token.type) {
        case Token::Type::IDENTIFIER:
        case Token::Type::STRING:
        case Token::Type::LEFT_PAREN:
        case Token::Type::OPERATOR:
        case Token::Type::INT:
        case Token::Type::FLOAT:
        case Token::Type::LEFT_BRACKET: {
            auto expr = this->expression();
            expr->line = token.line;
            return expr;
        }

        case Token::Type::KEYWORD: {
            auto result = std::unique_ptr<AstChild>();

            if (token.raw == "let" || token.raw == "const") result = this->variableDefinition(token.raw == "const");
            else if (token.raw == "if") result = this->ifStatement();
            else if (token.raw == "while") result = this->whileStatement();
            else if (token.raw == "for") result = this->forStatement();
            else if (token.raw == "func" || token.raw == "external") result = this->functionDefinition();
            else if (token.raw == "return") result = this->returnStatement();
            else if (token.raw == "import") result = this->importStatement();
            else if (token.raw == "switch") result = this->switchStatement();
            else if (token.raw == "class") result = this->classDefinition();
            else if (token.raw == "break") {
                this->currentTokenIndex++;
                result = std::make_unique<BreakStatementNode>();;
            } else if (token.raw == "continue") {
                this->currentTokenIndex++;
                result = std::make_unique<ContinueStatementNode>();
            } else throw std::runtime_error("Keyword not implemented: " + token.raw);

            result->line = token.line;
            return result;
        }


        default:
            throw std::runtime_error(
                    "Unknown token type: " + std::to_string(static_cast<int>(token.type)) + " on line: " +
                    std::to_string(token.line));
    }
}

std::unique_ptr<AstChild> Parser::checkArrayAccess(std::unique_ptr<AstChild> child) {
    if (this->currentTokenIndex >= this->tokens.size())
        return child;

    auto token = this->tokens[this->currentTokenIndex];

    if (token.type != Token::Type::LEFT_BRACKET)
        return child;

    this->currentTokenIndex++;

    auto expr = this->expression();
    expr->line = token.line;

    this->expect(Token::Type::RIGHT_BRACKET);

    if (this->tokens[this->currentTokenIndex].type == Token::Type::LEFT_BRACKET) {
        return this->checkArrayAccess(std::make_unique<ArrayAccessNode>(std::move(child), std::move(expr)));
    }

    return std::make_unique<ArrayAccessNode>(std::move(child), std::move(expr));
}

std::unique_ptr<AstChild> Parser::switchStatement() {
    this->currentTokenIndex++;

    auto expr = this->expression();

    this->expect(Token::Type::LEFT_BRACE);

    auto cases = std::vector<std::unique_ptr<SwitchCaseNode>>();

    while (this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_BRACE) {
        auto token = this->tokens[this->currentTokenIndex];

        if (token.type != Token::Type::KEYWORD || (token.raw != "case" && token.raw != "default"))
            throw std::runtime_error("Expected case keyword on line: " + std::to_string(token.line));

        this->currentTokenIndex++;

        auto statements = std::vector<std::unique_ptr<AstChild>>();

        std::unique_ptr<AstChild> caseExpr = nullptr;

        if (token.raw == "case") {
            caseExpr = this->expression();
            caseExpr->line = token.line;
        }

        this->expect(Token::Type::LEFT_BRACE);

        while (this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_BRACE) {
            statements.push_back(this->parseChild());
        }

        this->currentTokenIndex++;

        cases.push_back(std::make_unique<SwitchCaseNode>(std::move(caseExpr), std::move(statements)));
    }

    this->currentTokenIndex++;

    return std::make_unique<SwitchStatementNode>(std::move(expr), std::move(cases));
}

std::unique_ptr<AstChild> Parser::classDefinition() {
    this->currentTokenIndex++;

    auto name = this->tokens[this->currentTokenIndex].raw;

    this->expect(Token::Type::IDENTIFIER);
    this->expect(Token::Type::LEFT_PAREN);

    auto params = std::vector<std::string>();

    while (this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_PAREN) {
        auto token = this->tokens[this->currentTokenIndex];

        if (token.type != Token::Type::IDENTIFIER)
            throw std::runtime_error("Expected identifier on line: " + std::to_string(token.line));

        params.push_back(token.raw);

        this->currentTokenIndex++;

        if (this->tokens[this->currentTokenIndex].type == Token::Type::COMMA)
            this->currentTokenIndex++;
    }

    this->currentTokenIndex++;
    this->expect(Token::Type::LEFT_BRACE);

    auto members = std::vector<std::unique_ptr<AstChild>>();

    while (this->tokens[this->currentTokenIndex].type != Token::Type::RIGHT_BRACE) {
        members.push_back(this->parseChild());
    }

    this->currentTokenIndex++;

    return std::make_unique<ClassDefinitionNode>(name, std::move(members), std::move(params));
}
