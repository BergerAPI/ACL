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

#ifndef ACL_TYPE_H
#define ACL_TYPE_H

#include <iostream>
#include <utility>

class BasicValue {
public:
    enum Type {
        INT,
        FLOAT,
        STRING,
        LIST,
        VOID,
    };

    Type type;

    int intValue{};
    float floatValue{};
    std::string stringValue;
    std::vector<BasicValue> listValue;

    explicit BasicValue(int value) : type(INT), intValue(value) {}

    explicit BasicValue(float value) : type(FLOAT), floatValue(value) {}

    explicit BasicValue(std::string value) : type(STRING), stringValue(std::move(value)) {
        // Replacing all '\\n' with a new line character.
        auto a = stringValue.find("\\n");
        while (a != std::string::npos) {
            stringValue.replace(a, 2, "\n");
            a = stringValue.find("\\n");
        }
    }

    explicit BasicValue() : type(VOID), stringValue("void"), intValue(0) {}

    explicit BasicValue(std::vector<BasicValue> value) : type(LIST), listValue(std::move(value)) {}

    friend std::ostream &operator<<(std::ostream &os, const BasicValue &value) {
        switch (value.type) {
            case INT:
                os << value.intValue;
                break;
            case FLOAT:
                os << value.floatValue;
                break;
            case VOID:
            case STRING:
                os << value.stringValue;
                break;
            case LIST:
                os << "[";
                for (auto &v: value.listValue) {
                    os << v << ", ";
                }
                os << "]";
                break;
        }
        return os;
    }
};

#endif //ACL_TYPE_H
