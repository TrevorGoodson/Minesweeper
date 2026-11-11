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
    neighbor_buffer(vector<int>(8)),
    n_gen(height, width)
{}

vector<int> msolver::solve(int start_row, int start_col) {
    int first_guess = index_from_coords(start_row, start_col);
    current_square = first_guess;
    uncover(first_guess);
    resolve_unsolved();
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

    // cout << "checking square at ";
    // c.print();
    // cout << " for neighbors\n";

    int num_squares = 0;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (i == 0 && j == 0)
                continue;
            int row = c.row + i;
            int col = c.col + j;

            if (row < 0 || row >= height || col < 0 || col >= width)
                continue;
            
            int nearby_square = index_from_coords(row, col);
            int status = status_board[nearby_square];
            if (status == FRESH || status == UNSOLVED) {
                neighbor_buffer[num_squares] = nearby_square;
                ++num_squares;
            }
        }
    }
    // cout << "found neighbors!\n";
    return num_squares;
}

bool msolver::compare_clues(unsolved_square& square1, int clue_num1, unsolved_square& square2, int clue_num2) {
    clue solo1 = clue{
        square1.clues[clue_num1].min,
        square1.clues[clue_num1].max,
        square1.clues[clue_num1].squares
    };
    clue solo2 = clue {
        square2.clues[clue_num2].min,
        square2.clues[clue_num2].max,
        square2.clues[clue_num2].squares
    };
    clue shared = clue { 0, 8, vector<int>() };

    // This loop splits the two lists of squares into 3: 1 shared and 2 exclusive (1 for each clue)
    int i = 0;
    while (i < solo1.squares.size()) {
        bool is_shared = false;
        for (int j = 0; j < solo2.squares.size(); ++j) {
            if (solo1.squares[i] == solo2.squares[j]) {
                shared.squares.push_back(solo1.squares[i]);
                solo1.squares.erase(solo1.squares.begin() + i);
                solo2.squares.erase(solo2.squares.begin() + j);
                is_shared = true;
                break;
            }
        }
        i += (is_shared) ? 0 : 1;
    }

    if (shared.squares.empty()) {
        return false;
    }

    if (solo1.squares.empty() && solo2.squares.empty()) {
        return false;
    }

    // These are the constraints that each clue puts on the shared squares
    // Keep in mind the min/max are still the original
    int shared_min1 = max(int(solo1.min - solo1.squares.size()), 0);
    int shared_max1 = min(int(shared.squares.size()), solo1.max);
    int shared_min2 = max(int(solo2.min - solo2.squares.size()), 0);
    int shared_max2 = min(int(shared.squares.size()), solo2.max);

    // This updates clues 1 & 2 to now represent just the squares outside of the shared squares
    solo1.min = max(int(solo1.min - shared.squares.size()), 0);
    solo1.max = min(int(solo1.squares.size()), solo1.max);
    solo2.min = max(int(solo2.min - shared.squares.size()), 0);
    solo2.max = min(int(solo2.squares.size()), solo2.max);

    bool shared_updated = false;
    bool solo1_updated = false;
    bool solo2_updated = false;

    int shared_min_diff = shared_min1 - shared_min2;
    if (shared_min_diff < 0) {
        int old_max = solo1.max;
        solo1.max = min(int(solo1.squares.size()), square1.clues[clue_num1].max + shared_min_diff);
        shared.min = shared_min2;
        solo1_updated = old_max != solo1.max;
        shared_updated = true;
    } else if (shared_min_diff > 0) {
        int old_max = solo2.max;
        solo2.max = min(int(solo2.squares.size()), square2.clues[clue_num2].max - shared_min_diff);
        shared.min = shared_min1;
        solo2_updated = old_max != solo2.max;
        shared_updated = true;
    } else {
        shared.min = shared_min1;
    }

    int shared_max_diff = shared_max1 - shared_max2;
    if (shared_max_diff > 0) {
        int old_min = solo1.min;
        solo1.min = max(int(square1.clues[clue_num1].min + shared_max_diff - shared.squares.size()), 0);
        shared.max = shared_max2;
        solo1_updated = old_min != solo1.min;
        shared_updated = true;
    } else if (shared_max_diff < 0) {
        int old_min = solo2.min;
        solo2.min = max(int(square2.clues[clue_num2].min - shared_max_diff - shared.squares.size()), 0);
        shared.max = shared_max1;
        solo2_updated = old_min != solo2.min;
        shared_updated = true;
    } else {
        shared.max = shared_max1;
    }

    if (!shared_updated) {
        return false;
    }

    // TODO: check for a "perfect split" (not necessary, but makes things more efficient)

    // We can put the shared clue in either
    bool square1_updated = insert_if_unique(square1.clues, shared);
    if (square1_updated) shared.print();
    if (solo1_updated) {
        if (insert_if_unique(square1.clues, solo1)) {
            solo1.print();
            square1_updated = true;
        }
        //square1_updated = insert_if_unique(square1.clues, solo1) || square1_updated;
    }

    bool square2_updated = false;
    if (solo2_updated) {
        square2_updated = insert_if_unique(square2.clues, solo2);
    }
    if (square2_updated) solo2.print();

    return square1_updated || square2_updated;
}

bool msolver::compare_squares(int index1, int index2) {
    bool successful_comparison = false;
    for (int i = 0; i < unsolved[index1].clues.size() && !successful_comparison; ++i) {
        for (int j = 0; j < unsolved[index2].clues.size() && !successful_comparison; ++j) {
            clue old = unsolved[index1].clues[i];
            clue old2 = unsolved[index2].clues[j];

            successful_comparison = compare_clues(unsolved[index1], i, unsolved[index2], j);
            
            if (successful_comparison) {
                cout << " Successful comparison:  ";
                coords_from_index(index1).print();
                cout << " ( clue: ";
                old.print();
                cout << ") and ";
                coords_from_index(index2).print();
                cout << " ( clue: ";
                old2.print();
                cout << ")\n";
            }
        }
    }

    #ifdef INCLUDE_ERRORS
        if  (unsolved.find(index1) == unsolved.end() || unsolved.find(index2) == unsolved.end())
            throw runtime_error("Compare squares called with squares not found in unsolved");
    #endif

    if (successful_comparison) {
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
        #ifdef INCLUDE_ERRORS
            if  (unsolved.find(neighbor_buffer[i]) == unsolved.end())
                throw runtime_error("Square marked as unsolved on board is not found in map");
        #endif

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

    cout << "No mine: ";
    coords_from_index(index).print();
    cout << endl;

    vector<int> clue_squares; clue_squares.reserve(6);
    n_gen.start(index); int neighbor;
    do {
        neighbor = n_gen.next();
        if (status_board[neighbor] == UNKNOWN) {
            clue_squares.push_back(neighbor);
        }
        if (board[neighbor] == MINE && status_board[neighbor] == SOLVED) {
            --square_val;
        }
    } while (neighbor != n_gen.DONE);

    // int num_solved = get_neighbors(index, SOLVED);
    // for (int i = 0; i < num_solved; ++i) {
    //     if (board[neighbor_buffer[i]] == MINE) {
    //         --square_val;
    //     }
    // }

    // int num_squares = get_neighbors(index, UNKNOWN);

    status_board[index] = FRESH;
    unsolved.insert({index,
        unsolved_square{
            index,
            vector<clue>{clue{
                square_val,
                square_val,
                clue_squares
                //copy_neighbor_buffer(num_squares)
            }},
        }
    });
    fresh.push(index);

    freshen_neighbors(index);
}

void msolver::mark_mine(int index) {
    #ifdef INCLUDE_ERRORS
        if  (status_board[index] != UNKNOWN)
            throw runtime_error("Tried to mark a known square as a mine");
        if (board[index] != MINE)
            throw runtime_error("Falsly identified square as mine");
    #endif

    cout << "Mine: ";
    coords_from_index(index).print();
    cout << endl;

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

void msolver::iterate_current_square() {
    auto square_itr = unsolved.find(current_square);

    #ifdef INCLUDE_ERRORS
        if (square_itr == unsolved.end())
            throw runtime_error("Current square not found in unsolved");
    #endif

    ++square_itr;
    if (square_itr == unsolved.end())
        square_itr = unsolved.begin();
    current_square = square_itr->first;
}

void msolver::resolve_fresh_squares() {
    while (!fresh.empty()) {
        int square = fresh.top();
        fresh.pop();

        if (unsolved.find(square) == unsolved.end()) {
            // I DONT KNOW WHY THIS HAPPENS - IT SHOULDN'T
            // Just ignoring it doesn't seem to cause any issues though
            continue;
        }

        #ifdef INCLUDE_ERRORS
            if (unsolved.find(square) == unsolved.end()) {
                coords_from_index(square).print();
                cout << endl << status_board[square] << endl;
                cout << board[square] << endl;
                throw runtime_error("Fresh square not found in unsolved");
            }
        #endif

        resolve_clues(unsolved[square]);

        status_board[square] = UNSOLVED;
        if (unsolved[square].clues.empty()) {
            if (square == current_square) {
                iterate_current_square();
            }
            unsolved.erase(square);
            status_board[square] = SOLVED;
        }
    }
}

void msolver::resolve_unsolved() {
    int num_itrs_no_updates = 0;
    int maxitr = 200;
    int itr = 0;
    // loops until every unsolved square has been compared to neighbors with no updates
    while (num_itrs_no_updates < unsolved.size() && itr < maxitr) {
        ++itr;
        ++num_itrs_no_updates;

        if (!fresh.empty()) {
            num_itrs_no_updates = 0;
            resolve_fresh_squares();
            if (unsolved.empty()) {
                cout << "no unsolved squares" << endl;
                break;
            }
        }

        int num_squares = get_squares_with_overlapping_clues(current_square);
        // cout << "num squares: " << num_squares << endl;
        for (int i = 0; i < num_squares; ++i) {
            int new_clues = compare_squares(current_square, neighbor_buffer[i]);
            if (new_clues) {
                num_itrs_no_updates = 0;
                break;
            }
        }

        iterate_current_square();
    }

    if (itr >= maxitr) {
        cout << "max itr hit" << endl;
    }
}

msolver::neighbor_generator::neighbor_generator(int height, int width) : 
    height(height),
    width(width),
    DONE(height * width)
{}

void msolver::neighbor_generator::start(int index) {
    row = int(index / width) + 1;
    col = int(index % width) + 1;
    i = 0;
}

int msolver::neighbor_generator::next() {
    while (i < 8) {
        ((i & 2) ? row : col) += (i < 4) ? -1 : 1;
        ++i;

        if (row < 0 || row >= height || col < 0 || col >= width)
            continue;
        
        return row * width + col;
    }

    return DONE;
}