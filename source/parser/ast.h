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
            virtual llvm::Value *
            visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder, llvm::Module *module) = 0;

            virtual nlohmann::json to_json() = 0;
        };

        class ASTInteger : public ASTNode {
        public:
            int value;

            ASTInteger(int value) : value(value) {}

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTString : public ASTNode {
        public:
            std::string value;

            ASTString(std::string value) : value(value) {}

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTVariableDefinition : public ASTNode {
        public:
            std::string name;
            std::unique_ptr<ASTNode> value;

            ASTVariableDefinition(std::string name, std::unique_ptr<ASTNode> value) : name(name),
                                                                                      value(std::move(value)) {}

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTVariableReference : public ASTNode {
        public:
            std::string name;

            ASTVariableReference(std::string name) : name(name) {}

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTFunctionCall : public ASTNode {
        public:
            std::string name;
            std::vector<std::unique_ptr<ASTNode>> arguments;

            ASTFunctionCall(std::string name, std::vector<std::unique_ptr<ASTNode>> arguments) : name(name), arguments(
                    std::move(arguments)) {}

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTFunctionParameter : public ASTNode {
        public:
            std::string name;
            Type type;

            ASTFunctionParameter(std::string name, Type type) : name(name), type(type) {}

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

            ASTFunctionDefinition(std::string name, std::vector<std::unique_ptr<ASTFunctionParameter>> arguments,
                                  std::vector<std::unique_ptr<ASTNode>> body, Type return_type) : name(name),
                                                                                                  arguments(std::move(
                                                                                                          arguments)),
                                                                                                  body(std::move(body)),
                                                                                                  return_type(
                                                                                                          return_type) {}

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTFunctionReturn : public ASTNode {
        public:
            std::unique_ptr<ASTNode> value;

            ASTFunctionReturn(std::unique_ptr<ASTNode> value) : value(std::move(value)) {}

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };

        class ASTBinaryExpression : public ASTNode {
        public:
            std::unique_ptr<ASTNode> left;
            std::unique_ptr<ASTNode> right;
            std::string op;

            ASTBinaryExpression(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right, std::string op)
                    : left(std::move(left)), right(std::move(right)), op(op) {}

            llvm::Value *visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                    llvm::Module *module) override;

            nlohmann::json to_json() override;
        };
    }
}
