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

class NumberLiteralNode : public AstChild {

public:
    ~NumberLiteralNode() override = default;

    int value;

    // Constructor requires a value
    explicit NumberLiteralNode(int value) : value(value) {}

    [[nodiscard]] std::string getIdentifier() override {
        return "NumberLiteral";
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

class AbstractSyntaxTree {
public:
    virtual ~AbstractSyntaxTree() = default;

    std::vector<AstChild *> children;

    void print();
};

#endif //ACL_AST_H
