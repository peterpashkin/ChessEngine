//
// Created by Peter Pashkin on 25.08.23.
//
#include "Helper.h"
#include <cstdio>
#include <bits/stdc++.h>

#ifndef CHESSENGINE_BOARD_H
#define CHESSENGINE_BOARD_H

using namespace std;


class Board {
    public:
    /**
     * This array represents the board, the figures are placed this way:
     * 1 for pawn, 3 for knight, 4 for bishop, 6 for rook, 9 for queen and 63 for king. White gets positive values
     * while black gets negative values.
     * The weights are chosen as the chess weights to make calculations easier
     */
    int8_t** board;
    uint64_t errorCatcher;
    int piecesLeft;

    uint64_t whitePawn;
    uint64_t whiteKnight;
    uint64_t whiteBishop;
    uint64_t whiteRook;
    uint64_t whiteQueen;
    uint64_t whiteKing;
    uint64_t blackPawn;
    uint64_t blackKnight;
    uint64_t blackBishop;
    uint64_t blackRook;
    uint64_t blackQueen;
    uint64_t blackKing;

    uint64_t currentMove;
    uint64_t lastPawnMoveTime;
    pair<int8_t, int8_t> lastPawnMove; // used for en passant detection

    bool whiteCastles;
    bool blackCastles; // used for en Passant detection (was either piece moved)

    Board();
    vector<pair<int8_t,int8_t>> legalMoves(int8_t x, int8_t y);
    void fillBishopMoves(vector<pair<int8_t,int8_t>>& result, int8_t x, int8_t y, bool color);
    void fillRookMoves(vector<pair<int8_t,int8_t>>& result, int8_t x, int8_t y, bool color);
    int isGuarded(int8_t x, int8_t y, bool color);
    bool doSteps(int8_t x, int8_t y, function<pair<int8_t,int8_t>(int8_t, int8_t)> nextStep, function<bool(int8_t)> evaluate, bool color);
    pair<int8_t, int8_t> findNext(int8_t x, int8_t y, function<pair<int8_t,int8_t>(int8_t, int8_t)> nextStep, function<bool(int8_t)> evaluate, bool color);
    bool isMated(bool color);
    bool parseMove(int8_t x1, int8_t y1, int8_t x2, int8_t y2);
    void executeMove(int8_t x1, int8_t y1, int8_t x2, int8_t y2);
    uint64_t& getBitmaskOfPiece(int8_t piece);
    bool& getEnPassant(bool color);
    vector<uint32_t> getAllLegalMoves(bool color);
    bool inCheck(bool color);
    vector<pair<int8_t,int8_t>> getAttackingPieces(int8_t x, int8_t y, bool color, bool block);

    /**
     * This Function evaluates the current position
     * @return returns a positive number if white is winning, a negative if black and 0 when there is a tie
     */
    int evaluatePosition();
};


#endif //CHESSENGINE_BOARD_H
