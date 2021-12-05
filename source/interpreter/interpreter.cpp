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
    } else if (node->getIdentifier() == "VariableAssignment") {
        auto realNode = dynamic_cast<VariableAssignmentNode *>(node);

        // We need to find the variable in the current scope
        // Checking if the variable is defined in any scope above the current one
        for (auto scope = this->current_scope; scope != nullptr; scope = scope->parent) {
            for (auto &variable: scope->variables) {
                if (variable.first == realNode->name) {
                    variable.second = this->interpretExpression(realNode->value.release());
                    return;
                }
            }
        }

        // If we get here, the variable is not defined
        throw std::runtime_error("Variable " + realNode->name + " is not defined");
    } else if (node->getIdentifier() == "IfStatement") {
        auto realNode = dynamic_cast<IfStatementNode *>(node);

        // We need to check if the condition is true
        if (this->interpretExpression(realNode->condition.release()).intValue == 1) {
            // If it is, we need to interpret the true branch
            for (auto &item: realNode->thenBranch) {
                this->interpretChild(item.release());
            }
        } else {
            // If it is not, we need to interpret the false branch
            for (auto &item: realNode->elseBranch) {
                this->interpretChild(item.release());
            }
        }
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
            else if (realNode->op == "%") return BasicValue(left.intValue % right.intValue);
            else if (realNode->op == "==") return BasicValue(left.intValue == right.intValue);
            else if (realNode->op == "!=") return BasicValue(left.intValue != right.intValue);
            else if (realNode->op == "<") return BasicValue(left.intValue < right.intValue);
            else if (realNode->op == ">") return BasicValue(left.intValue > right.intValue);
            else if (realNode->op == "<=") return BasicValue(left.intValue <= right.intValue);
            else if (realNode->op == ">=") return BasicValue(left.intValue >= right.intValue);
            else if (realNode->op == "&&") return BasicValue(left.intValue && right.intValue);
            else if (realNode->op == "||") return BasicValue(left.intValue || right.intValue);
            else throw std::runtime_error("Unknown operator " + realNode->op);
        } else if (left.type == BasicValue::Type::STRING && right.type == BasicValue::Type::STRING) {
            if (realNode->op == "+") return BasicValue(left.stringValue + right.stringValue);
            else if (realNode->op == "==") return BasicValue(left.stringValue == right.stringValue);
            else if (realNode->op == "!=") return BasicValue(left.stringValue != right.stringValue);
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else if (left.type == BasicValue::Type::INT && right.type == BasicValue::Type::STRING) {
            if (realNode->op == "+") return BasicValue(std::to_string(left.intValue) + right.stringValue);
            else if (realNode->op == "==") return BasicValue(std::to_string(left.intValue) == right.stringValue);
            else if (realNode->op == "!=") return BasicValue(std::to_string(left.intValue) != right.stringValue);
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else if (left.type == BasicValue::Type::STRING && right.type == BasicValue::Type::INT) {
            if (realNode->op == "+") return BasicValue(left.stringValue + std::to_string(right.intValue));
            else if (realNode->op == "==") return BasicValue(left.stringValue == std::to_string(right.intValue));
            else if (realNode->op == "!=") return BasicValue(left.stringValue != std::to_string(right.intValue));
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
        } else if (realNode->name == "readFile") {
            // Reading a file
            if (realNode->args.size() != 1)
                throw std::runtime_error("file() takes exactly one argument");

            auto basic_value = this->interpretExpression(realNode->args[0].release());

            if (basic_value.type != BasicValue::Type::STRING)
                throw std::runtime_error("file() can only be used on strings");

            // Reading the file
            std::ifstream file(basic_value.stringValue);

            if (!file.is_open())
                throw std::runtime_error("Could not open file " + basic_value.stringValue);

            std::string value;

            while (file.good()) {
                std::string line;
                std::getline(file, line);
                value += line + "\n";
            }

            return BasicValue(value);
        } else if (realNode->name == "writeFile") {
            // Writing a file
            if (realNode->args.size() != 2)
                throw std::runtime_error("file() takes exactly two arguments");

            auto file_name = this->interpretExpression(realNode->args[0].release());

            if (file_name.type != BasicValue::Type::STRING)
                throw std::runtime_error("file() can only be used on strings");

            auto content = this->interpretExpression(realNode->args[1].release());

            if (content.type != BasicValue::Type::STRING)
                throw std::runtime_error("file() can only be used on strings");

            // Writing the file
            std::ofstream file(file_name.stringValue);

            // Create the file if it does not exist
            if (!file.is_open()) {
                file.open(file_name.stringValue, std::ios::out | std::ios::trunc);
            }

            file << content.stringValue;

            return BasicValue(1);
        }

        throw std::runtime_error("Function " + realNode->name + " is not defined");
    }

    throw std::runtime_error("Cannot interpret expression: " + node->getIdentifier());
}