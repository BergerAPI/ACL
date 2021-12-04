cmake_minimum_required(VERSION 3.21)
project(ACL)

set(CMAKE_CXX_STANDARD 23)

add_executable(ACL source/main.cpp source/lexer/lexer.cpp source/lexer/lexer.h source/parser/parser.cpp source/parser/parser.h source/parser/ast.h source/parser/ast.cpp source/interpreter/interpreter.cpp source/interpreter/interpreter.h source/interpreter/type.h)
