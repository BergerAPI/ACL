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

#ifndef ACL_AST_H
#define ACL_AST_H

#include <iostream>
#include <utility>
#include <vector>

class AstChild {
public:
    virtual ~AstChild() = default;

    virtual std::string getIdentifier() = 0;

    virtual void print() = 0;
};

class ExpressionNode : public AstChild {
public:
    ~ExpressionNode() override = default;

    std::unique_ptr<AstChild> left;
    std::unique_ptr<AstChild> right;
    std::string op;

    // Constructor requires a left and right child and an operator
    ExpressionNode(std::unique_ptr<AstChild> left, std::unique_ptr<AstChild> right, std::string op) {
        this->left = std::move(left);
        this->right = std::move(right);
        this->op = std::move(op);
    }

    [[nodiscard]] std::string getIdentifier() override {
        return "Expression";
    }

    void print() override {
        std::cout << "(";
        left->print();
        std::cout << " " << op << " ";
        right->print();
        std::cout << ")";
    }
};

class IntegerLiteralNode : public AstChild {

public:
    ~IntegerLiteralNode() override = default;

    int value;

    // Constructor requires a value
    explicit IntegerLiteralNode(int value) : value(value) {}

    [[nodiscard]] std::string getIdentifier() override {
        return "IntegerLiteral";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(" << value << ")";
    }
};

class FloatLiteralNode : public AstChild {
public:
    ~FloatLiteralNode() override = default;

    float value;

    // Constructor requires a value
    explicit FloatLiteralNode(float value) : value(value) {}

    [[nodiscard]] std::string getIdentifier() override {
        return "FloatLiteral";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(" << value << ")";
    }
};

class UnaryExpressionNode : public AstChild {
public:
    ~UnaryExpressionNode() override = default;

    std::unique_ptr<AstChild> child;
    std::string op;

    // Constructor requires a child and an operator
    UnaryExpressionNode(std::unique_ptr<AstChild> child, std::string op) {
        this->child = std::move(child);
        this->op = std::move(op);
    }

    [[nodiscard]] std::string getIdentifier() override {
        return "Unary";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(" << op << " ";
        child->print();
        std::cout << ")";
    }
};

class StringLiteralNode : public AstChild {
public:
    ~StringLiteralNode() override = default;

    std::string value;

    // Constructor requires a value
    explicit StringLiteralNode(std::string value) : value(std::move(value)) {}

    [[nodiscard]] std::string getIdentifier() override {
        return "StringLiteral";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(\"" << value << "\")";
    }
};

class VariableDefinitionNode : public AstChild {
public:
    ~VariableDefinitionNode() override = default;

    std::string name;
    std::unique_ptr<AstChild> value;

    // Constructor requires a name and a value
    VariableDefinitionNode(std::string name, std::unique_ptr<AstChild> value) {
        this->name = std::move(name);
        this->value = std::move(value);
    }

    [[nodiscard]] std::string getIdentifier() override {
        return "VariableDefinition";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(" << name << " ";
        value->print();
        std::cout << ")";
    }
};

class VariableReferenceNode : public AstChild {
public:
    ~VariableReferenceNode() override = default;

    std::string name;

    // Constructor requires a name
    explicit VariableReferenceNode(std::string name) : name(std::move(name)) {}

    [[nodiscard]] std::string getIdentifier() override {
        return "VariableReference";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(" << name << ")";
    }
};

class VariableAssignmentNode : public AstChild {
public:
    ~VariableAssignmentNode() override = default;

    std::string name;
    std::unique_ptr<AstChild> value;

    // Constructor requires a name and a value
    VariableAssignmentNode(std::string name, std::unique_ptr<AstChild> value) {
        this->name = std::move(name);
        this->value = std::move(value);
    }

    [[nodiscard]] std::string getIdentifier() override {
        return "VariableAssignment";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(" << name << " ";
        value->print();
        std::cout << ")";
    }
};

class FunctionCallNode : public AstChild {
public:
    ~FunctionCallNode() override = default;

    std::string name;
    std::vector<std::unique_ptr<AstChild>> args;

    // Constructor requires a name and a vector of arguments
    FunctionCallNode(std::string name, std::vector<std::unique_ptr<AstChild>> args) {
        this->name = std::move(name);
        this->args = std::move(args);
    }

    [[nodiscard]] std::string getIdentifier() override {
        return "FunctionCall";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(" << name << " ";
        for (auto &arg: args) {
            arg->print();
        }
        std::cout << ")";
    }
};

class IfStatementNode : public AstChild {
public:
    ~IfStatementNode() override = default;

    std::unique_ptr<AstChild> condition;
    std::vector<std::unique_ptr<AstChild>> thenBranch;
    std::vector<std::unique_ptr<AstChild>> elseBranch;

    // Constructor requires a condition, then branch and else branch
    IfStatementNode(std::unique_ptr<AstChild> condition, std::vector<std::unique_ptr<AstChild>> thenBranch,
                    std::vector<std::unique_ptr<AstChild>> elseBranch) {
        this->condition = std::move(condition);
        this->thenBranch = std::move(thenBranch);
        this->elseBranch = std::move(elseBranch);
    }

    [[nodiscard]] std::string getIdentifier() override {
        return "IfStatement";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(";
        condition->print();
        std::cout << " ";
        for (auto &branch: thenBranch) {
            branch->print();
        }
        std::cout << " ";
        for (auto &branch: elseBranch) {
            branch->print();
        }
        std::cout << ")";
    }
};

class WhileStatementNode : public AstChild {
public:
    ~WhileStatementNode() override = default;

    std::unique_ptr<AstChild> condition;
    std::vector<std::unique_ptr<AstChild>> body;

    // Constructor requires a condition and a body
    WhileStatementNode(std::unique_ptr<AstChild> condition, std::vector<std::unique_ptr<AstChild>> body) {
        this->condition = std::move(condition);
        this->body = std::move(body);
    }

    [[nodiscard]] std::string getIdentifier() override {
        return "WhileStatement";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(";
        condition->print();
        std::cout << " ";
        for (auto &statement: body) {
            statement->print();
            std::cout << " ";
        }
        std::cout << ")";
    }
};

class ForStatementNode : public AstChild {
public:
    ~ForStatementNode() override = default;

    std::string initializer;
    std::unique_ptr<AstChild> location;
    std::vector<std::unique_ptr<AstChild>> body;

    ForStatementNode(std::string initializer, std::unique_ptr<AstChild> location,
                     std::vector<std::unique_ptr<AstChild>> body) {
        this->initializer = std::move(initializer);
        this->location = std::move(location);
        this->body = std::move(body);
    }

    [[nodiscard]] std::string getIdentifier() override {
        return "ForStatement";
    }

    void print() override {
        std::cout << this->getIdentifier() << "(";
        std::cout << initializer;
        std::cout << " ";
        location->print();
        std::cout << " ";
        for (auto &statement: body) {
            statement->print();
            std::cout << " ";
        }
        std::cout << ")";
    }
};

class AbstractSyntaxTree {
public:
    virtual ~AbstractSyntaxTree() = default;

    std::vector<AstChild *> children;

    void print();
};

#endif //ACL_AST_H
