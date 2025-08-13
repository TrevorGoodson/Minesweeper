#pragma once

#include "board.h"

// public:

board::display::display(board& outer) : outer(outer) {}

void board::display::PrintUI() {
    clearScreen();
    PrintColumnLabel();
    for (int row = 0; row < outer.height; ++row) {
        PrintRowType1(row);
        PrintRowType2(row);
    }
    PrintRowType1(outer.height);
    PrintColumnLabel();
}

// private:

void board::display::PrintColumnLabel() {
    char c = 'A';
    cout << "  ";
    for (int i = 0; i < outer.length; ++i) {
        cout << "    " << c << "   ";
        ++c;
    }
    cout << '\n';
}

void board::display::PrintRowType1(int row) {
    cout << "  ";
    for (int column = 0; column < outer.length; ++column) {
        vector<string> border = WhatBorder(row, column);
        cout << border.at(CORNER);
        cout << border.at(HORIZONTAL);
    }
    cout << WhatBorder(row, outer.length).at(CORNER) << "\n";
}

void board::display::PrintRowType2(int row) {
    cout << intToLowerCase(row) << " ";
    for (int column = 0; column < outer.length; ++column) {
        vector<string> border = WhatBorder(row, column);
        cout << border.at(VERTICAL);
        cout << "  ";
        switch (outer.playerUI.at(row).at(column)) {
            case UNKNOWN_SQUARE:
                if (!outer.isDead) {
                    cout << intToLowerCase(row) << " " << intToUpperCase(column);
                    break;
                }
            //fall through
            case ZERO:
                cout << "   ";
            break;
            default:
                cout << " " << outer.playerUI.at(row).at(column) << " ";
            break;
        }
        cout << "  ";
    }
    cout << WhatBorder(row, outer.length).at(VERTICAL) << " " << intToLowerCase(row) << '\n';
}

vector<string> board::display::WhatBorder(int row, int column) const {
    vector<string> whatBorders(3);

    bool downRight = (row != outer.height && column != outer.length && HasBorder(row, column));
    bool upRight = (row != 0 && column != outer.length && HasBorder(row - 1, column));
    bool downLeft = (row != outer.height && column != 0 && HasBorder(row, column - 1));
    bool upLeft = (row != 0 && column != 0 && HasBorder(row - 1, column - 1));

    bool up = upLeft || upRight;
    bool down = downLeft || downRight;
    bool left = upLeft || downLeft;
    bool right = upRight || downRight;

    whatBorders.at(VERTICAL) = down ? outer.borderKey.at(1) : " ";
    whatBorders.at(HORIZONTAL) = right ? outer.borderKey.at(2) : "       ";

    int borderNum = up * 8 + right * 4 + down * 2 + left;
    whatBorders.at(CORNER) = outer.borderKey.at(borderNum);
    return whatBorders;
}

bool board::display::HasBorder(int row, int column) const {
    return outer.playerUI.at(row).at(column) == UNKNOWN_SQUARE ||
           outer.playerUI.at(row).at(column) == FLAG ||
           outer.playerUI.at(row).at(column) == MINE;
}