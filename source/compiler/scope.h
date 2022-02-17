#pragma once

#include <llvm/IR/Value.h>
#include <iostream>
#include "../parser/type.h"

class Variable {
public:
    std::string name;
    llvm::Value *value;

    Variable(std::string name, llvm::Value *value) : name(name), value(value) {}
};

class Function {
public:
    std::string name;
    llvm::Function *function;
    Type return_type;

    Function(std::string name, llvm::Function *function, Type return_type) : name(name), function(function),
                                                                             return_type(return_type) {}
};

class Scope {
public:
    std::vector<Variable> variables;
    std::vector<Function> functions;
    llvm::Function *current_function;
    Scope *parent;

    Scope(Scope *parent) : parent(parent) {}
};