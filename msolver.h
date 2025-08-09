//     class mineSolver {
//         public:
//         mineSolver(vector<int>& startingCoords) : direction({0,1}), coordinates(startingCoords), nextCoordinates(2), onTheRight(2) {
//             findNextSteps();
//         }
//         void printCoords() const {
//             cout << "(" << intToLowerCase(coordinates[0]) << "," << intToUpperCase(coordinates[1]) << ") ";
//         }
//         void printRightStep() const {
//             cout << "(" << intToLowerCase(onTheRight[0]) << "," << intToUpperCase(onTheRight[1]) << ") size: " << onTheRight.size();
//         }
//         void printDirection() const {
//             cout << "(" << direction[0] << "," << direction[1] << ") ";
//         }
//         void step() {
//             coordinates = nextCoordinates;
//             findNextSteps();
//         }
//         void turn(bool clockwise) {
//             int tempDirection1 = direction[1];
//             direction[1] = direction[0] * (clockwise * -1);
//             direction[0] = tempDirection1 * (!clockwise * -1);
//             findNextSteps();
//         }
//         vector<int> nextStep() const {
//             return nextCoordinates;
//         }
//         vector<int> whatsOnTheRight() const {
//             return onTheRight;
//         }
//         vector<int> location() const {
//             return direction;
//         }
//         void slideToTheRight() {
//             coordinates = onTheRight;
//             findNextSteps();
//         }
//         private:
//         vector<int> direction;
//         vector<int> onTheRight;
//         vector<int> coordinates;
//         vector<int> nextCoordinates;
//         void findNextSteps() {
//             vector<int> v = {direction[1], direction[0] * -1};
//             nextCoordinates = addVectors(coordinates, direction);
//             onTheRight = addVectors(coordinates, v);
//         }
//     };
//
//     struct mineBoard {
//         vector<vector<char>> board;
//         const int height;
//         const int length;
//         mineBoard(vector<vector<char>> inputBoard) : board(inputBoard), height(inputBoard.size()), length(inputBoard[0].size()) {}
//         char at(vector<int> coordinates) {
//             if (coordinates.size() != 2) {
//                 cerr << "Invalid Coordinates" << endl;
//                 exit(1);
//             }
//             if (coordinates[0] < 0 || coordinates[0] >= height || coordinates[1] < 0 || coordinates[1] >= length) {
//                 return OUT_OF_BOUNDS;
//             }
//             return board[coordinates[0]][coordinates[1]];
//         }
//         bool anyUnkownSquaresAround(vector<int> coordinates) {
//             if (coordinates[0] + 1 < height && board[coordinates[0] + 1][coordinates[1]] == UNKNOWN_SQUARE) {
//                 return true;
//             }
//             if (coordinates[1] + 1 < length && board[coordinates[0]][coordinates[1] + 1] == UNKNOWN_SQUARE) {
//                 return true;
//             }
//             if (coordinates[0] - 1 >= 0 && board[coordinates[0] - 1][coordinates[1]] == UNKNOWN_SQUARE) {
//                 return true;
//             }
//             if (coordinates[1] - 1 < length && board[coordinates[0]][coordinates[1] - 1] == UNKNOWN_SQUARE) {
//                 return true;
//             }
//             return false;
//         }
//     };
//
//     class unsolvedSquares {
//         public:
//         unsolvedSquares(vector<int> inputCoordinates) : coordinates(inputCoordinates), board(nullptr) {}
//         unsolvedSquares(vector<int> inputCoordinates, mineBoard* inputBoard) : coordinates(inputCoordinates), board(inputBoard) {}
//         bool operator < (unsolvedSquares& otherSquare) {
//             return coordinates < otherSquare.coordinates;
//         }
//         private:
//         vector<int> coordinates;
//         mineBoard* board;
//     };
//
// void board::testPossibility(vector<int> startingCoordinates) {
//     mineSolver EOD(startingCoordinates); //EOD stands for Explosive Ordnance Disposal
//
//     mineBoard testBoard(playerUI);
//
//     EOD.printRightStep();
//     cout << endl;
//     EOD.printCoords();
//     while (testBoard.at(EOD.whatsOnTheRight()) != UNKNOWN_SQUARE && testBoard.at(EOD.whatsOnTheRight()) != OUT_OF_BOUNDS) {
//         EOD.slideToTheRight();
//         EOD.printCoords();
//     }
//     cout << endl;
//
//     PrintBoard(testBoard.board);
// }