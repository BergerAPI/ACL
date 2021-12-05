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

void Interpreter::interpret() {
    // Interpreting all children in the AST
    for (auto child: this->ast->children) {
        this->interpretChild(child);
    }
}

void Interpreter::interpretChild(AstChild *node) {
    // Interpret the child node
    if (node->getIdentifier() == "Expression" || node->getIdentifier() == "NumberLiteral" ||
        node->getIdentifier() == "Unary" || node->getIdentifier() == "StringLiteral" ||
        node->getIdentifier() == "VariableReference" || node->getIdentifier() == "FunctionCall") {
        // We need to calculate the value of the expression
        // and store it in the value field of the node
        this->interpretExpression(node);
    } else if (node->getIdentifier() == "VariableDefinition") {
        auto realNode = dynamic_cast<VariableDefinitionNode *>(node);

        this->current_scope->variables.emplace_back(realNode->name,
                                                    this->interpretExpression(realNode->value.release()));
    }
}

BasicValue Interpreter::interpretExpression(AstChild *node) {
    if (node->getIdentifier() == "Expression") {
        auto *realNode = dynamic_cast<ExpressionNode *>(node);
        auto left = this->interpretExpression(realNode->left.release());
        auto right = this->interpretExpression(realNode->right.release());

        if (left.type == BasicValue::Type::INT && right.type == BasicValue::Type::INT) {
            if (realNode->op == "+") return BasicValue(left.intValue + right.intValue);
            else if (realNode->op == "-") return BasicValue(left.intValue - right.intValue);
            else if (realNode->op == "*") return BasicValue(left.intValue * right.intValue);
            else if (realNode->op == "/") return BasicValue(left.intValue / right.intValue);
        } else if (left.type == BasicValue::Type::STRING && right.type == BasicValue::Type::STRING) {
            if (realNode->op == "+") return BasicValue(left.stringValue + right.stringValue);
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else if (left.type == BasicValue::Type::INT && right.type == BasicValue::Type::STRING) {
            if (realNode->op == "+") return BasicValue(std::to_string(left.intValue) + right.stringValue);
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else if (left.type == BasicValue::Type::STRING && right.type == BasicValue::Type::INT) {
            if (realNode->op == "+") return BasicValue(left.stringValue + std::to_string(right.intValue));
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else throw std::runtime_error("Cannot perform math operation on non-integer values");
    } else if (node->getIdentifier() == "NumberLiteral")
        return BasicValue((dynamic_cast<NumberLiteralNode *>(node))->value);
    else if (node->getIdentifier() == "StringLiteral")
        return BasicValue((dynamic_cast<StringLiteralNode *>(node))->value);
    else if (node->getIdentifier() == "Unary") {
        auto realNode = dynamic_cast<UnaryExpressionNode *>(node);
        BasicValue value = this->interpretExpression(realNode->child.release());

        if (value.type == BasicValue::Type::INT) {
            if (realNode->op == "-")
                return BasicValue(-value.intValue);

            return BasicValue(value.intValue);
        } else throw std::runtime_error("Cannot perform unary operation on non-integer values");
    } else if (node->getIdentifier() == "VariableReference") {
        auto realNode = dynamic_cast<VariableReferenceNode *>(node);

        // Checking if the variable is defined in any scope above the current one
        for (auto scope = this->current_scope; scope != nullptr; scope = scope->parent) {
            for (auto &variable: scope->variables) {
                if (variable.first == realNode->name) {
                    return variable.second;
                }
            }
        }

        throw std::runtime_error("Variable " + realNode->name + " is not defined");
    } else if (node->getIdentifier() == "FunctionCall") {
        auto realNode = dynamic_cast<FunctionCallNode *>(node);

        // Build-in
        if (realNode->name == "print") {
            if (realNode->args.empty())
                throw std::runtime_error("print() takes at least one argument");

            std::string value;
            int index = 0;

            for (auto &item: realNode->args) {
                value += this->interpretExpression(item.release()).stringValue;

                if (index < realNode->args.size() - 1) value += " ";

                index++;
            }

            std::cout << value << std::endl;
            return BasicValue(1);
        } else if (realNode->name == "input") {
            if (!realNode->args.empty())
                throw std::runtime_error("input() takes no arguments");

            std::string value;
            std::cin >> value;

            return BasicValue(value);
        } else if (realNode->name == "len") {
            if (realNode->args.size() != 1)
                throw std::runtime_error("len() takes exactly one argument");

            auto basic_value = this->interpretExpression(realNode->args[0].release());

            if (basic_value.type != BasicValue::Type::STRING)
                throw std::runtime_error("len() can only be used on strings and lists");

            return BasicValue(std::to_string(basic_value.stringValue.size()));
        }

        throw std::runtime_error("Function " + realNode->name + " is not defined");
    }

    throw std::runtime_error("Cannot interpret expression: " + node->getIdentifier());
}