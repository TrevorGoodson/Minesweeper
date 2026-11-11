#pragma once
#include "msolver.h"

// public
msolver::msolver(vector<int> board, int height, int width) :
    height(height),
    width(width),
    size(height * width),
    board(board),
    status_board(vector<int>(size, UNKNOWN)),
    unsolved(map<int, unsolved_square>()),
    fresh(stack<int>()),
    neighbor_buffer(vector<int>(8))
{}

vector<int> msolver::solve(int start_row, int start_col) {
    int first_guess = index_from_coords(start_row, start_col);
    last_square_compared = first_guess;
    uncover(first_guess);
    resolve_fresh_squares();
    return status_board;
}

// private
int msolver::get_neighbors(int index, int status) {
    coord c = coords_from_index(index);
    int row = c.row + 1;
    int col = c.col + 1;

    int num_neighbors = 0;
    for (int i = 0; i < 8; ++i) {
        ((i & 2) ? row : col) += (i < 4) ? -1 : 1;

        if (row < 0 || row >= height || col < 0 || col >= width)
            continue;
        
        int neighbor = index_from_coords(row, col);
        if (status_board[neighbor] == status) {
            neighbor_buffer[num_neighbors] = neighbor;
            ++num_neighbors;
        }
    }

    return num_neighbors;
}

int msolver::get_squares_with_overlapping_clues(int index) {
    coord c = coords_from_index(index);
    int row = c.row;
    int col = c.col;

    int num_squares = 0;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; i < 3; ++j) {
            if (i < 0 || i >= height || j < 0 || j >= width)
                continue;
            
            int nearby_square = index_from_coords(row + i, col + j);
            int status = status_board[nearby_square];
            if (status == FRESH || status == UNSOLVED) {
                neighbor_buffer[num_squares] = nearby_square;
                ++num_squares;
            }
        }
    }
    return num_squares;
}

bool msolver::compare_clues(unsolved_square& square1, int clue_num1, unsolved_square& square2, int clue_num2) {
    auto solo1 = square1.clues[clue_num1].squares;
    auto shared = vector<int>();
    auto solo2 = square2.clues[clue_num2].squares;

    int i = 0;
    while (i < solo1.size()) {
        bool is_shared = false;
        for (int j = 0; j < solo2.size(); ++ j) {
            if (solo1[i] == solo2[j]) {
                shared.push_back(solo1[i]);
                solo1.erase(solo1.begin() + i);
                solo2.erase(solo2.begin() + j);
                is_shared = true;
                break;
            }
        }
        i += (is_shared) ? 0 : 1;
    }

    if (shared.empty()) {
        return false;
    }

    int min1 = square1.clues[clue_num1].min;
    int min2 = square2.clues[clue_num2].min;
    int max1 = square1.clues[clue_num1].max;
    int max2 = square2.clues[clue_num2].max;

    int shared_min = 0;
    if (solo1.size() < min1) {
        shared_min = min1 - solo1.size();
    } else if (shared.size() < min1) {
        shared_min = min1 - shared.size();
    } else if (solo2.size() < min2) {
        shared_min = min2 - solo2.size();
    } else if (shared.size() < min2) {
        shared_min = min2 - shared.size();
    } else {
        return false;
    }
    
    int shared_max = (max1 > max2) ? max1 : max2;
    int new_min1 = min1 - shared_max;
    int new_max1 = max1 - shared_min;
    // This is a "perfect split"
    if (new_min1 + shared_min == min1 && new_max1 + shared_max == max1) {
        square1.clues.erase(square1.clues.begin() + clue_num1);
        square2.clues.erase(square2.clues.begin() + clue_num2);
    }
    
    square1.clues.push_back({
        shared_min,
        shared_max,
        shared
    });
    square1.clues.push_back({
        min1 - shared_max,
        max1 - shared_min,
        solo1
    });
    square2.clues.push_back({
        min2 - shared_max,
        min2 - shared_min,
        solo2
    });

    return true;
}

bool msolver::compare_squares(int index1, int index2) {
    unsolved_square * sq1 = &unsolved[index1];
    unsolved_square * sq2 = &unsolved[index2];

    vector<clue> * clues1 = &sq1->clues;
    vector<clue> * clues2 = &sq2->clues;

    bool susuccessful_comparison = false;
    for (int i = 0; i < clues1->size() && !susuccessful_comparison; ++i) {
        for (int j = 0; j < clues2->size() && !susuccessful_comparison; ++j) {
            susuccessful_comparison = compare_clues(*sq1, i, *sq2, j);
        }
    }

    if (susuccessful_comparison) {
        fresh.push(index1);
        fresh.push(index2);
        status_board[index1] = FRESH;
        status_board[index2] = FRESH;
        return true;
    }
    return false;
}

void msolver::freshen_neighbors(int index) {
    int num_neigbors = get_neighbors(index, UNSOLVED);
    for (int i = 0; i < num_neigbors; ++i) {
        board[neighbor_buffer[i]] = FRESH;
        fresh.push(neighbor_buffer[i]);
    }
}

void msolver::uncover(int index) {
    int square_val = board[index];

    #ifdef INCLUDE_ERRORS
        if  (status_board[index] != UNKNOWN)
            throw runtime_error("Tried to uncover a known square");
        if (square_val == MINE)
            throw runtime_error("Tried to uncover a mine");
    #endif

    int num_solved = get_neighbors(index, SOLVED);
    for (int i = 0; i < num_solved; ++i) {
        if (board[neighbor_buffer[i]] == MINE) {
            --square_val;
        }
    }

    int num_squares = get_neighbors(index, UNKNOWN);

    status_board[index] = FRESH;
    unsolved.emplace(index,
        unsolved_square {
            index,
            vector<clue>{clue {
                square_val,
                square_val,
                copy_neighbor_buffer(num_squares)
            }},
        }
    );
    fresh.push(index);

    freshen_neighbors(index);
}

void msolver::mark_mine(int index) {
    #ifdef INCLUDE_ERRORS
        if (board[index] != MINE)
            throw runtime_error("Falsly identified square as mine");
    #endif

    status_board[index] = SOLVED;

    freshen_neighbors(index);
}

void msolver::resolve_clues(unsolved_square& square) {
    /*
        Each iteration does one of 3 things:
        - increments i (the clue in question was not resolved)
        - removes the clue & resets i to 0 (clue is resolved; new squares are uncovered)
        - removes the clue & does not adjust i (clue is resolved; no new squares are uncovered)
    */
    int i = 0;
    while (i < square.clues.size()) {
        vector<int> * clue_squares = &square.clues[i].squares;

        // Filter out revealed squares
        int j = 0;
        while (j < clue_squares->size()) {
            int sq = (*clue_squares)[j];
            // if (square.index == 14) {
            //     cout << " - square: " << sq << " status: " << status_board[sq] << endl;
            // }
            if (status_board[sq] != UNKNOWN) {
                clue_squares->erase(clue_squares->begin() + j);
                if (board[sq] == MINE) {
                    square.clues[i].min -= 1;
                    square.clues[i].max -= 1;
                }                    
            } else {
                ++j;
            }
        }

        int max = square.clues[i].max;
        int min = square.clues[i].min;
        int num_squares = clue_squares->size();
        square.clues[i].max = (max > num_squares) ? num_squares : max; 

        #ifdef INCLUDE_ERRORS
            if (num_squares < min)
                throw runtime_error("Clue has fewer squares than min.");
        #endif

        // 3 conditions to remove the clue:

        // 1) there are no more squares
        if (num_squares == 0) {
            square.clues.erase(square.clues.begin() + i);
        }

        // 2) there are no mines
        else if (max == 0) {
            for (int sq : *clue_squares) {
                uncover(sq); 
            }

            square.clues.erase(square.clues.begin() + i);
            i = 0;
        }

        // all squares are mines
        else if (num_squares == min) {
            for (int sq : *clue_squares) {
                mark_mine(sq);
            }

            square.clues.erase(square.clues.begin() + i);
            i = 0;
        }

        else {
            ++i;
        }
    }
}

void msolver::resolve_fresh_squares() {
    while (!fresh.empty()) {
        int square = fresh.top();
        fresh.pop();

        #ifdef INCLUDE_ERRORS
            if (unsolved.find(square) == unsolved.end())
                throw runtime_error("Fresh square not found in unsolved");
        #endif

        resolve_clues(unsolved[square]);

        status_board[square] = UNSOLVED;
        if (unsolved[square].clues.empty()) {
            unsolved.erase(square);
            status_board[square] = SOLVED;
        }
    }
}

void msolver::resolve_unsolved() {
    int current_square = last_square_compared;
    while (true) {
        resolve_fresh_squares();
        if (unsolved.empty())
            break;
        int num_squares = get_squares_with_overlapping_clues(current_square);
        for (int i = 0; i < num_squares; ++i) {
            int new_clues = compare_squares(current_square, neighbor_buffer[i]);
            if (new_clues) {

                break;
            }
        }
    }
}