#pragma once

#include <iostream>

#include "../parser/ast.h"
#include "scope.h"

// LLVM
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Constants.h>
#include "llvm/IR/DerivedTypes.h"

class Compiler
{
public:
    std::vector<std::unique_ptr<parser::ast::ASTNode>> *statements;
    Scope *scope;

    Compiler(std::vector<std::unique_ptr<parser::ast::ASTNode>> *statements) : statements(std::move(statements))
    {
        scope = new Scope(nullptr);
    }

    void build();
};