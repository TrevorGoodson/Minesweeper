#include "board.h"
using namespace std;

int main() {
    #if INCLUDE_ERRORS
        try {
    #endif

    board player1;
    while (!player1.GetHealth() && !player1.DidWin()) {
        player1.PrintUI();
        player1.GetGuess();
    }
    player1.ShowXs();
    player1.PrintUI();

    #if INCLUDE_ERRORS
        } catch (const std::exception& e) {
            cout << "Caught an error: " << e.what() << '\n';
        }
    #endif

    return 0;
}