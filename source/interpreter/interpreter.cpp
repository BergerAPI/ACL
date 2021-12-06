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
    if (node->getIdentifier() == "Expression" || node->getIdentifier() == "IntegerLiteral" ||
        node->getIdentifier() == "FloatLiteral" ||
        node->getIdentifier() == "Unary" || node->getIdentifier() == "StringLiteral" ||
        node->getIdentifier() == "VariableReference" || node->getIdentifier() == "FunctionCall") {
        // We need to calculate the value of the expression
        // and store it in the value field of the node
        this->interpretExpression(node);
    } else if (node->getIdentifier() == "VariableDefinition") {
        auto realNode = dynamic_cast<VariableDefinitionNode *>(node);

        this->current_scope->variables.emplace_back(realNode->name,
                                                    this->interpretExpression(realNode->value.get()));
    } else if (node->getIdentifier() == "VariableAssignment") {
        auto realNode = dynamic_cast<VariableAssignmentNode *>(node);

        // We need to find the variable in the current scope
        // Checking if the variable is defined in any scope above the current one
        for (auto scope = this->current_scope; scope != nullptr; scope = scope->parent) {
            for (auto &variable: scope->variables) {
                if (variable.first == realNode->name) {
                    variable.second = this->interpretExpression(realNode->value.get());
                    return;
                }
            }
        }

        // If we get here, the variable is not defined
        throw std::runtime_error("Variable " + realNode->name + " is not defined");
    } else if (node->getIdentifier() == "IfStatement") {
        auto realNode = dynamic_cast<IfStatementNode *>(node);

        // We need to check if the condition is true
        if (this->interpretExpression(realNode->condition.get()).intValue == 1) {
            // If it is, we need to interpret the true branch
            for (auto &item: realNode->thenBranch) {
                this->interpretChild(item.get());
            }
        } else {
            // If it is not, we need to interpret the false branch
            for (auto &item: realNode->elseBranch) {
                this->interpretChild(item.get());
            }
        }
    } else if (node->getIdentifier() == "WhileStatement") {
        auto realNode = dynamic_cast<WhileStatementNode *>(node);
        auto condition = std::move(realNode->condition).get();

        // We need to check if the condition is true
        while (this->interpretExpression(condition).intValue == 1) {
            // If it is, we need to interpret the true branch
            for (auto &item: realNode->body) {
                this->interpretChild(item.get());
            }
        }
    } else if (node->getIdentifier() == "ForStatement") {
        auto realNode = dynamic_cast<ForStatementNode *>(node);
        auto location = this->interpretExpression(realNode->location.get());

        if (location.type != BasicValue::Type::LIST)
            throw std::runtime_error("For loop location is not a list");

        auto list = location.listValue;

        this->current_scope->variables.emplace_back(realNode->initializer, BasicValue(0));

        for (auto &item: list) {
            for (auto &variable : this->current_scope->variables) {
                if (variable.first == realNode->initializer) {
                    variable.second = item;

                    break;
                }
            }

            for (auto &scope: realNode->body) {
                this->interpretChild(scope.get());
            }
        }
    }
}

BasicValue Interpreter::interpretExpression(AstChild *node) {
    if (node->getIdentifier() == "Expression") {
        auto *realNode = dynamic_cast<ExpressionNode *>(node);

        auto left = this->interpretExpression(realNode->left.get());
        auto right = this->interpretExpression(realNode->right.get());
        auto op = realNode->op;

        if (left.type == BasicValue::Type::INT && right.type == BasicValue::Type::INT) {
            if (op == "+") return BasicValue(left.intValue + right.intValue);
            else if (op == "-") return BasicValue(left.intValue - right.intValue);
            else if (op == "*") return BasicValue(left.intValue * right.intValue);
            else if (op == "/") return BasicValue(left.intValue / right.intValue);
            else if (op == "%") return BasicValue(left.intValue % right.intValue);
            else if (op == "==") return BasicValue(left.intValue == right.intValue);
            else if (op == "!=") return BasicValue(left.intValue != right.intValue);
            else if (op == "<") return BasicValue(left.intValue < right.intValue);
            else if (op == ">") return BasicValue(left.intValue > right.intValue);
            else if (op == "<=") return BasicValue(left.intValue <= right.intValue);
            else if (op == ">=") return BasicValue(left.intValue >= right.intValue);
            else if (op == "&&") return BasicValue(left.intValue && right.intValue);
            else if (op == "||") return BasicValue(left.intValue || right.intValue);
            else throw std::runtime_error("Unknown operator " + op);
        } else if (left.type == BasicValue::Type::STRING && right.type == BasicValue::Type::STRING) {
            if (op == "+") return BasicValue(left.stringValue + right.stringValue);
            else if (op == "==") return BasicValue(left.stringValue == right.stringValue);
            else if (op == "!=") return BasicValue(left.stringValue != right.stringValue);
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else if (left.type == BasicValue::Type::INT && right.type == BasicValue::Type::STRING) {
            if (op == "+") return BasicValue(std::to_string(left.intValue) + right.stringValue);
            else if (op == "==") return BasicValue(std::to_string(left.intValue) == right.stringValue);
            else if (op == "!=") return BasicValue(std::to_string(left.intValue) != right.stringValue);
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else if (left.type == BasicValue::Type::STRING && right.type == BasicValue::Type::INT) {
            if (op == "+") return BasicValue(left.stringValue + std::to_string(right.intValue));
            else if (op == "==") return BasicValue(left.stringValue == std::to_string(right.intValue));
            else if (op == "!=") return BasicValue(left.stringValue != std::to_string(right.intValue));
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else if (left.type == BasicValue::Type::FLOAT && right.type == BasicValue::Type::FLOAT) {
            std::cout << "left: " << left.floatValue << " right: " << right.floatValue << std::endl;

            if (op == "+") return BasicValue(left.floatValue + right.floatValue);
            else if (op == "-") return BasicValue(left.floatValue - right.floatValue);
            else if (op == "*") return BasicValue(left.floatValue * right.floatValue);
            else if (op == "/") return BasicValue(left.floatValue / right.floatValue);
            else if (op == "==") return BasicValue(left.floatValue == right.floatValue);
            else if (op == "!=") return BasicValue(left.floatValue != right.floatValue);
            else if (op == "<") return BasicValue(left.floatValue < right.floatValue);
            else if (op == ">") return BasicValue(left.floatValue > right.floatValue);
            else if (op == "<=") return BasicValue(left.floatValue <= right.floatValue);
            else if (op == ">=") return BasicValue(left.floatValue >= right.floatValue);
            else throw std::runtime_error("Unknown operator " + op);
        } else if (left.type == BasicValue::Type::STRING && right.type == BasicValue::Type::FLOAT) {
            if (op == "+") return BasicValue(left.stringValue + std::to_string(right.floatValue));
            else if (op == "==") return BasicValue(left.stringValue == std::to_string(right.floatValue));
            else if (op == "!=") return BasicValue(left.stringValue != std::to_string(right.floatValue));
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else if (left.type == BasicValue::Type::FLOAT && right.type == BasicValue::Type::STRING) {
            if (op == "+") return BasicValue(std::to_string(left.floatValue) + right.stringValue);
            else if (op == "==") return BasicValue(std::to_string(left.floatValue) == right.stringValue);
            else if (op == "!=") return BasicValue(std::to_string(left.floatValue) != right.stringValue);
            else throw std::runtime_error("Cannot divide, multiply two strings");
        } else if (left.type == BasicValue::Type::LIST && right.type == BasicValue::Type::LIST) {
            if (op == "==") {
                // Comparing all elements
                if (left.listValue.size() != right.listValue.size()) return BasicValue(false);

                for (int i = 0; i < left.listValue.size(); i++) {
                    if (left.type != right.type) return BasicValue(false);

                    switch(left.type) {
                        case BasicValue::Type::INT:
                            if (left.listValue[i].intValue != right.listValue[i].intValue) return BasicValue(false);
                            break;
                        case BasicValue::Type::FLOAT:
                            if (left.listValue[i].floatValue != right.listValue[i].floatValue) return BasicValue(false);
                            break;
                        case BasicValue::Type::STRING:
                            if (left.listValue[i].stringValue != right.listValue[i].stringValue) return BasicValue(false);
                            break;
                        case BasicValue::Type::LIST:
                            throw std::runtime_error("Unimplemented");
                        case BasicValue::VOID:
                            return BasicValue(false);
                    }
                }

                return BasicValue(true);
            } else throw std::runtime_error("Unknown operator " + op);
        }

        throw std::runtime_error("Cannot perform math operation on non-integer values");
    } else if (node->getIdentifier() == "IntegerLiteral") {
        auto realNode = dynamic_cast<IntegerLiteralNode *>(node);
        return BasicValue(realNode->value);
    } else if (node->getIdentifier() == "FloatLiteral") {
        auto realNode = dynamic_cast<FloatLiteralNode *>(node);
        return BasicValue(realNode->value);
    } else if (node->getIdentifier() == "StringLiteral") {
        auto realNode = dynamic_cast<StringLiteralNode *>(node);
        return BasicValue(realNode->value);
    } else if (node->getIdentifier() == "Unary") {
        auto realNode = dynamic_cast<UnaryExpressionNode *>(node);
        BasicValue value = this->interpretExpression(realNode->child.get());

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
                auto expression = this->interpretExpression(item.get());

                switch (expression.type) {
                    case BasicValue::Type::INT:
                        value += std::to_string(expression.intValue);
                        break;
                    case BasicValue::Type::FLOAT:
                        value += std::to_string(expression.floatValue);
                        break;
                    case BasicValue::Type::STRING:
                        value += expression.stringValue;
                        break;
                    default:
                        throw std::runtime_error("Cannot print non-integer values");
                }

                if (index < realNode->args.size() - 1) value += " ";

                index++;
            }

            std::cout << value << std::endl;
            return BasicValue();
        } else if (realNode->name == "input") {
            if (!realNode->args.empty())
                throw std::runtime_error("input() takes no arguments");

            // Reading the entire line
            std::string value;

            std::getline(std::cin, value);

            return BasicValue(value);
        } else if (realNode->name == "len") {
            if (realNode->args.size() != 1)
                throw std::runtime_error("len() takes exactly one argument");

            auto basic_value = this->interpretExpression(realNode->args[0].get());

            if (basic_value.type != BasicValue::Type::STRING)
                throw std::runtime_error("len() can only be used on strings and lists");

            // Returning an integer
            return BasicValue((int) basic_value.stringValue.size());
        } else if (realNode->name == "readFile") {
            // Reading a file
            if (realNode->args.size() != 1)
                throw std::runtime_error("file() takes exactly one argument");

            auto basic_value = this->interpretExpression(realNode->args[0].get());

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

            auto file_name = this->interpretExpression(realNode->args[0].get());

            if (file_name.type != BasicValue::Type::STRING)
                throw std::runtime_error("file() can only be used on strings");

            auto content = this->interpretExpression(realNode->args[1].get());

            if (content.type != BasicValue::Type::STRING)
                throw std::runtime_error("file() can only be used on strings");

            // Writing the file
            std::ofstream file(file_name.stringValue);

            // Create the file if it does not exist
            if (!file.is_open()) {
                file.open(file_name.stringValue, std::ios::out | std::ios::trunc);
            }

            file << content.stringValue;

            return BasicValue();
        } else if (realNode->name == "time") {
            if (!realNode->args.empty())
                throw std::runtime_error("time() takes no arguments");

            // We need the time in seconds
            auto string = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());

            // converting to float
            return BasicValue(std::stoi(string));
        } else if (realNode->name == "list") {
            if (realNode->args.empty())
                throw std::runtime_error("list() takes at least one arguments");

            std::vector<BasicValue> values;

            for (auto &arg: realNode->args) {
                values.push_back(this->interpretExpression(arg.get()));
            }

            return BasicValue(values);
        } else if (realNode->name == "range") {
            auto start = BasicValue(0);
            auto end = BasicValue(0);
            auto step = BasicValue(1);

            if (realNode->args.empty() || realNode->args.size() > 3)
                throw std::runtime_error("range() takes 1, 2 or 3 arguments");

            if (realNode->args.size() == 1)
                end = this->interpretExpression(realNode->args[0].get());

            if (realNode->args.size() == 2) {
                start = this->interpretExpression(realNode->args[0].get());
                end = this->interpretExpression(realNode->args[1].get());
            }

            if (realNode->args.size() == 3) {
                start = this->interpretExpression(realNode->args[0].get());
                end = this->interpretExpression(realNode->args[1].get());
                step = this->interpretExpression(realNode->args[2].get());
            }

            if (start.type != BasicValue::Type::INT || end.type != BasicValue::Type::INT || step.type != BasicValue::Type::INT)
                throw std::runtime_error("range() can only be used on integers");

            std::vector<BasicValue> values;

            for (int i = start.intValue; i < end.intValue; i += step.intValue) {
                values.emplace_back(i);
            }

            return BasicValue(values);
        }

        throw std::runtime_error("Function " + realNode->name + " is not defined");
    }

    throw std::runtime_error("Cannot interpret expression: " + node->getIdentifier());
}