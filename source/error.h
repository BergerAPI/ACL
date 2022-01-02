//
// Created by peu77 on 1/2/22.
//

#ifndef ACL_ERROR_H
#define ACL_ERROR_H
#include "iostream"
#include "vector"

using namespace std;


enum ErrorType {
    FATAL, WARNING
};

void throwError(ErrorType type, const string message, string line, string code, string errorPointer, string helpMessage,int errorStart, int errorEnd);


#endif //ACL_ERROR_H
