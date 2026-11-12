#pragma once
#include <iomanip>
#include <iostream>
#include <random>

inline int random_number(int a) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, a);
    return distr(gen);
}

template<class T>
void PrintBoard(std::vector<std::vector<T>> inputBoard) {
    for (auto& row : inputBoard) {
        for (auto& column : row) {
            std::cout << std::setw(3) << column;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

inline char intToLowerCase(int a) {
    return a + 'a';
}

inline char intToUpperCase(int a) {
    return a + 'A';
}

inline int letterToInt(char a) {
    a = tolower(a);
    a -= 'a';
    return a;
}

inline int digitToInt(char a) {
    return a - '0';
}

inline void clearScreen() {
    //OS specific function
    system("cls");
}

static void print_int_vector(std::vector<int> vec) {
    std::cout << "[";
    for (int v : vec) {
        std::cout << v << ", ";
    }
    std::cout << "]" << std::endl;
}