#include "board.h"
using namespace std;

int main() {
    board player1;
    while (!player1.GetHealth() && !player1.DidWin()) {
        player1.PrintUI();
        player1.GetGuess();
    }
    player1.ShowXs();
    player1.PrintUI();
    return 0;
}