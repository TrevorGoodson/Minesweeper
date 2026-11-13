#include "msolver.h"
using std::vector; using std::stack;
using std::map;    using std::set;
using std::min;    using std::max;
using std::cout;   using std::endl;
using std::runtime_error;

// public
msolver::msolver(vector<int> board, int height, int width) :
    height(height),
    width(width),
    size(height * width),
    OUT_OF_RANGE(size),
    board(board),
    status_board(vector<Status>(size, Status::UNKNOWN)),
    solution_graph(vector<vector<int>>(size)),
    unknown_border(set<int>()),
    unsolved(map<int, unsolved_square>()),
    fresh(stack<int>()),
    n_gen(height, width),
    n_gen2(height, width),
    sc_gen(height, width)
{}

vector<msolver::Status> msolver::solve(int start_row, int start_col) {
    first_guess = index_from_coords(start_row, start_col);
    current_square = first_guess;
    uncover(first_guess);
    resolve_unsolved();

    int i = 0; int max_iter = 100;
    while (!unknown_border.empty() && i < max_iter) {
        ++i;
        change_one_square();
        resolve_unsolved();
    }
    if (i == max_iter) {
        cout << "max changes hit" << endl;
    }

    // cout << "\nInitial solution graph:" << endl;
    // for (int i = 0; i < solution_graph.size(); ++i) {
    //     cout << disp(i) << ": ";
    //     for (auto x : solution_graph[i]) {
    //         cout << disp(x) << " ";
    //     }
    //     cout << endl;
    // }

    unsolved = map<int, unsolved_square>();
    solution_graph = vector<vector<int>>(size);
    unknown_border = set<int>();
    status_board = vector<Status>(size, Status::UNKNOWN);
    current_square = first_guess;

    cout << "\nchecking solution:" << endl;

    uncover(first_guess);
    resolve_unsolved();

    // cout << "\nFinal solution graph:" << endl;
    // for (int i = 0; i < solution_graph.size(); ++i) {
    //     cout << disp(i) << ": ";
    //     for (auto x : solution_graph[i]) {
    //         cout << disp(x) << " ";
    //     }
    //     cout << endl;
    // }
    return status_board;
}

//private
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

    shared.contributors = square1.clues[clue_num1].contributors;
    append_if_unique(shared.contributors, square2.clues[clue_num2].contributors);
    if (solo2_updated) {
        solo2.contributors = shared.contributors;
        append_if_unique(solo2.contributors, square1.index);
    }
    append_if_unique(shared.contributors, square2.index);
    if (solo1_updated) {
        solo1.contributors = shared.contributors;
    }

    // We can add the shared clue to either square
    bool shared_added = insert_if_unique(square1.clues, shared);
    if (shared_added) {
        // cout << disp_c(shared) << " ";
    }

    bool solo1_added = false;
    if (solo1_updated){
        solo1_added = insert_if_unique(square1.clues, solo1);
    }
    if (solo1_added) {
        // cout << disp_c(solo1) << " ";
    }

    bool solo2_added = false;
    if (solo2_updated) {
        solo2_added = insert_if_unique(square2.clues, solo2);
    }
    if (solo2_added) {
        // cout << disp_c(solo2) << " ";
    }

    bool return_result = (shared_added || solo1_added || solo2_added);
    if (return_result) {
        // cout << endl;
    }

    return shared_added || solo1_added || solo2_added;
}

bool msolver::compare_squares(int index1, int index2) {
    for (int i = 0; i < unsolved[index1].clues.size(); ++i) {
        for (int j = 0; j < unsolved[index2].clues.size(); ++j) {
            clue old = unsolved[index1].clues[i];
            clue old2 = unsolved[index2].clues[j];

            bool new_clues_added = compare_clues(unsolved[index1], i, unsolved[index2], j);
            if (new_clues_added) {
                // cout << " Successful comparison:  " << disp(index1) << " " << disp_c(old) << " and " << disp(index2) << " " << disp_c(old2) << endl;

                fresh.push(index1);
                fresh.push(index2);
                status_board[index1] = Status::FRESH;
                status_board[index2] = Status::FRESH;
                return true;
            }
        }
    }
    #ifdef INCLUDE_ERRORS
        if  (unsolved.find(index1) == unsolved.end() || unsolved.find(index2) == unsolved.end())
            throw runtime_error("Compare squares called with squares not found in unsolved");
    #endif

    return false;
}

void msolver::maybe_remove_mine_from_unknown_border(int index) {
    if (unknown_border.find(index) == unknown_border.end()) {
        return;
    }
    for (int n = n_gen2.start(index); n != n_gen2.DONE; n = n_gen2.next()) {
        if (status_board[n] == Status::UNKNOWN) {
            return;
        }
    }
    unknown_border.erase(index);
}

void msolver::maybe_remove_unknown_from_unknown_border(int index) {
    auto ng2 = neighbor_generator(height, width);

    for (int m = ng2.start(index); m != ng2.DONE; m = ng2.next()) {
        if (status_board[m] != Status::UNKNOWN && board[index] != MINE) {
            return;
        }
    }
    unknown_border.erase(index);
}

void msolver::uncover(int index) {
    #ifdef INCLUDE_ERRORS
        if (board[index] == MINE) {
            std::string di = disp(index);
            throw runtime_error("Tried to uncover a mine");
        }
    #endif
    
    int square_val = board[index];
    unknown_border.erase(index);
    
    vector<int> clue_squares; clue_squares.reserve(6);
    for (int n = n_gen.start(index); n != n_gen.DONE; n = n_gen.next()) {
        if (status_board[n] == Status::UNKNOWN) {
            clue_squares.push_back(n);
            unknown_border.insert(n);
        }
        else if (status_board[n] == Status::UNSOLVED) {
            status_board[n] = Status::FRESH;
            fresh.push(n);
        }
        else if (board[n] == MINE) {
            --square_val; // Adjust the clue to only account for the unknown squares
            maybe_remove_mine_from_unknown_border(n);
        } 
    }

    if (clue_squares.empty()) {
        // if (status_board[index] == Status::FRESH) {
        //     cout << "uncover called on a solved fresh square" << endl;
        // }
        status_board[index] = Status::SOLVED;
        return;
    }

    if (status_board[index] != Status::FRESH) {
        fresh.push(index);
        status_board[index] = Status::FRESH;
    }
    unsolved.insert({index,
        unsolved_square{
            index,
            vector<clue>{clue{
                square_val,
                square_val,
                clue_squares
            }},
        }
    });
}

void msolver::mark_mine(int index) {
    #ifdef INCLUDE_ERRORS
        if  (status_board[index] != Status::UNKNOWN)
            throw runtime_error("Tried to mark a known square as a mine");
        if (board[index] != MINE)
            throw runtime_error("Falsly identified square as mine");
    #endif

    // cout << "Mine: ";
    // coords_from_index(index).print();
    // cout << endl;

    status_board[index] = Status::SOLVED;
    bool bordering_unknown = false;
    for (int n = n_gen.start(index); n != n_gen.DONE; n = n_gen.next()) {
        if (status_board[n] == Status::UNKNOWN) {
            bordering_unknown = true;
        }
        else if (status_board[n] == Status::UNSOLVED) {
            status_board[n] = Status::FRESH;
            fresh.push(n);
        }
        else if (board[n] == MINE) {
            maybe_remove_mine_from_unknown_border(n);
        }
    }

    if (bordering_unknown) {
        unknown_border.insert(index);
    } else {
        unknown_border.erase(index);
    }
}

void msolver::resolve_clues(unsolved_square& square) {
    /*
        Each iteration does one of 3 things:
        - increments i (the clue in question was not resolved)
        - removes the clue & resets i to 0 (clue is resolved; new squares are uncovered)
        - removes the clue & does not adjust i (clue is resolved; no new squares are uncovered)
    */
    bool removed_first = false;
    int i = 0;
    while (i < square.clues.size()) {
        vector<int> * clue_squares = &square.clues[i].squares;

        // Filter out revealed squares
        int j = 0;
        while (j < clue_squares->size()) {
            int sq = (*clue_squares)[j];
            if (status_board[sq] == Status::UNKNOWN) {
                ++j;
                continue;
            }
            clue_squares->erase(clue_squares->begin() + j);
            if (board[sq] == MINE) {
                square.clues[i].min -= 1;
                square.clues[i].max -= 1;
            }
            append_if_unique(solution_graph[sq], square.index);
            // append_if_unique(solution_graph[square.index].solved_by, sq);
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
            if (i == 0) removed_first = true;
        }

        // 2) there are no mines
        else if (max == 0) {
            for (int sq : *clue_squares) {
                uncover(sq);
                // append_if_unique(solution_graph[sq].solved_by, square.index);
                append_if_unique(solution_graph[square.index], sq);
                for (int contributor : square.clues[i].contributors) {
                    // append_if_unique(solution_graph[sq].solved_by, contributor);
                    append_if_unique(solution_graph[contributor], sq);
                }
            }
            if (i == 0) removed_first = true;
            square.clues.erase(square.clues.begin() + i);
            i = 0;
        }

        // 3) all squares are mines
        else if (num_squares == min) {
            for (int sq : *clue_squares) {
                mark_mine(sq);
                // append_if_unique(solution_graph[sq].solved_by, square.index);
                append_if_unique(solution_graph[square.index], sq);
                for (int contributor : square.clues[i].contributors) {
                    // append_if_unique(solution_graph[sq].solved_by, contributor);
                    append_if_unique(solution_graph[contributor], sq);
                }
            }
            if (i == 0) removed_first = true;
            square.clues.erase(square.clues.begin() + i);
            i = 0;
        }

        // Clue could not be resolved
        else {
            ++i;
        }
    }
    if (removed_first && square.clues.size() != 0) {
        cout << "removed first clue but still has remaining clues" << endl;
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
            // This happens only after unsolving squares
            // Just ignoring it doesn't seem to cause any issues though
            // if (status_board[square] == Status::SOLVED) {
            //     cout << "solved square (" << disp(square) << ") found in fresh" << endl;
            // } else if (status_board[square] == Status::UNKNOWN) {
            //     cout << "unknown square (" << disp(square) << ") found in fresh" << endl;
            // }
            continue;
        }

        #ifdef INCLUDE_ERRORS
            if (unsolved.find(square) == unsolved.end()) {
                coords_from_index(square).print();
                cout << endl << static_cast<int>(status_board[square]) << endl;
                cout << board[square] << endl;
                throw runtime_error("Fresh square not found in unsolved");
            }
        #endif

        resolve_clues(unsolved[square]);

        status_board[square] = Status::UNSOLVED;
        if (unsolved[square].clues.empty()) {
            if (square == current_square) {
                iterate_current_square();
            }
            if (unknown_border.find(square) != unknown_border.end()) {
                cout << "resolved square found in unknown border" << endl;
            }
            unsolved.erase(square);
            status_board[square] = Status::SOLVED;
            remove_clues_with_contributor(square);
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

        for (int n = sc_gen.start(current_square); n != sc_gen.DONE; n = sc_gen.next()) {
            if (status_board[n] != Status::UNSOLVED && status_board[n] != Status::FRESH) {
                continue;
            }
            bool new_clues = compare_squares(current_square, n);
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

void msolver::remove_clues_with_contributor(int index) {
    auto rcg = reverse_compare_generator(height, width);

    stack<int> squares;
    squares.push(index);
    while (!squares.empty()) {
        int sq = squares.top();
        squares.pop();

        for (int n = rcg.start(sq); n != rcg.DONE; n = rcg.next()) {
            if (status_board[n] == Status::UNKNOWN || status_board[n] == Status::SOLVED) {
                continue;
            }

            int i = 1; // Skip the first clue, it's never one with contributors
            bool had_clue_with_contributor = false;
            while (i < unsolved[n].clues.size()) {
                if (contains(unsolved[n].clues[i].contributors, index)) {
                    unsolved[n].clues.erase(unsolved[n].clues.begin() + i);
                    had_clue_with_contributor = true;
                } else {
                    ++i;
                }
            }

            if (had_clue_with_contributor) {
                // cout << disp(n) << " had a clue with contributor " << disp(index) << ". Removed it." << endl;
                squares.push(n);
            }
        }
    }
}

void msolver::search_up_sol_graph(int index) {
    // for (int i = 0; i < size; ++i) {
    //     for (int j = 0; j < solution_graph[i].size(); ++j) {
    //         if (solution_graph[i][j] == index) {
    //             solution_graph[i].erase(solution_graph[i].begin() + j);
    //         }
    //     }
    // }
    // return;
    auto rcg = reverse_compare_generator(height, width);

    stack<int> squares;
    squares.push(index);
    while (!squares.empty()) {
        int sq = squares.top();
        squares.pop();

        for (int n = rcg.start(sq); n != rcg.DONE; n = rcg.next()) {
            if (status_board[n] == Status::UNKNOWN) {
                continue;
            }

            for (int i = 0; i < solution_graph[n].size(); ++i) {
                if (solution_graph[n][i] == index) {
                    squares.push(n);
                    solution_graph[n].erase(solution_graph[n].begin() + i);
                    break;
                }
            }
        }
    }
}

void msolver::unsolve_square(int index) {
    auto ng = neighbor_generator(height, width);
    auto ng2 = neighbor_generator(height, width);

    set<int> neighbors_needing_reset;
    stack<int> squares = stack<int>();
    squares.push(index);
    while (!squares.empty()) {
        int sq = squares.top();
        squares.pop();
        cout << disp(sq) << " ";

        status_board[sq] = Status::UNKNOWN;
        unsolved.erase(sq);
        search_up_sol_graph(sq);

        for (int n = ng.start(sq); n != ng.DONE; n = ng.next()) {
            if (status_board[n] == Status::UNKNOWN) {
                maybe_remove_unknown_from_unknown_border(n);
                continue;
            }
            if (board[n] == MINE) {
                unknown_border.insert(n);
            } else {
                unknown_border.insert(sq);
                unsolved.erase(n);
                status_board[n] = Status::UNKNOWN;
                neighbors_needing_reset.insert(n);
                for (int m = ng2.start(n); m != ng2.DONE; m = ng2.next()) {
                    if (status_board[m] == Status::UNKNOWN) {
                        maybe_remove_unknown_from_unknown_border(m);
                    }
                }
            }
        }

        remove_clues_with_contributor(sq);
        for (int solved : solution_graph[sq]) {
            if (status_board[solved] != Status::UNKNOWN) {
                squares.push(solved);
            }
        }
        solution_graph[sq].clear();
    }

    for (int n : neighbors_needing_reset) {
        // unsolved.erase(n);
        // status_board[n] = Status::UNKNOWN;
        uncover(n);
    }
}

void msolver::remove_mine(int index) {
    auto ng = neighbor_generator(height, width);
    cout << "Removing mine: " << disp(index) << endl;

    board[index] = 0;
    for (int n = ng.start(index); n != ng.DONE; n = ng.next()) {
        if (board[n] == MINE) {
            ++board[index];
        } else {
            --board[n];
        }
        if (status_board[n] != Status::UNKNOWN) {
            unsolve_square(n);
        }
    }
}

void msolver::add_mine(int index) {
    auto ng = neighbor_generator(height, width);
    cout << "Adding mine: " << disp(index) << endl;

    board[index] = MINE;
    for (int n = ng.start(index); n != ng.DONE; n = ng.next()) {
        if (board[n] != MINE) {
            ++board[n];
        }
        if (status_board[n] != Status::UNKNOWN) {
            unsolve_square(n);
        }
    }
}

void msolver::change_one_square() {
    unknown_border.clear();
    for (int i = 0; i < size; ++i) {
        // if (board[i] == MINE && status_board[i] != Status::UNKNOWN) {
        //     for (int n = n_gen.start(i); n != n_gen.DONE; n = n_gen.next()) {
        //         if (status_board[n] == Status::UNKNOWN) {
        //             unknown_border.insert(i);
        //             break;
        //         }
        //     }
        // }
        if (status_board[i] == Status::UNKNOWN) {
            for (int n = n_gen.start(i); n != n_gen.DONE; n = n_gen.next()) {
                if (status_board[n] != Status::UNKNOWN) {
                    unknown_border.insert(i);
                    break;
                }
            }
        }
    }
    if (unknown_border.empty()){
        return;
    }

    // Get random square on border
    auto sq_to_change_itr = unknown_border.begin();
    advance(sq_to_change_itr, random_number(unknown_border.size() - 1));
    int sq_to_change = *sq_to_change_itr;

    // for (auto sq : unsolved) {
    //     unsolved[sq.first].clues = vector<clue>{sq.second.clues[0]};
    // }
    cout << "initial unsolving: ";
    unsolve_square(sq_to_change);
    cout << endl;
    if (board[sq_to_change] == MINE) {
        remove_mine(sq_to_change);
    }
    else {
        add_mine(sq_to_change);
    }
    cout << endl;

    current_square = unsolved.begin()->first;
}

// neighbor generator
msolver::neighbor_generator::neighbor_generator(int height, int width) :
    height(height),
    width(width),
    DONE(height * width)
{}

inline int msolver::neighbor_generator::start(int index) {
    row = int(index / width) + 1;
    col = int(index % width) + 1;
    i = 0;

    return next();
}

inline int msolver::neighbor_generator::next() {
    while (i < 8) {
        ((i & 2) ? row : col) += (i < 4) ? -1 : 1;
        ++i;

        if (row < 0 || row >= height || col < 0 || col >= width)
            continue;
        
        return row * width + col;
    }

    return DONE;
}

// clue compare generator
msolver::square_compare_generator::square_compare_generator(int height, int width) :
    height(height),
    width(width),
    DONE(height * width)
{}

inline int msolver::square_compare_generator::start(int index) {
    row = int(index / width);
    col = int(index % width);
    i = 0;

    return next();
}

inline int msolver::square_compare_generator::next() {
    while (i < 8) {
        ++i;
        int r = row + (i / 3);
        int c = col + (i % 3);

        if (r < 0 || r >= height || c < 0 || c >= width)
            continue;
        
        return r * width + c;
    }

    return DONE;
}

// reverse clue compare generator
msolver::reverse_compare_generator::reverse_compare_generator(int height, int width) :
    height(height),
    width(width),
    DONE(height * width)
{}

inline int msolver::reverse_compare_generator::start(int index) {
    row = int(index / width);
    col = int(index % width);
    i = -2; j = -2;
    return next();
}

inline int msolver::reverse_compare_generator::next() {
    while (i <= 2) {
        while (j <= 2) {
            int r = row + i;
            int c = col + j;
            ++j;

            if ((i == 2 || i == -2) && (j == 2 || j == -2))
                continue;
            if (i == 0 && j - 1 == 0)
                continue;
            if (r < 0 || r >= height || c < 0 || c >= width)
                continue;
            
            return r * width + c;
        }
        ++i;
        j = -2;
    }
    return DONE;
}
