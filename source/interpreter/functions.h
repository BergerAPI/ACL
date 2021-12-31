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

#ifndef ACL_FUNCTIONS_H
#define ACL_FUNCTIONS_H

#include "type.h"
#include "../parser/ast.h"
#include <memory>
#include <vector>

// If a function is declared as integrated
bool function_exists(const std::string& name);

// Executing a function integrated in the interpreter.
BasicValue executeFunction(const std::string& name, const std::vector<BasicValue>& arguments);

#endif //ACL_FUNCTIONS_H
