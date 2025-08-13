#pragma once
#include <iomanip>
#include <iostream>
#include <random>
using namespace std;

inline int random_number(int a) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(0, a);
    return distr(gen);
}

template<class T>
void PrintBoard(vector<vector<T>> inputBoard) {
    for (auto& row : inputBoard) {
        for (auto& column : row) {
            cout << setw(3) << column;
        }
        cout << endl;
    }
    cout << endl;
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