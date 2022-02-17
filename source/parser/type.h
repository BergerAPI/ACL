#pragma once

#include <iostream>
#include "../lexer.h"

class Type {

public:
    enum class Kind {
        Void,
        Bool,
        Int,
        Float,
        String,
        Array,
        Struct,
        Enum,
        Pointer,
        Function,
        UserDefined,
    };

    Kind kind;
    std::string name;

    Type(Kind kind, std::string name) : kind(kind), name(name) {}
};

// This code is a little messed up, but it's the best I could come up with.
// I'm not sure if it's the best way to do it, but it works.
// C++ doesn't provide any way to get the name of an enum, so I had to
// write my own.

static std::string kind_to_string(Type::Kind type) {
    switch (type) {
        case Type::Kind::Void:
            return "Void";
        case Type::Kind::Bool:
            return "Bool";
        case Type::Kind::Int:
            return "Int";
        case Type::Kind::Float:
            return "Float";
        case Type::Kind::String:
            return "String";
        case Type::Kind::Array:
            return "Array";
        case Type::Kind::Struct:
            return "Struct";
        case Type::Kind::Enum:
            return "Enum";
        case Type::Kind::Pointer:
            return "Pointer";
        case Type::Kind::Function:
            return "Function";
        case Type::Kind::UserDefined:
            return "UserDefined";
    }

    return "";
}

static Type::Kind string_to_kind(std::string type) {
    if (type == "Void") return Type::Kind::Void;
    if (type == "Bool") return Type::Kind::Bool;
    if (type == "Int") return Type::Kind::Int;
    if (type == "Float") return Type::Kind::Float;
    if (type == "String") return Type::Kind::String;
    if (type == "Function") return Type::Kind::Function;
    if (type == "Array") return Type::Kind::Array;
    if (type == "Struct") return Type::Kind::Struct;
    if (type == "Enum") return Type::Kind::Enum;
    if (type == "Pointer") return Type::Kind::Pointer;

    return Type::Kind::UserDefined;
}

static llvm::Type *kind_to_llvm(llvm::LLVMContext *context, Type type) {
    switch (type.kind) {
        case Type::Kind::Void:
            return llvm::Type::getVoidTy(*context);
        case Type::Kind::Bool:
            return llvm::Type::getInt1Ty(*context);
        case Type::Kind::Int:
            return llvm::Type::getInt32Ty(*context);
        case Type::Kind::Float:
            return llvm::Type::getFloatTy(*context);
        case Type::Kind::String:
            return llvm::Type::getInt8PtrTy(*context);
        case Type::Kind::Array:
            return llvm::Type::getInt8PtrTy(*context);
        case Type::Kind::Struct:
            return llvm::Type::getInt8PtrTy(*context);
        case Type::Kind::Enum:
            return llvm::Type::getInt32Ty(*context);
        case Type::Kind::Pointer:
            return llvm::Type::getInt8PtrTy(*context);
        case Type::Kind::Function:
            return llvm::Type::getInt8PtrTy(*context);
        case Type::Kind::UserDefined:
            throw std::runtime_error("User defined type not supported");
    }

    throw std::runtime_error("Unknown type");
}

static Type *token_to_type(Token value) {
    if (value.type == Token::Type::IDENTIFIER)
        return new Type(Type::Kind::UserDefined, value.raw);

    if (value.type == Token::Type::INT)
        return new Type(Type::Kind::Int, "Int");

    if (value.type == Token::Type::FLOAT)
        return new Type(Type::Kind::Float, "Float");

    if (value.type == Token::Type::STRING)
        return new Type(Type::Kind::String, "String");

    return nullptr;
}