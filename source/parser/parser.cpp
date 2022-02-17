#include "parser.h"
#include "../error/error.h"

Token *parser::Parser::expect(Token::Type type) {
    Token *token = &this->tokens[this->current_token_index];

    if (token->type != type) {
        err::error(6, "Expected token of type " + type_to_str(type) + ", found " + type_to_str(token->type), token);
        exit(EXIT_FAILURE);
    }

    this->current_token_index++;

    if (this->current_token_index >= this->tokens.size()) {
        this->current_token = nullptr;
        return token;
    }

    this->current_token = &this->tokens[this->current_token_index];

    return token;
}

std::unique_ptr<parser::ast::ASTNode> parser::Parser::operand() {
    if (current_token->type == Token::Type::INT) {
        auto i = this->expect(Token::Type::INT);
        return std::make_unique<ast::ASTInteger>(std::stoi(i->raw), i);
    }

    if (current_token->type == Token::Type::STRING) {
        auto s = this->expect(Token::Type::STRING);
        return std::make_unique<ast::ASTString>(s->raw, s);
    }

    if (current_token->type == Token::Type::LEFT_PAREN) {
        this->expect(Token::Type::LEFT_PAREN);
        auto node = this->expression();
        this->expect(Token::Type::RIGHT_PAREN);
        return node;
    }

    if (current_token->type == Token::Type::KEYWORD) {
        auto k = this->expect(Token::Type::KEYWORD);

        if (k->raw == "func") {
            auto identifier = this->expect(Token::Type::IDENTIFIER);
            auto params = std::vector<std::unique_ptr<ast::ASTFunctionParameter>>();
            auto body = std::vector<std::unique_ptr<ast::ASTNode>>();
            auto return_type = Type(Type::Kind::Void, "Void");

            this->expect(Token::Type::LEFT_PAREN);

            while (current_token->type != Token::Type::RIGHT_PAREN) {
                auto param_name = this->expect(Token::Type::IDENTIFIER);

                this->expect(Token::Type::COLON);

                auto param_type_raw = this->expect(Token::Type::IDENTIFIER);
                auto param_type = string_to_kind(param_type_raw->raw);
                auto type = Type(param_type, param_type_raw->raw);

                params.push_back(std::make_unique<ast::ASTFunctionParameter>(param_name->raw, type, k));

                if (current_token->type != Token::Type::RIGHT_PAREN) {
                    this->expect(Token::Type::COMMA);
                }
            }

            this->expect(Token::Type::RIGHT_PAREN);

            if (current_token->type == Token::Type::ARROW) {
                this->expect(Token::Type::ARROW);

                auto return_type_raw = this->expect(Token::Type::IDENTIFIER);
                return_type = Type(string_to_kind(return_type_raw->raw), return_type_raw->raw);
            }

            this->expect(Token::Type::LEFT_BRACE);

            while (current_token != nullptr && current_token->type != Token::Type::RIGHT_BRACE) {
                body.push_back(this->expression());
            }

            this->expect(Token::Type::RIGHT_BRACE);

            return std::make_unique<ast::ASTFunctionDefinition>(identifier->raw, std::move(params), std::move(body),
                                                                return_type, k);
        }

        if (k->raw == "return") {
            if (this->current_token->raw == "void")
                return std::make_unique<ast::ASTFunctionReturn>(std::move(k));

            return std::make_unique<ast::ASTFunctionReturn>(std::move(this->expression()), k);
        }

        if (k->raw == "if") {
            this->expect(Token::Type::LEFT_PAREN);
            auto condition = this->expression();
            this->expect(Token::Type::RIGHT_PAREN);

            auto then_branch = std::vector<std::unique_ptr<ast::ASTNode>>();
            auto else_branch = std::vector<std::unique_ptr<ast::ASTNode>>();

            this->expect(Token::Type::LEFT_BRACE);

            while (current_token != nullptr && current_token->type != Token::Type::RIGHT_BRACE) {
                then_branch.push_back(this->expression());
            }

            this->expect(Token::Type::RIGHT_BRACE);

            if (current_token != nullptr && current_token->type == Token::Type::KEYWORD && current_token->raw == "else") {
                this->expect(Token::Type::KEYWORD);
                this->expect(Token::Type::LEFT_BRACE);

                while (current_token != nullptr && current_token->type != Token::Type::RIGHT_BRACE) {
                    else_branch.push_back(this->expression());
                }

                this->expect(Token::Type::RIGHT_BRACE);
            }

            return std::make_unique<ast::ASTIfStatement>(std::move(condition), std::move(then_branch),
                                                         std::move(else_branch), k);
        }
    }

    return identifier();
}

std::unique_ptr<parser::ast::ASTNode> parser::Parser::term() {
    auto node = this->operand();

    if (current_token == nullptr)
        return node;

    while (current_token != nullptr && (current_token->raw == "*" || current_token->raw == "/")) {
        Token *op = this->expect(current_token->type);;

        auto right = this->operand();

        node = std::make_unique<ast::ASTBinaryExpression>(std::move(node), std::move(right), op->raw, node->get_base());
    }

    return node;
}

std::unique_ptr<parser::ast::ASTNode> parser::Parser::expression() {
    auto node = this->term();

    while (current_token != nullptr && (current_token->raw == "+" || current_token->raw == "-")) {
        Token *op = this->expect(current_token->type);

        auto right = this->term();

        node = std::make_unique<ast::ASTBinaryExpression>(std::move(node), std::move(right), op->raw, node->get_base());
    }

    // Checking for == and !=
    while (current_token != nullptr &&
        (current_token->raw == "==" || current_token->raw == "!=" || current_token->raw == ">" ||
         current_token->raw == "<" || current_token->raw == ">=" || current_token->raw == "<=")) {

        Token *op = this->expect(current_token->type);

        auto right = this->expression();

        node = std::make_unique<ast::ASTBinaryExpression>(std::move(node), std::move(right), op->raw, node->get_base());
    }

    return node;
}

std::unique_ptr<parser::ast::ASTNode> parser::Parser::identifier() {
    auto identifier = this->expect(Token::Type::IDENTIFIER);

    // Checking for variable creating (identifier := expression)
    if (current_token != nullptr && current_token->type == Token::Type::ASSIGN) {
        this->expect(Token::Type::ASSIGN);
        auto expression = this->expression();

        return std::make_unique<ast::ASTVariableDefinition>(identifier->raw, std::move(expression), identifier);
    }

    // Checking for function class (identifier(parameters)
    if (current_token != nullptr && current_token->type == Token::Type::LEFT_PAREN) {
        std::vector<std::unique_ptr<ast::ASTNode>> parameters;

        this->expect(Token::Type::LEFT_PAREN);

        while (current_token != nullptr && current_token->type != Token::Type::RIGHT_PAREN) {
            parameters.push_back(this->expression());

            if (current_token != nullptr && current_token->type != Token::Type::RIGHT_PAREN) {
                this->expect(Token::Type::COMMA);
            }
        }

        this->expect(Token::Type::RIGHT_PAREN);

        return std::make_unique<ast::ASTFunctionCall>(identifier->raw, std::move(parameters), identifier);
    }

    return std::make_unique<ast::ASTVariableReference>(identifier->raw, identifier);
}

std::vector<std::unique_ptr<parser::ast::ASTNode>> parser::Parser::build() {
    current_token = &this->tokens[0];

    std::vector<std::unique_ptr<parser::ast::ASTNode>> statements;

    // Compile the AST
    while (current_token != nullptr) {
        if (current_token->type == Token::Type::INT || current_token->type == Token::Type::STRING ||
            current_token->type == Token::Type::IDENTIFIER || current_token->type == Token::Type::KEYWORD) {
            statements.push_back(std::move(expression()));
            continue;
        }

        if (current_token == nullptr)
            break;

        // Didn't match any of the above, so we're done
        err::error(0, current_token);
        exit(EXIT_FAILURE);
    }

    return statements;
}