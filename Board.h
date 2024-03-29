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
    uint64_t errorCatcher{0};
    int piecesLeft;

    unordered_map<uint64_t, tuple<uint32_t, int64_t, int>> transpositionTable; // map that for each position stores bestMove, evaluation, depthAtEvaluation

    uint64_t whitePawn{0};
    uint64_t whiteKnight{0};
    uint64_t whiteBishop{0};
    uint64_t whiteRook{0};
    uint64_t whiteQueen{0};
    uint64_t whiteKing{0};
    uint64_t blackPawn{0};
    uint64_t blackKnight{0};
    uint64_t blackBishop{0};
    uint64_t blackRook{0};
    uint64_t blackQueen{0};
    uint64_t blackKing{0};

    uint64_t currentMove;


    uint64_t pieceSquareHash[12][64];
    uint64_t blackMovingHash; // obvious
    uint64_t castlingHashes[4]; // ws wl bs bl
    uint64_t enPassantHashes[8];

    uint64_t currentHash;

    // TODO add add moving Piece to struct and adjust hash in undoLastMove
    struct move {
        pair<int8_t,int8_t> from;
        pair<int8_t,int8_t> to;
        int8_t beatenPiece;
        bool castle;
        bool enPassant;
        bool promotion;
        bool preWhiteShortCastle;
        bool preBlackShortCastle;
        bool preWhiteLongCastle;
        bool preBlackLongCastle;
        uint64_t lastHash;
    };
    stack<move> lastMoves;
    // it kinda makes sense to use a stack


    uint64_t debugInfo = 0;


    bool whiteShortCastle;
    bool blackShortCastle;

    bool whiteLongCastle;
    bool blackLongCastle;



    Board();
    vector<pair<int8_t,int8_t>> legalMoves(int8_t x, int8_t y);
    void fillBishopMoves(uint64_t& result, int8_t x, int8_t y, bool color);
    void fillRookMoves(uint64_t& result, int8_t x, int8_t y, bool color);
    int isGuarded(int8_t x, int8_t y, bool color);
    bool doSteps(int8_t x, int8_t y, function<pair<int8_t,int8_t>(int8_t, int8_t)> nextStep, function<bool(int8_t)> evaluate, bool color);
    pair<int8_t, int8_t> findNext(int8_t x, int8_t y, function<pair<int8_t,int8_t>(int8_t, int8_t)> nextStep, function<bool(int8_t)> evaluate, bool color);
    bool isMated(bool color);
    bool parseMove(int8_t x1, int8_t y1, int8_t x2, int8_t y2);
    void executeMove(int8_t x1, int8_t y1, int8_t x2, int8_t y2);
    uint64_t& getBitmaskOfPiece(int8_t piece);
    bool& canCastleShort(bool color);
    bool& canCastleLong(bool color);
    vector<uint32_t> getAllLegalMoves(bool color);
    bool inCheck(bool color);
    vector<pair<int8_t,int8_t>> getAttackingPieces(int8_t x, int8_t y, bool color, bool block);
    pair<int8_t, int8_t> pinnedPiece(bool color, int8_t x, int8_t y);
    void undoLastMove();
    uint64_t getHash(int8_t piece, int8_t x, int8_t y);

    /**
     * This Function evaluates the current position
     * @return returns a positive number if white is winning, a negative if black and 0 when there is a draw
     */
    int evaluatePosition();

    pair<stack<uint32_t>, int> bestMove(int depth, bool color, int alpha, int beta);
    pair<stack<uint32_t>, int> captureSearch(bool color, int alpha, int beta);
    uint64_t rateMove(uint32_t move);

};


#endif //CHESSENGINE_BOARD_H
