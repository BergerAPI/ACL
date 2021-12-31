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

    // Getting the parsed pAbstractSyntaxTree
    auto pAbstractSyntaxTree = parse_file(realNode->path);

    // Adding all functions and variables to the current scope
    for (auto &item: pAbstractSyntaxTree->children) {
        if (item->getIdentifier() == "FunctionDefinition") {
            auto realItem = dynamic_cast<FunctionDefinitionNode *>(item);

            this->current_scope->functions.emplace_back(realItem->name, &realItem->parameters, &realItem->body,
                                                        this->current_scope);
        } else if (item->getIdentifier() == "VariableDefinition") {
            auto realItem = dynamic_cast<VariableDefinitionNode *>(item);

            this->current_scope->variables.emplace_back(realItem->name,
                                                        this->interpretExpression(realItem->value.get()));
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

        this->current_scope->variables.emplace_back(realNode->initializer, BasicValue(0));

        auto hasBreak = false;

        for (auto &item: list) {
            for (auto &variable: this->current_scope->variables) {
                if (variable.first == realNode->initializer) {
                    variable.second = item;

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
                                                    this->current_scope);
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
                if (variable.first == realNode->name) {
                    return variable.second;
                }
            }
        }

        throw std::runtime_error(
                "Variable " + realNode->name + " is not defined, line: " + std::to_string(realNode->line + 1));
    } else if (node->getIdentifier() == "FunctionCall") {
        auto realNode = dynamic_cast<FunctionCallNode *>(node);

        // Build-in
        if (realNode->name == "write") {
            // Highly low level operation
            if (realNode->args.size() != 3) throw std::runtime_error("Wrong number of arguments for write");

            auto fd = this->interpretExpression(realNode->args[0].get());
            auto buf = this->interpretExpression(realNode->args[1].get());
            auto bytes = this->interpretExpression(realNode->args[2].get());

            if (fd.type != BasicValue::Type::INT) throw std::runtime_error("write() first argument must be an integer");
            if (buf.type != BasicValue::Type::STRING)
                throw std::runtime_error("write() second argument must be a string");
            if (bytes.type != BasicValue::Type::INT)
                throw std::runtime_error("write() third argument must be an integer");

            write(fd.intValue, buf.stringValue.c_str(), bytes.intValue);

            return BasicValue();
        } else if (realNode->name == "input") {
            // Get input from user
            if (!realNode->args.empty()) throw std::runtime_error("Wrong number of arguments for input");

            std::string input;
            std::getline(std::cin, input);

            return BasicValue(input);
        } else if (realNode->name == "os") {
            // getting the os name
            if (!realNode->args.empty()) throw std::runtime_error("Wrong number of arguments for os");

            auto os = "Other";

#ifdef _WIN32
            os = "Windows";
#elif _WIN64
            os = "Windows";
#elif __APPLE__ || __MACH__
            os = "MacOS";
#elif __linux__
            os = "Linux";
#endif

            return BasicValue(os);
        } else if (realNode->name == "exit") {
            // getting the os name
            if (realNode->args.size() != 1) throw std::runtime_error("Wrong number of arguments for exit");

            auto code = this->interpretExpression(realNode->args[0].get());

            if (code.type != BasicValue::Type::INT) throw std::runtime_error("exit() argument must be an integer");

            exit(code.intValue);

            return BasicValue();
        } else if (realNode->name == "len") {
            if (realNode->args.size() != 1)
                throw std::runtime_error("len() takes exactly one argument");

            auto basic_value = this->interpretExpression(realNode->args[0].get());

            if (basic_value.type != BasicValue::Type::STRING)
                throw std::runtime_error(
                        "len() can only be used on strings and lists, line: " + std::to_string(realNode->line + 1) +
                        ". Used on: " + std::to_string(basic_value.type));

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

            if (start.type != BasicValue::Type::INT || end.type != BasicValue::Type::INT ||
                step.type != BasicValue::Type::INT)
                throw std::runtime_error("range() can only be used on integers");

            std::vector<BasicValue> values;

            for (int i = start.intValue; i < end.intValue; i += step.intValue) {
                values.emplace_back(i);
            }

            return BasicValue(values);
        }

        // Searching in the all the scopes above the current scope
        for (auto scope = this->current_scope; scope != nullptr; scope = scope->parent) {
            for (const auto &item: scope->functions) {
                if (item.name == realNode->name) {
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
                                                                  realNode->args[index].get()));
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