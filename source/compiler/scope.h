#pragma once

#include <llvm/IR/Value.h>
#include <iostream>

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

    Function(std::string name, llvm::Function *function) : name(name), function(function) {}
};

class Scope {
public:
    std::vector<Variable> variables;
    std::vector<Function> functions;
    Scope *parent;

    Scope(Scope *parent) : parent(parent) {}
};