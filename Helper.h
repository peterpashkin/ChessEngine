//
// Created by Peter Pashkin on 25.08.23.
//

#include <cstdio>
#include <bits/stdc++.h>
#ifndef CHESSENGINE_HELPER_H
#define CHESSENGINE_HELPER_H
#define color(x) (x > 0)
#define shift(x,s) (s<0 ? x<<=-s:x>>=s)

using namespace std;


class Helper {
public:
    static int8_t** loadFile();
    static pair<int8_t, int8_t> getCoordinates(uint64_t bitmask);
    static uint64_t coordinatesToBitmask(int8_t x, int8_t y);
    static void removeBitFromCoordinates(uint64_t& bitmask, int8_t x, int8_t y);
    static void moveBitFromTo(uint64_t& bitmask, int8_t x1, int8_t y1, int8_t x2, int8_t y2);
    static void printBoard(int8_t** board);
    static uint64_t getPotentialKnightPattern(int8_t x, int8_t y);
    static uint64_t getPotentialPawnPattern(int8_t x, int8_t y);
    static uint64_t getPotentialKingPattern(int8_t x, int8_t y);
    static uint64_t getPotentialBishopPattern(int8_t x, int8_t y, int8_t iteration);
    static uint64_t getPotentialRookPattern(int8_t x, int8_t y, int8_t iteration);
    static uint64_t getPotentialQueenPattern(int8_t x, int8_t y, int8_t iteration);
};


#endif //CHESSENGINE_HELPER_H
