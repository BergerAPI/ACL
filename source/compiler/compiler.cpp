#include "compiler.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"

void Compiler::build()
{
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
        auto printf = llvm::Function::Create(
            llvm::FunctionType::get(builder->getInt32Ty(), {builder->getInt8PtrTy()}, true),
            llvm::Function::ExternalLinkage,
            "printf",
            module);

        scope->functions.push_back(Function("print", printf));

        // Setting the insertion point to the entry point
        builder->SetInsertPoint(llvm::BasicBlock::Create(*context, "entry", entry));

        for (const auto &item : *statements)
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