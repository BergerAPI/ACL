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

#include "interpreter.h"

void Interpreter::interpret(AbstractSyntaxTree *ast) {
    // Interpreting all children in the AST
    for (auto child: ast->children) {
        interpretChild(child);
    }
}

void Interpreter::interpretChild(AstChild *node) {
    // Interpret the child node
    if (node->getIdentifier() == "Expression" || node->getIdentifier() == "NumberLiteral" ||
        node->getIdentifier() == "Unary") {
        // We need to calculate the value of the expression
        // and store it in the value field of the node
        std::cout << interpretMathExpression(node) << std::endl;
    }
}

BasicValue Interpreter::interpretMathExpression(AstChild *node) {
    if (node->getIdentifier() == "Expression") {
        auto *realNode = dynamic_cast<ExpressionNode *>(node);
        auto left = interpretMathExpression(realNode->left.release());
        auto right = interpretMathExpression(realNode->right.release());

        if (left.type == BasicValue::Type::INT && right.type == BasicValue::Type::INT) {
            if (realNode->op == "+") return BasicValue(left.intValue + right.intValue);
            else if (realNode->op == "-") return BasicValue(left.intValue - right.intValue);
            else if (realNode->op == "*") return BasicValue(left.intValue * right.intValue);
            else if (realNode->op == "/") return BasicValue(left.intValue / right.intValue);
        } else throw std::runtime_error("Cannot perform math operation on non-integer values");
    } else if (node->getIdentifier() == "NumberLiteral")
        return BasicValue((dynamic_cast<NumberLiteralNode *>(node))->value);
    else if (node->getIdentifier() == "Unary") {
        auto realNode = dynamic_cast<UnaryExpressionNode *>(node);
        BasicValue value = interpretMathExpression(realNode->child.release());

        if (value.type == BasicValue::Type::INT) {
            if (realNode->op == "-")
                return BasicValue(-value.intValue);

            return BasicValue(value.intValue);
        } else throw std::runtime_error("Cannot perform unary operation on non-integer values");
    }

    throw std::runtime_error("Cannot interpret math expression");
}