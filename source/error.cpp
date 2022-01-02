//
// Created by peu77 on 1/2/22.
//

#include "error.h"
#include "utils.h"

void throwError(ErrorType type, const string message, string line, string code, string errorPointer, string helpMessage,int errorStart, int errorEnd) {
    cout << getColor(Color::FG_RED) << (type == ErrorType::WARNING ? "WARNING" : "ERROR") << getColor(Color::RESET)
         << ": ";
    cout << message << endl;
    cout << getColor(Color::FG_BLUE) << "--> " << getColor(Color::FG_GRAY) << endl;
    cout << endl;
    cout << "       " << code << endl;

}