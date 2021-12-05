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

class Scope {
public:
    // Variables in vector
    std::vector<std::pair<std::string, BasicValue>> variables;
};

class Interpreter {
private:
    Scope *current_scope;

public:
    AbstractSyntaxTree *ast;

    Interpreter(AbstractSyntaxTree *ast) {
        this->ast = ast;
        this->current_scope = new Scope();
    }

    void interpret();

    void interpretChild(AstChild *node);

    BasicValue interpretExpression(AstChild *node);
};


#endif //ACL_INTERPRETER_H
