//
// Created by peu77 on 1/2/22.
//

#include "utils.h"


vector<string> splitString(string text, string delim) {
    vector<string> result;
    auto start = 0U;
    auto end = text.find(delim);
    while (end != std::string::npos) {
        result.push_back(text.substr(start, end - start));
        start = end + delim.length();
        end = text.find(delim, start);
    }
    result.push_back(text.substr(start, end));
    return result;
}
