#include "compiler.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"

void add_external(Scope *scope, llvm::IRBuilder<> *builder, llvm::Module *module) {
    scope->functions.push_back(Function("print", llvm::Function::Create(
            llvm::FunctionType::get(builder->getInt32Ty(), {builder->getInt8PtrTy()}, true),
            llvm::Function::ExternalLinkage,
            "printf",
            module), Type(Type::Kind::Void, "void")));
    scope->functions.push_back(Function("malloc", llvm::Function::Create(
            llvm::FunctionType::get(builder->getInt8PtrTy(), {builder->getInt32Ty()}, false),
            llvm::Function::ExternalLinkage,
            "malloc",
            module), Type(Type::Kind::Pointer, "Void")));
    scope->functions.push_back(Function("strcpy", llvm::Function::Create(
            llvm::FunctionType::get(builder->getInt8PtrTy(), {builder->getInt8PtrTy(), builder->getInt8PtrTy()}, false),
            llvm::Function::ExternalLinkage,
            "strcpy",
            module), Type(Type::Kind::Pointer, "Void")));
    scope->functions.push_back(Function("strlen", llvm::Function::Create(
            llvm::FunctionType::get(builder->getInt32Ty(), {builder->getInt8PtrTy()}, false),
            llvm::Function::ExternalLinkage,
            "strlen",
            module), Type(Type::Kind::Int, "Int")));
    scope->functions.push_back(Function("strcat", llvm::Function::Create(
            llvm::FunctionType::get(builder->getInt8PtrTy(), {builder->getInt8PtrTy(), builder->getInt8PtrTy()}, false),
            llvm::Function::ExternalLinkage,
            "strcat",
            module), Type(Type::Kind::Pointer, "Void")));
    scope->functions.push_back(Function("strcmp", llvm::Function::Create(
            llvm::FunctionType::get(builder->getInt1Ty(), {builder->getInt8PtrTy(), builder->getInt8PtrTy()}, false),
            llvm::Function::ExternalLinkage,
            "strcmp",
            module), Type(Type::Kind::Int, "Int")));
}

void Compiler::build() {
    auto *context = new llvm::LLVMContext();
    auto *builder = new llvm::IRBuilder<>(*context);
    auto *module = new llvm::Module("main", *context);

    // Create the entry point
    auto *entry = llvm::Function::Create(
            llvm::FunctionType::get(builder->getInt32Ty(), false),
            llvm::Function::ExternalLinkage,
            "main",
            module);

    // external print function
    add_external(scope, builder, module);

    scope->current_function = entry;

    // Setting the insertion point to the entry point
    builder->SetInsertPoint(llvm::BasicBlock::Create(*context, "entry", entry));

    for (const auto &item: *statements)
        item->visit_node(this->scope, context, builder, module);

    // Return
    builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));

    std::cout << "Compilation finished." << std::endl;

    // Getting the IR code, with a temporary raw_fd_ostream
    std::error_code error;
    llvm::raw_fd_ostream out("out.ll", error);
    module->print(out, nullptr);
    out.close();
}