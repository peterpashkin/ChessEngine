#include <iostream>
#include "Board.h"
#define printPair(x) int(x.first) << " " << int(x.second)
#define endl "\n"

int main() {
    //cout << filesystem::current_path().string() << endl;
    auto b = new Board();




    /*auto x = b->getAllLegalMoves(false);
    for(auto performingMove: x) {
        int8_t x1 = performingMove >> 24;
        int8_t y1 = (performingMove & 0xff0000) >> 16;
        int8_t x2 = (performingMove & 0xff00) >> 8;
        int8_t y2 = performingMove & 0xff;
        cout << int(x1) << " " << int(y1) << "  " << int(x2) << " " << int(y2) << endl;
    }
    return 1;*/



    Helper::printBoard(b->board);
    cout << endl;




    bool currentColor = true;
    int counter = 0;



    while(counter < 6) {
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
        if(currentColor) {
            auto pair = b->bestMove(5, currentColor);
            uint32_t performingMove = pair.first;
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
        currentColor = !currentColor;
        //this_thread::sleep_for(chrono::seconds(2));
        cout << endl;
        //counter++;
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
