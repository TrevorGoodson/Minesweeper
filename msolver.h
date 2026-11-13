#ifdef INCLUDE_ERRORS
#include <stdexcept>
#endif

#include <iostream>
#include <stack>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "helperFunctions.h"

class msolver {
public:
    enum class Status : unsigned char {
        UNKNOWN = 0b00,  // a square that has not been determined as a mine or not yet
        SOLVED = 0b10,   // either a known mine or a known square with no surrounding unknowns
        UNSOLVED = 0b01, // a square that has been determined to not be a mine, but is bordering an unknown square
        FRESH = 0b11,    // an unsolved square whose clues have not yet been evaluated
    };

    msolver(std::vector<int> board, int height, int width);
    std::vector<Status> solve(int start_row, int start_col);
    std::vector<int> board;
private:
    enum board_key { MINE = 9 };
    struct clue {
        int min;
        int max;
        std::vector<int> squares;
        std::vector<int> contributors;

        void print() {
            std::cout << min << "-" << max << ": ";
            for (int s : squares) {
                coord{
                    int(s / 14),
                    int(s % 14)
                }.print();
                std::cout << ", ";
            }
        }

        bool operator==(const clue& other) const {
            // Ignores contributors
            if (this->min != other.min
                || this->max != other.max
                || this->squares.size() != other.squares.size())
                return false;
        
            for (int i = 0; i < this->squares.size(); ++i) {
                if (this->squares[i] != other.squares[i])
                    return false;
            }
            return true;
        }
    };
    struct unsolved_square {
        int index;
        // The first clue is always the one that you get from the number on the board
        // Any additional clues are ones derived from comparing to neighboring squares
        std::vector<clue> clues;

        void print() {
            std::cout << "clues at " << index << " | ";
            for (clue c : clues) {
                c.print();
                std::cout << "| ";
            }
        }
    };
    struct coord {
        int row;
        int col;

        void print() {
            std::cout << "(" << char('a' + row) << ", " << char('A' + col) << ")";
        }
    };
    struct node {
        std::vector<int> solved_by;
        std::vector<int> solved;
    };
    class neighbor_generator {
        public:
            neighbor_generator(int height, int width);
            inline int start(int index);
            inline int next();
            const int DONE;
        private:
            int height, width, row, col, i;
    };
    class square_compare_generator {
        public:
            square_compare_generator(int height, int width);
            inline int start(int index);
            inline int next();
            const int DONE;
        private:
            int height, width, row, col, i;
    };
    class reverse_compare_generator {
        public:
            reverse_compare_generator(int height, int width);
            inline int start(int index);
            inline int next();
            const int DONE;
        private:
            int height, width, row, col, i, j;
    };

    int height, width, size, current_square, first_guess;
    const int OUT_OF_RANGE;
    std::vector<Status> status_board;
    std::vector<int> solution_order;
    std::vector<node> solution_graph;
    std::set<int> unknown_border;
    std::map<int, unsolved_square> unsolved; // All UNSOLVED squares (including FRESH)
    std::stack<int> fresh; // All FRESH squares
    neighbor_generator n_gen, n_gen2; // Two to accomodate for nested use
    square_compare_generator sc_gen;

    bool compare_clues(unsolved_square& square1, int clue_num1, unsolved_square& square2, int clue_num2);
    bool compare_squares(int index1, int index2);
    void uncover(int index);
    void mark_mine(int index);
    void resolve_clues(unsolved_square& square);
    void iterate_current_square();
    void resolve_fresh_squares();
    void resolve_unsolved();
    void remove_clues_with_contributor(int index);
    void unsolve_square(int index);
    void remove_mine(int index);
    void add_mine(int index);
    void change_one_square();

    std::string disp(int index) {
        std::string result = "  ";
        result[0] = char(index / width) + 'a';
        result[1] = char(index % width) + 'A';
        return result;
    }
    
    inline int index_from_coords(int row, int col) {
        return row * width + col;
    }

    inline coord coords_from_index(int index) {
        return coord{
            int(index / width),
            int(index % width)
        };
    }

    inline static bool equal(const clue& a, const clue& b) {
        if (a.min != b.min || a.max != b.max || a.squares.size() != b.squares.size())
            return false;
        
        for (int i = 0; i < a.squares.size(); ++ i) {
            if (a.squares[i] != b.squares[i])
                return false;
        }
        return true; 
    }

    static bool insert_if_unique(std::vector<clue>& clues, clue new_clue) {
        for (const clue& c : clues) {
            if (equal(c, new_clue)) {
                return false;
            }
        }
        clues.push_back(new_clue);
        return true;
    }

    template <typename T> static bool append_if_unique(std::vector<T>& v, T x) {
        for (T y : v) {
            if (y == x) {
                return false;
            }
        }
        v.push_back(x);
        return true;
    }

    template <typename T> static bool append_if_unique(std::vector<T>& v1, std::vector<T>& v2) {
        bool result = false;
        for (T x : v2) {
            bool appended = append_if_unique(v1, x);
            result = result || appended;
        }
        return result;
    }

    template <typename T> static bool contains(std::vector<T> v, T x) {
        for (T y : v) {
            if (y == x) {
                return true;
            }
        }
        return false;
    }

    template <typename T> static bool remove(std::vector<T> v, T x) {
        for (int i = 0; i < v.size(); ++i) {
            if (v[i] == x) {
                v.erase(v.begin() + i);
                return true;
            }
        }
        return false;
    }
};