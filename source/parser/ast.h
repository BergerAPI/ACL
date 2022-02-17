#pragma once

// LLVM
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "../json.hpp"
#include "../compiler/scope.h"
#include "type.h"

namespace parser {
    namespace ast {
        class ASTNode {
        public:
            virtual Token *get_base() = 0;

            virtual llvm::Value *
            visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder, llvm::Module *module) = 0;

            virtual nlohmann::json to_json() = 0;
        };

        class ASTInteger : public ASTNode {
        public:
            int value;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTInteger(int value, Token *base_token) : value(value) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTString : public ASTNode {
        public:
            std::string value;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTString(std::string value, Token *base_token) : value(value) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTVariableDefinition : public ASTNode {
        public:
            std::string name;
            std::unique_ptr<ASTNode> value;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTVariableDefinition(std::string name, std::unique_ptr<ASTNode> value, Token *base_token) : name(name),
                                                                                                         value(std::move(
                                                                                                                 value)) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTVariableReference : public ASTNode {
        public:
            std::string name;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTVariableReference(std::string name, Token *base_token) : name(name) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTFunctionCall : public ASTNode {
        public:
            std::string name;
            std::vector<std::unique_ptr<ASTNode>> arguments;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTFunctionCall(std::string name, std::vector<std::unique_ptr<ASTNode>> arguments, Token *base_token)
                    : name(name),
                      arguments(std::move(
                              arguments)) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTFunctionParameter : public ASTNode {
        public:
            std::string name;
            Type type;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTFunctionParameter(std::string name, Type type, Token *base_token) : name(name), type(type) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTFunctionDefinition : public ASTNode {
        public:
            std::string name;
            std::vector<std::unique_ptr<ASTFunctionParameter>> arguments;
            std::vector<std::unique_ptr<ASTNode>> body;
            Type return_type;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTFunctionDefinition(std::string name, std::vector<std::unique_ptr<ASTFunctionParameter>> arguments,
                                  std::vector<std::unique_ptr<ASTNode>> body, Type return_type, Token *base_token)
                    : name(name),
                      arguments(std::move(arguments)),
                      body(std::move(body)),
                      return_type(return_type) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTFunctionReturn : public ASTNode {
        public:
            std::unique_ptr<ASTNode> value;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTFunctionReturn(std::unique_ptr<ASTNode> value, Token *base_token) : value(std::move(value)) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            ASTFunctionReturn(Token *base_token) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTIfStatement : public ASTNode {
        public:
            std::unique_ptr<ASTNode> condition;
            std::vector<std::unique_ptr<ASTNode>> body;
            std::vector<std::unique_ptr<ASTNode>> else_branch;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTIfStatement(std::unique_ptr<ASTNode> condition, std::vector<std::unique_ptr<ASTNode>> body,
                           std::vector<std::unique_ptr<ASTNode>> else_branch, Token *base_token)
                    : condition(std::move(condition)),
                      body(std::move(body)),
                      else_branch(std::move(else_branch)) {
                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTBinaryExpression : public ASTNode {
        public:
            std::unique_ptr<ASTNode> left;
            std::unique_ptr<ASTNode> right;
            std::string op;
            Token *base_token;

            Token *get_base() override {
                return base_token;
            }

            ASTBinaryExpression(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right, std::string op,
                                Token *base_token)
                    : left(std::move(left)), right(std::move(right)), op(op) {

                // Bad approach, but somehow the base_token is modified after setting it
                this->base_token = new Token(base_token->type, base_token->raw, base_token->file_name, base_token->line,
                                             base_token->start, base_token->end);
            }

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };
    }
}
