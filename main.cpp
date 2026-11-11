#include "board.h"
using namespace std;

int main() {
    try {
        board player1;
        while (!player1.GetHealth() && !player1.DidWin()) {
            player1.PrintUI();
            player1.GetGuess();
        }
        player1.ShowXs();
        player1.PrintUI();
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Caught an error: " << e.what() << '\n';
    }
    return 0;
}