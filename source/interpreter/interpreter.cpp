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

void Interpreter::importFile(AstChild *node) {
    auto realNode = dynamic_cast<ImportStatementNode *>(node);

    // Checking if we are in the root scope
    if (this->current_scope->parent != nullptr) {
        throw std::runtime_error("Import statement is not allowed in inner scopes");
    }

    // Getting the parsed abstractSyntaxTree
    auto abstractSyntaxTreeList = parse_file(realNode->path);

    for (const auto &abstractSyntaxTree: abstractSyntaxTreeList)
        // Adding all functions and variables to the current scope
        for (auto &item: abstractSyntaxTree->children) {
            if (item->getIdentifier() == "FunctionDefinition") {
                auto realItem = dynamic_cast<FunctionDefinitionNode *>(item);

                this->current_scope->functions.emplace_back(realItem->name, &realItem->parameters, &realItem->body,
                                                            this->current_scope, realItem->isExternal);
            } else if (item->getIdentifier() == "VariableDefinition") {
                auto realItem = dynamic_cast<VariableDefinitionNode *>(item);

                this->current_scope->variables.emplace_back(realItem->name,
                                                            this->interpretExpression(realItem->value.get()),
                                                            realItem->constant);
            } else if (item->getIdentifier() == "ImportStatement") {
                this->importFile(item);
            }
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
    } else if (node->getIdentifier() == "ImportStatement") {
        importFile(node);
    } else if (node->getIdentifier() == "VariableDefinition") {
        auto realNode = dynamic_cast<VariableDefinitionNode *>(node);

        this->current_scope->variables.emplace_back(realNode->name,
                                                    this->interpretExpression(realNode->value.get()),
                                                    realNode->constant);
    } else if (node->getIdentifier() == "VariableAssignment") {
        auto realNode = dynamic_cast<VariableAssignmentNode *>(node);

        // We need to find the variable in the current scope
        // Checking if the variable is defined in any scope above the current one
        for (auto scope = this->current_scope; scope != nullptr; scope = scope->parent) {
            for (auto &variable: scope->variables) {
                if (variable.name == realNode->name) {
                    if (variable.constant)
                        throw std::runtime_error("Cannot assign to constant variable");

                    variable.value = this->interpretExpression(realNode->value.get());
                    return;
                }
            }
        }

        // If we get here, the variable is not defined
        throw std::runtime_error("Variable " + realNode->name + " is not defined");
    } else if (node->getIdentifier() == "IfStatement") {
        auto realNode = dynamic_cast<IfStatementNode *>(node);

        // new scope
        this->current_scope = new Scope(this->current_scope);

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

        // back to the parent scope
        this->current_scope = this->current_scope->parent;
    } else if (node->getIdentifier() == "SwitchStatement") {
        auto realNode = dynamic_cast<SwitchStatementNode *>(node);
        auto expr = this->interpretExpression(realNode->condition.get());

        // Checking which case is the correct one
        for (auto &caseNode: realNode->cases) {
            auto caseExpr = this->interpretExpression(caseNode->condition.get());

            if (caseExpr.getValue() == expr.getValue()) {
                // If it is, we need to interpret the case
                for (auto &item: caseNode->body) {
                    // new scope
                    this->current_scope = new Scope(this->current_scope);

                    this->interpretChild(item.get());

                    // back to the parent scope
                    this->current_scope = this->current_scope->parent;
                }

                return;
            }
        }

    } else if (node->getIdentifier() == "WhileStatement") {
        auto realNode = dynamic_cast<WhileStatementNode *>(node);
        auto condition = std::move(realNode->condition).get();

        // new scope
        this->current_scope = new Scope(this->current_scope);

        // We need to check if the condition is true
        while (this->interpretExpression(condition).intValue == 1) {
            auto hasBreak = false;

            // If it is, we need to interpret the true branch
            for (auto &item: realNode->body) {
                for (const auto &n: this->getAllNodesInNode(item.get(), true)) {
                    if (n->getIdentifier() == "BreakStatement") {
                        hasBreak = true;
                        break;
                    }

                    if (n->getIdentifier() == "ContinueStatement") {
                        throw std::runtime_error("Continue statement is not supported in while loops");
                    }

                    if (n->getIdentifier() == "ReturnStatement") {
                        this->current_scope = this->current_scope->parent;
                        return;
                    }

                    if (!hasBreak)
                        this->interpretChild(n);

                    if (this->interpretExpression(condition).intValue == 0)
                        break;
                }
            }

            if (hasBreak)
                break;
        }

        // back to the parent scope
        this->current_scope = this->current_scope->parent;
    } else if (node->getIdentifier() == "ForStatement") {
        auto realNode = dynamic_cast<ForStatementNode *>(node);
        auto location = this->interpretExpression(realNode->location.get());

        if (location.type != BasicValue::Type::LIST)
            throw std::runtime_error("For loop location is not a list");

        auto list = location.listValue;

        // new scope
        this->current_scope = new Scope(this->current_scope);

        this->current_scope->variables.emplace_back(realNode->initializer, BasicValue(0), false);

        auto hasBreak = false;

        for (auto &item: list) {
            for (auto &variable: this->current_scope->variables) {
                if (variable.name == realNode->initializer) {
                    variable.value = item;

                    break;
                }
            }

            auto hasContinue = false;

            for (auto &scope: realNode->body) {
                // We need to go through everything. even if-statements in if-statements
                for (const auto &n: this->getAllNodesInNode(scope.get(), true)) {
                    if (n->getIdentifier() == "BreakStatement")
                        hasBreak = true;

                    if (n->getIdentifier() == "ContinueStatement")
                        hasContinue = true;

                    if (!hasBreak && !hasContinue)
                        this->interpretChild(n);
                }
            }
        }

        // back to the parent scope
        this->current_scope = this->current_scope->parent;
    } else if (node->getIdentifier() == "FunctionDefinition") {
        auto realNode = dynamic_cast<FunctionDefinitionNode *>(node);

        // Adding to the current scope
        this->current_scope->functions.emplace_back(realNode->name, &realNode->parameters, &realNode->body,
                                                    this->current_scope, realNode->isExternal);
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

                    switch (left.type) {
                        case BasicValue::Type::INT:
                            if (left.listValue[i].intValue != right.listValue[i].intValue) return BasicValue(false);
                            break;
                        case BasicValue::Type::FLOAT:
                            if (left.listValue[i].floatValue != right.listValue[i].floatValue) return BasicValue(false);
                            break;
                        case BasicValue::Type::STRING:
                            if (left.listValue[i].stringValue != right.listValue[i].stringValue)
                                return BasicValue(false);
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

        throw std::runtime_error(
                "Cannot perform math operation on non-integer values, line: " + std::to_string(realNode->line) +
                ". Values: " + std::to_string(left.type) + " and " + std::to_string(right.type));
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
                if (variable.name == realNode->name) {
                    return variable.value;
                }
            }
        }

        throw std::runtime_error(
                "Variable " + realNode->name + " is not defined, line: " + std::to_string(realNode->line + 1));
    } else if (node->getIdentifier() == "FunctionCall") {
        auto realNode = dynamic_cast<FunctionCallNode *>(node);

        // Searching in the all the scopes above the current scope
        for (auto scope = this->current_scope; scope != nullptr; scope = scope->parent) {
            for (const auto &item: scope->functions) {
                if (item.name == realNode->name) {
                    // Build-in
                    if (item.isExternal && function_exists(realNode->name)) {
                        // Mapping the arguments, so all arguments are BasicValues
                        std::vector<BasicValue> args;

                        for (auto &arg: realNode->args)
                            args.push_back(this->interpretExpression(arg.get()));

                        return executeFunction(realNode->name, args);
                    }

                    // Checking the arguments
                    if (item.parameters->size() != realNode->args.size())
                        throw std::runtime_error("Wrong number of arguments");

                    // Saving the current scope
                    auto old_scope = this->current_scope;

                    // new scope
                    auto new_scope = new Scope(item.scope);

                    // We need to add the parameters to the scope
                    auto index = 0;

                    for (auto &parameter: *item.parameters) {
                        new_scope->variables.emplace_back(parameter,
                                                          this->interpretExpression(
                                                                  realNode->args[index].get()), false);
                        index++;
                    }

                    this->current_scope = new_scope;

                    // Interpreting
                    for (const auto &bodyNode: *item.body) {
                        for (auto &nItem: this->getAllNodesInNode(bodyNode.get())) {
                            if (nItem->getIdentifier() == "ReturnStatement") {
                                auto returnNode = dynamic_cast<ReturnStatementNode *>(nItem);

                                if (returnNode->value != nullptr) {
                                    auto value = this->interpretExpression(returnNode->value.get());

                                    // Returning the value
                                    this->current_scope = old_scope;
                                    return value;
                                }

                                // Returning the value
                                this->current_scope = old_scope;
                                return BasicValue();
                            }

                            this->interpretChild(nItem);
                        }
                    }

                    // back to the parent scope
                    this->current_scope = old_scope;

                    return BasicValue();
                }
            }
        }

        throw std::runtime_error("Function " + realNode->name + " is not defined");
    } else if (node->getIdentifier() == "Array") {
        // Defining an array
        auto realNode = dynamic_cast<ArrayNode *>(node);

        std::vector<BasicValue> values;

        for (auto &value: realNode->elements)
            values.push_back(this->interpretExpression(value.get()));

        return BasicValue(values);
    } else if (node->getIdentifier() == "ArrayAccess") {
        // Accessing an array
        auto realNode = dynamic_cast<ArrayAccessNode *>(node);

        auto array = this->interpretExpression(realNode->array.get());

        if (array.type != BasicValue::Type::LIST)
            throw std::runtime_error("Array is not an array");

        auto index = this->interpretExpression(realNode->index.get());

        if (index.type != BasicValue::Type::INT)
            throw std::runtime_error("Index is not an integer");

        if (index.intValue < 0 || index.intValue >= array.listValue.size())
            throw std::runtime_error("Index out of bounds");

        return array.listValue[index.intValue];
    }

    throw std::runtime_error("Cannot interpret expression: " + node->getIdentifier());
}

std::vector<AstChild *> Interpreter::getAllNodesInNode(AstChild *node, bool ignoreLoops) {
    std::vector<AstChild *> nodes;

    nodes.push_back(node);

    if (node->getIdentifier() == "IfStatement") {
        // Removing the node (This is a workaround, because other nodes need to be added too, even if they
        // don't have a special case in this function)
        nodes.pop_back();

        auto ifNode = dynamic_cast<IfStatementNode *>(node);

        if (this->interpretExpression(ifNode->condition.get()).intValue == 1)
            for (auto &nItem: ifNode->thenBranch) {
                for (auto &item: this->getAllNodesInNode(nItem.get(), ignoreLoops)) {
                    nodes.push_back(item);
                }
            }
        else
            for (auto &nItem: ifNode->elseBranch) {
                for (auto &item: this->getAllNodesInNode(nItem.get(), ignoreLoops)) {
                    nodes.push_back(item);
                }
            }
    }

    if (!ignoreLoops) {
        if (node->getIdentifier() == "WhileStatement") {
            auto whileNode = dynamic_cast<WhileStatementNode *>(node);

            while (this->interpretExpression(whileNode->condition.get()).intValue == 1) {
                for (auto &item: whileNode->body)
                    for (const auto &childNode: getAllNodesInNode(item.get(), ignoreLoops))
                        nodes.push_back(childNode);
            }
        } else if (node->getIdentifier() == "ForStatement") {
            auto forNode = dynamic_cast<ForStatementNode *>(node);

            for (auto &item: forNode->body)
                for (const auto &childNode: getAllNodesInNode(item.get(), ignoreLoops))
                    nodes.push_back(childNode);
        }
    }

    return nodes;
}