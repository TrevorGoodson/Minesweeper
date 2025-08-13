#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <windows.h>
#include "helperFunctions.h"
using namespace std;

class board {
public:
    board();
    void PrintUI();
    void GetGuess();
    bool GetHealth() const;
    bool DidWin() const;
    void ShowXs();
private:
    enum play_type {
        GUESS = 0,
        PLACE_FLAG = 1
    };
    enum SearchAround_key {
        SET_NUMBERS = 0,
        MULTI_GUESS = 1,
        COUNT_FLAGS = 2,
        COUNT_UNKNOWNS = 3,
        MULTI_FLAG = 4
    };
    enum whatBorder_key {
        CORNER = 0,
        VERTICAL = 1,
        HORIZONTAL = 2
    };
    enum character_symbols {
        ZERO = '0',
        UNKNOWN_SQUARE = ' ',
        MINE = 'X',
        FLAG = '*'
    };

    class display {
    public:
        display(board& outer);
        void PrintUI();
    private:
        board& outer;

        void PrintColumnLabel();
        void PrintRowType1(int row);
        void PrintRowType2(int row);
        vector<string> WhatBorder(int row, int column) const;
        bool HasBorder(int row, int column) const;
    };

    friend class display;
    display disp;

    int height;
    int length;
    double mineDensity;
    int numMines;
    int numMarkedMines;
    bool isDead;
    vector<vector<char>> playerBoard;
    vector<vector<char>> playerUI;
    set<vector<int>> resolvedSquares;
    const vector<string> borderKey = {" ", "│", "───────", "╮", "", "", "╭", "┬", "", "╯", "", "┤", "╰", "┴", "├", "┼"};
    //const vector<string> borderKey = {" ", "|", "-------", "+", "", "", "+", "+", "", "+", "", "+", "+", "+", "+", "+"};

    void SetBoard(int row, int column);
    void PlayGuess(int row, int column);
    void RevealSquare(int row, int column);
    void PlayFlag(int row, int column);
    vector<int> GetInput();
    void SearchAround(int row, int column, int whatToDo);
};

