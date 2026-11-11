#ifdef INCLUDE_ERRORS
#include <stdexcept>
#endif

#include <iostream>
#include <stack>
#include <vector>
#include <map>
using namespace std;

class msolver {
public:
    msolver(vector<int> board, int height, int width);
    vector<int> solve(int start_row, int start_col);
private:
    enum board_key { MINE = 9 };
    enum status_values {
        UNKNOWN = 0,
        UNSOLVED = 1,
        FRESH = 2,
        SOLVED = 3
    };
    
    struct clue {
        int min;
        int max;
        vector<int> squares;

        void print() {
            cout << min << "-" << max << ": ";
            for (int s : squares) {
                cout << s << ", ";
            }
        }
    };
    struct unsolved_square {
        int index;
        vector<clue> clues;

        void print() {
            cout << "clues at " << index << " | ";
            for (clue c : clues) {
                c.print();
                cout << "| ";
            }
        }
    };
    struct coord {
        int row;
        int col;
    };

    int height;
    int width;
    int size;
    vector<int> board;
    vector<int> status_board;
    map<int, unsolved_square> unsolved;
    stack<int> fresh;
    vector<int> neighbor_buffer;
    int last_square_compared;

    inline int index_from_coords(int row, int col) {
        return row * width + col;
    }

    inline coord coords_from_index(int index) {
        return coord{
            int(index / width),
            int(index % width)
        };
    }

    inline vector<int> copy_neighbor_buffer(int num_neighbors) {
        return vector<int>(neighbor_buffer.begin(), neighbor_buffer.begin() + num_neighbors);
    }

    int get_neighbors(int index, int status);
    int get_squares_with_overlapping_clues(int index);
    bool compare_clues(unsolved_square& square1, int clue_num1, unsolved_square& square2, int clue_num2);
    bool compare_squares(int index1, int index2);
    void freshen_neighbors(int index);
    void uncover(int index);
    void mark_mine(int index);
    void resolve_clues(unsolved_square& square);
    void resolve_fresh_squares();
    void resolve_unsolved();
};