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

#include <map>
#include <istream>
#include <fstream>
#include "functions.h"

BasicValue print(std::vector<BasicValue> &arguments) {
    std::string result;

    // Iterating through the arguments
    for (const auto &argument: arguments) {
        // Printing the argument
        result += argument.stringValue;
    }

    std::cout << result;

    return BasicValue();
}

BasicValue println(std::vector<BasicValue> &arguments) {
    print(arguments);

    std::cout << std::endl;

    return BasicValue();
}

BasicValue input(std::vector<BasicValue> &arguments) {
    if (!arguments.empty())
        throw std::runtime_error("Invalid number of arguments for input()");

    std::string result;

    std::getline(std::cin, result);

    return BasicValue(result);
}

BasicValue os(std::vector<BasicValue> &arguments) {
    // Getting the os name
    if (arguments.empty()) throw std::runtime_error("Wrong number of arguments for os");
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
}

BasicValue exit_(std::vector<BasicValue> arguments) {
    if (!arguments.empty() || arguments.size() > 1)
        throw std::runtime_error("Invalid number of arguments for exit()");

    exit(arguments.size() == 1 ? arguments[0].intValue : 0);
}

BasicValue len(std::vector<BasicValue> arguments) {
    if (arguments.size() != 1)
        throw std::runtime_error("len() takes exactly one argument");

    if (arguments[0].type != BasicValue::Type::STRING)
        throw std::runtime_error(
                "len() can only be used on strings and lists. Used on: " + std::to_string(arguments[0].type));

    // Returning an integer
    return BasicValue((int) arguments[0].stringValue.size());
}

BasicValue readFile(std::vector<BasicValue> arguments) {
    if (arguments.size() != 1)
        throw std::runtime_error("readFile() takes exactly one argument");

    if (arguments[0].type != BasicValue::Type::STRING)
        throw std::runtime_error(
                "readFile() can only be used on strings. Used on: " + std::to_string(arguments[0].type));

    // Opening the file
    std::ifstream file(arguments[0].stringValue);

    // Checking if the file is open
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + arguments[0].stringValue);

    // Reading the file
    std::string result;
    std::string line;

    while (std::getline(file, line)) {
        result += line;
    }

    // Closing the file
    file.close();

    return BasicValue(result);
}

BasicValue writeFile(std::vector<BasicValue> arguments) {
    if (arguments.size() != 2)
        throw std::runtime_error("writeFile() takes exactly two arguments");

    if (arguments[0].type != BasicValue::Type::STRING)
        throw std::runtime_error(
                "writeFile() can only be used on strings. Used on: " + std::to_string(arguments[0].type));

    if (arguments[1].type != BasicValue::Type::STRING)
        throw std::runtime_error(
                "writeFile() can only be used on strings. Used on: " + std::to_string(arguments[1].type));

    // Opening the file
    std::ofstream file(arguments[0].stringValue);

    // Checking if the file is open
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + arguments[0].stringValue);

    // Writing the file
    file << arguments[1].stringValue;

    // Closing the file
    file.close();

    return BasicValue();
}

BasicValue range(std::vector<BasicValue> arguments) {
    auto start = BasicValue(0);
    auto end = BasicValue(0);
    auto step = BasicValue(1);

    if (arguments.empty() || arguments.size() > 3)
        throw std::runtime_error("range() takes 1, 2 or 3 arguments");

    if (arguments.size() == 1) {
        end = std::move(arguments[0]);
    }

    if (arguments.size() == 2) {
        start = arguments[0];
        end = std::move(arguments[1]);
    }

    if (arguments.size() == 3) {
        start = std::move(arguments[0]);
        end = std::move(arguments[1]);
        step = std::move(arguments[2]);
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

BasicValue list(std::vector<BasicValue> arguments) {
    if (arguments.empty())
        return BasicValue(std::vector<BasicValue>());

    if (arguments.size() == 1) {
        if (arguments[0].type == BasicValue::Type::LIST)
            return arguments[0];
        else
            throw std::runtime_error("list() can only be used on lists");
    }

    std::vector<BasicValue> values;

    for (auto &arg: arguments) {
        if (arg.type != BasicValue::Type::INT)
            throw std::runtime_error("list() can only be used on integers");

        values.emplace_back(arg);
    }

    return BasicValue(values);
}

// A list of all function names and their corresponding function pointers.
const std::map<std::string, void *> functionMap = {
        {"print",     (void *) &print},
        {"println",   (void *) &println},
        {"input",     (void *) &input},
        {"os",        (void *) &os},
        {"exit",      (void *) &exit_},
        {"len",       (void *) &len},
        {"readFile",  (void *) &readFile},
        {"writeFile", (void *) &writeFile},
        {"range",     (void *) &range},
        {"list",      (void *) &list},
};

BasicValue executeFunction(const std::string& name, const std::vector<BasicValue>& arguments) {
    // Searching for the function
    auto function = functionMap.find(name);

    // If the function was found
    if (function != functionMap.end()) {
        // Calling the function
        return ((BasicValue(*)(const std::vector<BasicValue> &)) function->second)(arguments);
    }

    // If the function was not found
    return BasicValue(-1);
}

bool function_exists(const std::string& name) {
    // Searching for the function
    auto function = functionMap.find(name);

    // If the function was found
    if (function != functionMap.end()) {
        return true;
    }

    return false;
}