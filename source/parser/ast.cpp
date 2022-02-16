#include "ast.h"

llvm::Value *
parser::ast::ASTBinaryExpression::visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                             llvm::Module *module) {
    auto l = this->left->visit_node(scope, context, builder, module);
    auto r = this->right->visit_node(scope, context, builder, module);

    auto insert_block = builder->GetInsertBlock();

    if (op == "+")
        return llvm::BinaryOperator::CreateAdd(l, r, "", insert_block);

    if (op == "-")
        return llvm::BinaryOperator::CreateSub(l, r, "", insert_block);

    if (op == "*")
        return llvm::BinaryOperator::CreateMul(l, r, "", insert_block);

    if (op == "/")
        return llvm::BinaryOperator::CreateSDiv(l, r, "", insert_block);

    return nullptr;
}

nlohmann::json parser::ast::ASTBinaryExpression::to_json() {
    nlohmann::json j;
    j["type"] = "ASTBinaryExpression";
    j["op"] = this->op;
    j["left"] = this->left->to_json();
    j["right"] = this->right->to_json();
    return j;
}

llvm::Value *parser::ast::ASTInteger::visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                                 llvm::Module *module) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), value, true);
}

nlohmann::json parser::ast::ASTInteger::to_json() {
    nlohmann::json j;
    j["type"] = "ASTInteger";
    j["value"] = this->value;
    return j;
}

llvm::Value *parser::ast::ASTString::visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                                llvm::Module *module) {
    return builder->CreateGlobalStringPtr(llvm::StringRef(value), "", false);
}

nlohmann::json parser::ast::ASTString::to_json() {
    nlohmann::json j;
    j["type"] = "ASTString";
    j["value"] = this->value;
    return j;
}

llvm::Value *
parser::ast::ASTVariableDefinition::visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                               llvm::Module *module) {
    auto v = this->value->visit_node(scope, context, builder, module);
    auto ai = new llvm::AllocaInst(v->getType(), 0, name, builder->GetInsertBlock());

    // Adding the store instruction
    builder->CreateStore(v, ai);

    scope->variables.push_back(Variable(name, ai));

    return ai;
}

nlohmann::json parser::ast::ASTVariableDefinition::to_json() {
    nlohmann::json j;
    j["type"] = "ASTVariableDefinition";
    j["name"] = this->name;
    j["value"] = this->value->to_json();
    return j;
}

llvm::Value *
parser::ast::ASTVariableReference::visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                              llvm::Module *module) {
    for (auto s = scope; s != nullptr; s = s->parent) {
        for (auto &f: s->variables) {
            if (f.name == name) {
                return builder->CreateLoad(f.value, name);
            }
        }
    }

    std::cerr << "Variable " << name << " not found." << std::endl;
    exit(EXIT_FAILURE);
}

nlohmann::json parser::ast::ASTVariableReference::to_json() {
    nlohmann::json j;
    j["type"] = "ASTVariableReference";
    j["name"] = this->name;
    return j;
}

llvm::Value *
parser::ast::ASTFunctionCall::visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                         llvm::Module *module) {
    std::vector<llvm::Value *> args;

    for (auto &a: this->arguments) {
        args.push_back(a->visit_node(scope, context, builder, module));
    }

    for (auto s = scope; s != nullptr; s = s->parent) {
        for (auto &f: s->functions) {
            if (f.name == name) {
                return builder->CreateCall(f.function, args);
            }
        }
    }

    std::cerr << "Function " << this->name << " not found." << std::endl;
    exit(EXIT_FAILURE);
}

nlohmann::json parser::ast::ASTFunctionCall::to_json() {
    nlohmann::json j;
    j["type"] = "ASTFunctionCall";
    j["name"] = this->name;
    return j;
}

llvm::Value *
parser::ast::ASTFunctionDefinition::visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                               llvm::Module *module) {
    std::vector<llvm::Type *> args;

    // Creating the scope
    Scope *function_scope = new Scope(scope);

    // Creating the arguments
    for (auto &a: this->arguments) {
        auto type = kind_to_llvm(context, a->type);

        args.push_back(type);
    }

    // Creating the function
    llvm::FunctionType *f_type = llvm::FunctionType::get(kind_to_llvm(context, this->return_type), args, false);
    llvm::Function *function = llvm::Function::Create(f_type, llvm::Function::ExternalLinkage, this->name, module);

    // Creating the basic block
    llvm::BasicBlock *block = llvm::BasicBlock::Create(*context, "entry", function);

    // Saving the insert point
    auto save_point = builder->GetInsertBlock();

    builder->SetInsertPoint(block);

    // Adding the variables (we have the function here)
    int index = 0;

    for (auto &a: this->arguments) {
        auto value = function->arg_begin() + index;
        auto ai = new llvm::AllocaInst(value->getType(), 0, "", builder->GetInsertBlock());

        builder->CreateStore(value, ai);

        function_scope->variables.push_back(Variable(a->name, ai));

        index++;
    }

    // Creating the body
    for (auto &b: this->body) {
        b->visit_node(function_scope, context, builder, module);
    }

    // Returning
    if (this->return_type.kind == Type::Kind::Void)
        builder->CreateRetVoid();

    // Restoring the insert point
    builder->SetInsertPoint(save_point);

    scope->functions.push_back(Function(this->name, function));

    return function;
}

nlohmann::json parser::ast::ASTFunctionDefinition::to_json() {
    nlohmann::json j;
    j["type"] = "ASTFunctionDefinition";
    j["name"] = this->name;
    j["return_type"] = this->return_type.name;
    return j;
}

llvm::Value *
parser::ast::ASTFunctionParameter::visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                              llvm::Module *module) {
    return nullptr;
}

nlohmann::json parser::ast::ASTFunctionParameter::to_json() {
    nlohmann::json j;
    j["type"] = "ASTFunctionParameter";
    j["name"] = this->name;
    j["type"] = this->type.name;
    return j;
}

llvm::Value *
parser::ast::ASTFunctionReturn::visit_node(Scope *scope, llvm::LLVMContext *context, llvm::IRBuilder<> *builder,
                                              llvm::Module *module) {
    return builder->CreateRet(this->value->visit_node(scope, context, builder, module));
}

nlohmann::json parser::ast::ASTFunctionReturn::to_json() {
    nlohmann::json j;
    j["type"] = "ASTFunctionParameter";
    j["value"] = this->value->to_json();
    return j;
}