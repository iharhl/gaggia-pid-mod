#ifndef MYPRINT_H
#define MYPRINT_H

#include <string>

template <typename T>
void printdebug(const char* str, T value) {
    printf("%s: %s\n", str, std::to_string(value).c_str());
}

template <typename T>
void printfloat(T value) {
    // Format - 3 digits before comma, 2 digits after
    printf("%3.2f ", static_cast<float>(value));
}

#endif //MYPRINT_H
