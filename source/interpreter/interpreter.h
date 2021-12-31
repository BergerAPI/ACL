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

#ifndef ACL_INTERPRETER_H
#define ACL_INTERPRETER_H


#include <map>
#include "../parser/ast.h"
#include "type.h"
#include <fstream>
#include <utility>
#include "../main.h"
#include <memory>
#include <chrono>
#include <unistd.h>

class Scope;

class InterpreterFunction {
public:
    std::string name;
    std::vector<std::string> *parameters;
    std::vector<std::unique_ptr<AstChild>> *body;
    Scope *scope;

    explicit InterpreterFunction(std::string name, std::vector<std::string> *parameters, std::vector<std::unique_ptr<AstChild>> *body, Scope* scope) : name(std::move(name)), parameters(parameters), body(body), scope(scope) {}
};

class InterpretedVariable {
public:
    std::string name;
    BasicValue value;
    bool constant;

    explicit InterpretedVariable(std::string name, BasicValue value, bool constant) : name(std::move(name)), value(value), constant(constant) {}
};

class Scope {
public:
    explicit Scope(Scope *parent = nullptr) : parent(parent) {}

    // Variables in vector
    std::vector<InterpretedVariable> variables;

    // Functions in map (function name, function)
    std::vector<InterpreterFunction> functions;

    // The parent scope
    Scope *parent;
};

class Interpreter {
private:
    Scope *current_scope;

public:
    AbstractSyntaxTree *ast;

    explicit Interpreter(AbstractSyntaxTree *ast) {
        this->ast = ast;

        // Genesis Scope
        this->current_scope = new Scope();
    }

    void interpret();

    void importFile(AstChild *node);

    void interpretChild(AstChild *node);

    std::vector<AstChild*> getAllNodesInNode(AstChild *node, bool ignoreLoops = false);

    BasicValue interpretExpression(AstChild *node);
};


#endif //ACL_INTERPRETER_H
