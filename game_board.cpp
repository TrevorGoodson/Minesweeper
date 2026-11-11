#include <vector>
using namespace std;

class game_board {
public:
    game_board(int height, int width) :
        height(height),
        width(width),
        values(vector<int>(height * width))
    {}

    enum board_value {
        MINE = 9
    };

    int get(int row, int col) {
        return values[row * width + col];
    }

    void set(int row, int col, int value) {
        values[row * width + col] = value;
    }

    int get(int index) {
        return values[index];
    }
    
    int is_in_range(int index) {
        return index < values.size();
    }

    int get_width() {
        return width;
    }

    int get_height() {
        return height;
    }
private:
    vector<int> values;
    int width;
    int height;
};