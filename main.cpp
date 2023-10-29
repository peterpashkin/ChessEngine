#include <iostream>
#include "Board.h"
#define printPair(x) int(x.first) << " " << int(x.second)
#define endl "\n"

int main() {
    //cout << filesystem::current_path().string() << endl;
    auto b = new Board();



    bool currentColor = true;
    int counter = 0;

    /*auto x = b->getAllLegalMoves(false);
    for(auto performingMove: x) {
        int8_t x1 = performingMove >> 24;
        int8_t y1 = (performingMove & 0xff0000) >> 16;
        int8_t x2 = (performingMove & 0xff00) >> 8;
        int8_t y2 = performingMove & 0xff;
        cout << char('a'+y1) << " " <<  int(8-x1) << " " << char('a'+y2) << " " << int(8-x2) << endl;
    }*/




    while(true) {
        // let's assume we are only given good moves

        // i hate this, just do random piece
        auto moves = b->getAllLegalMoves(currentColor);


        if(moves.empty() || b->piecesLeft <= 2) {
            break;
        }

        /* random moves no more
        random_device dev;
        mt19937 rng(dev());
        uniform_int_distribution<std::mt19937::result_type> dist6(0,moves.size()-1);


        int index = dist6(rng);

        uint32_t performingMove = moves[index];*/
        if(true) {
            auto bestMove = b->bestMove(7, currentColor, INT_MIN, INT_MAX);
            uint32_t performingMove = bestMove.first.top();
            int8_t x1 = performingMove >> 24;
            int8_t y1 = (performingMove & 0xff0000) >> 16;
            int8_t x2 = (performingMove & 0xff00) >> 8;
            int8_t y2 = performingMove & 0xff;


            b->executeMove(x1, y1, x2, y2);
        } else {
            int x1,y1,x2,y2; cin >> x1 >> y1 >> x2 >> y2;
            b->executeMove(x1,y1,x2,y2);
        }

        Helper::printBoard(b->board);
        cout << endl;
        currentColor = !currentColor;
    }

    Helper::printBoard(b->board);

    cout << "The game lasted " << b->currentMove << " moves!" << endl;
    cout << b->piecesLeft << endl;
    if(b->piecesLeft <= 2 || (!b->inCheck(currentColor))) {
        cout << "The game ends in a draw" << endl;
    } else if(!currentColor) {
        cout << "White wins!";
    } else {
        cout << "Black wins!";
    }


    return 0;
}
