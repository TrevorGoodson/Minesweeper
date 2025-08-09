#pragma once
#include "board.h"
using namespace std;

//public:

board::board() :
    height(17),
    length(14),
    mineDensity(0.17),
    numMines(height * length * mineDensity),
    numMarkedMines(0),
    isDead(false),
    playerUI(height, vector<char>(length, UNKNOWN_SQUARE))
{}

void board::PrintUI() {
    //ClearScreen();
    PrintColumnLabel();
    for (int row = 0; row < height; ++row) {
        PrintRowType1(row);
        PrintRowType2(row);
    }
    PrintRowType1(height);
    PrintColumnLabel();
}

void board::GetGuess() {
    vector<int> input = GetInput();
    int row = input.at(0);
    int column = input.at(1);
    int playType = input.at(2);

    if (playType == GUESS) {
        PlayGuess(row,column);
    } else { //PLACE_FLAG
        PlayFlag(row,column);
    }
}

bool board::GetHealth() const {
   return isDead;
}

bool board::DidWin() const {
    if (playerBoard.empty()) {
        return false;
    }
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < length; ++column) {
            if (playerUI.at(row).at(column) == UNKNOWN_SQUARE && playerBoard.at(row).at(column) != MINE) {
                return false;
            }
            if (playerUI.at(row).at(column) == FLAG && playerBoard.at(row).at(column) != MINE) {
                return false;
            }
        }
    }
    return true;
}

void board::ShowXs() {
for (int row = 0; row < height; ++row) {
    for  (int column = 0; column < length; ++column) {
            if (playerBoard.at(row).at(column) ==MINE) {
                playerUI.at(row).at(column) = MINE;
            }
        }
    }
}

//private:

void board::SetBoard(int row, int column) {
    playerBoard = vector<vector<char>>(height, vector<char>(length, ZERO));

    int numPlacedMines = 0;
    while (numPlacedMines < numMines) {
        int randomRow = random_number(height - 1);
        int randomColumn = random_number(length - 1);

        if (randomRow >= row - 1 &&
            randomRow <= row + 1 &&
            randomColumn >= column - 1 &&
            randomColumn <= column + 1) {
            continue;
        }

        if (playerBoard.at(randomRow).at(randomColumn) == ZERO) {
            playerBoard.at(randomRow).at(randomColumn) = MINE;
            ++numPlacedMines;
        }
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < length; ++j) {
            if (playerBoard.at(i).at(j) != MINE) {
                SearchAround(i, j, SET_NUMBERS);
            }
        }
    }

    RevealSquare(row, column);
}

void board::PlayGuess(int row, int column) {
    if (playerBoard.empty()) {
        SetBoard(row, column);
    }
    if (playerUI.at(row).at(column) != UNKNOWN_SQUARE) {
        if (playerUI.at(row).at(column) != FLAG && playerUI.at(row).at(column) != ZERO) {
            SearchAround(row, column, COUNT_FLAGS);
        }
        return;
    }

    RevealSquare(row, column);
}

void board::RevealSquare(int row, int column) {
    playerUI.at(row).at(column) = playerBoard.at(row).at(column);
    if (playerUI.at(row).at(column) == MINE) {
        isDead = true;
    }
    if (playerUI.at(row).at(column) == ZERO) {
        SearchAround(row, column, MULTI_GUESS);
    }
}

void board::PlayFlag(int row, int column) {
    switch (playerUI.at(row).at(column)) {
        case UNKNOWN_SQUARE:
            playerUI.at(row).at(column) = FLAG;
            ++numMarkedMines;
            break;
        case FLAG:
            playerUI.at(row).at(column) = UNKNOWN_SQUARE;
            --numMarkedMines;
            break;
        default:
            SearchAround(row, column, COUNT_UNKNOWNS);
            break;
    }
}

vector<string> board::WhatBorder(int row, int column) const {
    vector<string> whatBorders(3);

    bool downRight = (row != height && column != length && HasBorder(row, column));
    bool upRight = (row != 0 && column != length && HasBorder(row - 1, column));
    bool downLeft = (row != height && column != 0 && HasBorder(row, column - 1));
    bool upLeft = (row != 0 && column != 0 && HasBorder(row - 1, column - 1));

    bool up = upLeft || upRight;
    bool down = downLeft || downRight;
    bool left = upLeft || downLeft;
    bool right = upRight || downRight;

    whatBorders.at(VERTICAL) = down ? borderKey.at(1) : " ";
    whatBorders.at(HORIZONTAL) = right ? borderKey.at(2) : "       ";

    int borderNum = up * 8 + right * 4 + down * 2 + left;
    whatBorders.at(CORNER) = borderKey.at(borderNum);
    return whatBorders;
}

bool board::HasBorder(int row, int column) const {
    return playerUI.at(row).at(column) == UNKNOWN_SQUARE ||
           playerUI.at(row).at(column) == FLAG ||
           playerUI.at(row).at(column) == MINE;
}

void board::PrintColumnLabel() {
    char c = 'A';
    cout << "  ";
    for (int i = 0; i < length; ++i) {
        cout << "    " << c << "   ";
        ++c;
    }
    cout << '\n';
}

void board::PrintRowType1(int row) {
    cout << "  ";
    for (int column = 0; column < length; ++column) {
        vector<string> border = WhatBorder(row, column);
        cout << border.at(CORNER);
        cout << border.at(HORIZONTAL);
    }
    cout << WhatBorder(row, length).at(CORNER) << "\n";
}

void board::PrintRowType2(int row) {
    cout << intToLowerCase(row) << " ";
    for (int column = 0; column < length; ++column) {
        vector<string> border = WhatBorder(row, column);
        cout << border.at(VERTICAL);
        cout << "  ";
        switch (playerUI.at(row).at(column)) {
            case UNKNOWN_SQUARE:
                if (!isDead) {
                    cout << intToLowerCase(row) << " " << intToUpperCase(column);
                    break;
                }
            //fall through
            case ZERO:
                cout << "   ";
            break;
            default:
                cout << " " << playerUI.at(row).at(column) << " ";
            break;
        }
        cout << "  ";
    }
    cout << WhatBorder(row, length).at(VERTICAL) << " " << intToLowerCase(row) << '\n';
}

vector<int> board::GetInput() {
    vector<int> inputCoords(3);

    bool inputValid = false;
    while (!inputValid) {
        string input;
        cin >> input;

        if (input.size() != 2 && input.size() != 3) {
            continue;
        }
        if (!isalpha(input.at(0)) || !isalpha(input.at(1))) {
            continue;
        }
        inputCoords.at(0) = letterToInt(input.at(0));
        inputCoords.at(1) = letterToInt(input.at(1));
        if (inputCoords.at(0) >= height || inputCoords.at(1) >= length) {
            continue;
        }
        inputCoords.at(2) = (input.size() == 2) ? GUESS : PLACE_FLAG;

        inputValid = true;
    }

    return inputCoords;
}

void board::SearchAround(int row, int column, int whatToDo) {
    int numberMarkedMines = 0;
    int numUnknowns = 0;
    int row1 = row + 1;
    int column1 = column + 1;

    if (whatToDo == MULTI_GUESS) {
        resolvedSquares.insert({row,column});
    }

    for (int i = 0; i < 8; ++i) {
        ((i % 4 / 2) ? row1 : column1) += (i / 4 * 2 - 1);

        if (row1 < 0 || row1 >= height || column1 < 0 || column1 >= length) {
            continue;
        }

        switch (whatToDo) {
            case MULTI_FLAG:
                if (playerUI.at(row1).at(column1) == UNKNOWN_SQUARE) {
                    playerUI.at(row1).at(column1) = FLAG;
                    ++numMarkedMines;
                }
                break;
            case COUNT_UNKNOWNS:
                if (playerUI.at(row1).at(column1) == UNKNOWN_SQUARE) {
                    ++numUnknowns;
                    break;
                }
                //fallthrough
            case COUNT_FLAGS:
                if (playerUI.at(row1).at(column1) == FLAG) {
                    ++numberMarkedMines;
                }
                break;
            case MULTI_GUESS:
                //if (resolvedSquares.contains({row1, column1})) {
                if (resolvedSquares.find({row1, column1}) != resolvedSquares.end()) {
                    break;
                }
                if (playerUI.at(row1).at(column1) == FLAG) {
                    break;
                }
                RevealSquare(row1, column1);
                break;
            default: //SET_NUMBERS
                if (playerBoard.at(row1).at(column1) == MINE) {
                    ++playerBoard.at(row).at(column);
                }
                break;
        }
    }

    switch (whatToDo) {
        case COUNT_UNKNOWNS:
            if (numberMarkedMines + numUnknowns == digitToInt(playerUI.at(row).at(column))) {
                SearchAround(row,column,MULTI_FLAG);
            }
            break;
        case COUNT_FLAGS:
            if (numberMarkedMines == digitToInt(playerUI.at(row).at(column))) {
                SearchAround(row, column, MULTI_GUESS);
            }
            break;
        default: //SET_NUMBERS, MULTI_FLAG, MULTI_GUESS
            break;
    }
}