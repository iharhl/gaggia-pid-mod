#ifndef MYPRINT_H
#define MYPRINT_H

#include <string>

template <typename T>
void printForGraph(const char* str, T value) {
    printf("%s: %6.2f | ", str, static_cast<float>(value));
}

template <typename T>
void printDebug(const char* str, T value) {
    printf("%s: %s | ", str, std::to_string(value).c_str());
}

template <typename T>
void printFloat(T value) {
    // Format - 3 digits before comma, 2 digits after
    printf("%3.2f ", static_cast<float>(value));
}

#endif //MYPRINT_H
