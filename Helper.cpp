//
// Created by Peter Pashkin on 25.08.23.
//

#include "Helper.h"
using namespace std;

uint64_t leftEliminate = 0x3f3f3f3f3f3f3f3f, rightEliminate = 0xfcfcfcfcfcfcfcfc;

/**
 * We will look at the file and read the Board configuration
 * We have the following pieces written into the file
 * p - pawn, n - knight, b - bishop, r - rook, q - queen, k - king
 * White pieces will be in upper case, while black ones are in lower case
 * @return
 */
int8_t **Helper::loadFile() {
    ifstream myfile ("../chess_start.txt", ios::in);

    auto** result = (int8_t**) malloc(64);

    for(int i=0; i<8; i++) {
        result[i] = static_cast<int8_t *>(malloc(8));
    }


    for(int i=0; i<8; i++) {
        string s;
        getline(myfile, s);
        for(int u=0; u<8; u++) {
            char c = s[u];
            int8_t writing = 0;
            switch(c) {
                case 'P':
                    writing = 1;
                    break;
                case 'N':
                    writing = 3;
                    break;
                case 'B':
                    writing = 4;
                    break;
                case 'R':
                    writing = 6;
                    break;
                case 'Q':
                    writing = 9;
                    break;
                case 'K':
                    writing = 63;
                    break;
                case 'p':
                    writing = -1;
                    break;
                case 'n':
                    writing = -3;
                    break;
                case 'b':
                    writing = -4;
                    break;
                case 'r':
                    writing = -6;
                    break;
                case 'q':
                    writing = -9;
                    break;
                case 'k':
                    writing = -63;
                    break;
                default:
                    writing = 0;
                    break;
            }
            result[i][u] = writing;
        }
    }

    myfile.close();

    return result;

}

pair<int8_t, int8_t> Helper::getCoordinates(uint64_t bitmask) {
    int tmp = __builtin_clzll(bitmask);
    return make_pair(tmp/8, tmp%8);
}

uint64_t Helper::coordinatesToBitmask(int8_t x, int8_t y) {
    uint64_t mask = 1;
    mask <<= (7-x) * 8;
    mask <<= (7-y);
    return mask;
}

void Helper::removeBitFromCoordinates(uint64_t& bitmask, int8_t x, int8_t y) {
    uint64_t removeMask = coordinatesToBitmask(x, y);
    bitmask &= (~removeMask);
}

void Helper::moveBitFromTo(uint64_t &bitmask, int8_t x1, int8_t y1, int8_t x2, int8_t y2) {
    uint64_t removeMask = coordinatesToBitmask(x1, y1);
    uint64_t addindMask = coordinatesToBitmask(x2, y2);
    bitmask &= (~removeMask);
    bitmask |= addindMask;
}

void Helper::printBoard(int8_t **board) {

    cout << "  0 1 2 3 4 5 6 7" << endl;

    for(int i=0; i<8; i++) {
        cout << i << " ";
        for(int u=0; u<8; u++) {
            auto current = board[i][u];
            bool colorVal = color(current);
            char toPrint = '.';
            auto value = abs(current);

            switch(value) {
                case 1:
                    toPrint = 'p';
                    break;
                case 3:
                    toPrint = 'n';
                    break;
                case 4:
                    toPrint = 'b';
                    break;
                case 6:
                    toPrint = 'r';
                    break;
                case 9:
                    toPrint = 'q';
                    break;
                case 63:
                    toPrint = 'k';
                    break;
                default:
                    toPrint = '.';
                    break;
            }


            if(colorVal) {
                toPrint -= 32;
            }

            cout << toPrint << " ";
        }
        cout << endl;
    }


}


uint64_t Helper::getPotentialKnightPattern(int8_t x, int8_t y) {
    uint64_t knightPattern = 0b0101000010001000000000001000100001010000000000000000000000000000;
    int yShift = y-2;
    shift(knightPattern, yShift);
    int xShift = x - 2;
    shift(knightPattern, xShift * 8);

    if(y < 2) {
        knightPattern &= rightEliminate;
    } else if(y > 5) {
        knightPattern &= leftEliminate;
    }

    return knightPattern;
}

// the color computation will just be done in the program
uint64_t Helper::getPotentialPawnPattern(int8_t x, int8_t y) {
    uint64_t pawnPattern = 0x50'0000'0000;
    int yShift = y-2;
    shift(pawnPattern, yShift);
    int xShift = x - 2;
    shift(pawnPattern, xShift * 8);

    if(y < 2) {
        pawnPattern &= rightEliminate;
    } else if(y > 5) {
        pawnPattern &= leftEliminate;
    }

    return pawnPattern;
}

uint64_t Helper::getPotentialKingPattern(int8_t x, int8_t y) {
    uint64_t kingPattern = 0b0000000001110000010100000111000000000000000000000000000000000000;
    int yShift = y-2;
    int xShift = x-2;
    shift(kingPattern, yShift);
    shift(kingPattern, xShift*8);

    if(y < 2) {
        kingPattern &= rightEliminate;
    } else if(y > 5) {
        kingPattern &= leftEliminate;
    }

    return kingPattern;

}

uint64_t Helper::getPotentialBishopPattern(int8_t x, int8_t y, int8_t iteration) {
    // this method cannot work like the previous ones since no single bitpattern
    // can go over the whole board
    uint64_t bishopPattern = 0;
    // we will make the pattern by knowing there are at most 4 possible squares
    // we check if they can exist and add them

    if(x + iteration < 8 && y + iteration < 8) bishopPattern |= coordinatesToBitmask(x+iteration, y+iteration);
    if(x - iteration >= 0 && y + iteration < 8) bishopPattern |= coordinatesToBitmask(x-iteration, y+iteration);
    if(x + iteration < 8 && y - iteration >= 0) bishopPattern |= coordinatesToBitmask(x+iteration, y-iteration);
    if(x - iteration >= 0 && y - iteration >= 0) bishopPattern |= coordinatesToBitmask(x-iteration, y-iteration);

    return bishopPattern;
}

uint64_t Helper::getPotentialRookPattern(int8_t x, int8_t y, int8_t iteration) {
    uint64_t rookPattern = 0;

    if(x + iteration < 8) rookPattern |= coordinatesToBitmask(x + iteration, y);
    if(x - iteration >= 0) rookPattern |= coordinatesToBitmask(x - iteration, y);
    if(y + iteration < 8) rookPattern |= coordinatesToBitmask(x, y + iteration);
    if(y - iteration >= 0) rookPattern |= coordinatesToBitmask(x, y - iteration);

    return rookPattern;
}

uint64_t Helper::getPotentialQueenPattern(int8_t x, int8_t y, int8_t iteration) {
    uint64_t queenPattern = getPotentialBishopPattern(x,y,iteration) | getPotentialRookPattern(x,y,iteration);
    return queenPattern;
}

vector<pair<int8_t, int8_t>> Helper::convertBitMaskToPoints(uint64_t bitmask) {
    vector<pair<int8_t, int8_t>> result;
    while(bitmask) {
        auto tmp = getCoordinates(bitmask);
        bitmask &= ~(coordinatesToBitmask(tmp.first, tmp.second));
        result.push_back(tmp);
    }

    return result;
}

/**
 * This method will return a bitmask of all squares between the attacker and the king
 * @param attackerX
 * @param attackerY
 * @param kingX
 * @param kingY
 * @return bitmask of possible moves for a piece pinned by the attacker
 */
uint64_t Helper::possiblePinnedMoves(int8_t attackerX, int8_t attackerY, int8_t kingX, int8_t kingY) {
    // we will start from the attacker and do normalized steps till we get to the king

    if(attackerX == -1) return -1;


    int8_t normalizedX = normalize(kingX - attackerX);
    int8_t normalizedY = normalize(kingY - attackerY);
    uint64_t result = 0;
    while(attackerX != kingX && attackerY != kingY) {
        result |= coordinatesToBitmask(attackerX, attackerY);
        attackerX += normalizedX;
        attackerY += normalizedY;
    }

    return result;
}




