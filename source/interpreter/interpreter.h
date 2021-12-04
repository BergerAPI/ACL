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


#include "../parser/ast.h"
#include "type.h"

class Interpreter {
public:
    static void interpret(AbstractSyntaxTree *ast);

    static void interpretChild(AstChild *node);

    static BasicValue interpretExpression(AstChild *node);
};


#endif //ACL_INTERPRETER_H
