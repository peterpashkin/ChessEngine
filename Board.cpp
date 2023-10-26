//
// Created by Peter Pashkin on 25.08.23.
//

/* TODO
 * this list has every bug that is currently known
 * - pinned pieces are not handled
 * - (should be fixed) somehow knight can't block
 * - (should be fixed) PAWN can block mates by moving since we just check if someone attacks an empty square
 * - (fixed) staleMate
 * - (fixed) the fucking king is in the way so the game thinks the square is not guarded (just do attacking patterns as bit) rewrite is Guarded
 * + temporary fix: doSteps and findNext skip the king of it's color
 * - it's more efficient for the inCheck case to just return all defensive pieces
 */

#define printPair(x) int(x.first) << " " << int(x.second)

#include "Board.h"
#define abs(x) (x < 0 ? -x:x)
#define shift(x,s) (s<0 ? x<<=-s:x>>=s)
#define step(first, second) [](int8_t x, int8_t y) -> pair<int8_t, int8_t> {return make_pair(x+first, y+second);}
#define movingPiece(value) [&color](int8_t piece) -> bool {return color(piece) != color && (abs(piece) == value || abs(piece) == 9);}
#define normalize(x) (x == 0 ? 0: (x < 0 ? -1:1))
#define sameSign(x,y) ((x<0 && y<0) || (x>0 && y>0) || (!x && !y)) // so both negative/positive/zero
#define endl "\n"

uint64_t whitePromoted = 0xff00'0000'0000'0000;
uint64_t blackPromoted = 0xff;


Board::Board() {
    board = Helper::loadFile();
    // we initialize the bitboards

    // we will fill the board like this
    // 1 2 4 8 ...
    // 256 512 ...
    // .
    // .
    errorCatcher = 0;
    currentMove = 1;
    blackCastles = true;
    whiteCastles = true;
    lastPawnMoveTime = -1;
    piecesLeft = 0;

    // en passant helper
    move placeholder = {{0,0}, {0,0}, 0, false, false, false, false, false};
    lastMoves.push(placeholder);

    long long int currentBit = 1;
    for (int i = 7; i >= 0; i--) {
        for (int u = 7; u >= 0; u--) {
            int8_t current = board[i][u];
            getBitmaskOfPiece(current) |= currentBit;
            currentBit <<= 1;
            if(current) piecesLeft++;
        }
    }
}





vector<pair<int8_t, int8_t>> Board::legalMoves(int8_t x, int8_t y) {
    int8_t piece = board[x][y];

    bool color = (piece > 0);
    piece = abs(piece);
    vector<pair<int8_t,int8_t>> result;

    // TODO and with possible moves if pinned
    uint64_t possibleMoves = 0;

    // we will look for a check
    // depending on this we might give the opportunity to capture an attacking piece
    // or just allow kingMoves (for everything >1 check)
    // TODO blocking pieces, probably needs some rewrite of code, check every square from king to attacking Piece


    // pinned pieces
    auto kingCoordinates = Helper::getCoordinates(color ? whiteKing:blackKing);
    auto pinningPiece = pinnedPiece(color, x, y);
    uint64_t pinnedMoves = Helper::possiblePinnedMoves(pinningPiece.first, pinningPiece.second, kingCoordinates.first, kingCoordinates.second);

    //if(inCheck(color) && piece!=63) {
    if(false) {

        // so this does the following
        // 1. get king coordinates of my color
        // 2. get everyAttacker on the king
        // 3. if there is more than one everyAttacker, we have to do a king move, there is no other way, we return
        // 4. if not we know there is exactly one everyAttacker
        // 5. we will do the following for every square from everyAttacker to king
        //  check which pieces could go onto this square, if they match with the piece we are at right now, we add it
        //  TODO honestly since we check for a specific piece, there is probably something more clever



        auto everyAttacker = getAttackingPieces(kingCoordinates.first, kingCoordinates.second, color, false);


        if(everyAttacker.size() > 1) {
            return result;
        }



        auto attacker = everyAttacker[0];
        // TODO maybe save this to avoid computation for other pieces
        // TODO yeah pinned pieces can't defend

        int8_t attackingPieceX = attacker.first;
        int8_t attackingPieceY = attacker.second;


        int8_t directionX = kingCoordinates.first-attackingPieceX; // so positive is everyAttacker up
        int8_t directionY = kingCoordinates.second-attackingPieceY; // so positive is everyAttacker left

        int8_t normalizedStepX = normalize(directionX);
        int8_t normalizedStepY = normalize(directionY);

        if(abs(board[attackingPieceX][attackingPieceY]) == 3) {
            // knight implementation
            // the attack can't be blocked, only taking the knight is an option
            auto potentialDefenders = getAttackingPieces(attackingPieceX, attackingPieceY, !color, false);
            for (auto check: potentialDefenders) {
                if (check.first == x && check.second == y) {
                    possibleMoves |= Helper::coordinatesToBitmask(attackingPieceX, attackingPieceY);
                }
            }
        } else {
            // first iteration will be done manually do distinguish between pawns blocking and taking
            auto potentialDefenders = getAttackingPieces(attackingPieceX, attackingPieceY, !color, false);
            for (auto check: potentialDefenders) {
                if (check.first == x && check.second == y) {
                    possibleMoves |= Helper::coordinatesToBitmask(attackingPieceX, attackingPieceY);
                }
            }

            attackingPieceX += normalizedStepX;
            attackingPieceY += normalizedStepY;

            while ((attackingPieceX != kingCoordinates.first) || (attackingPieceY != kingCoordinates.second)) {
                potentialDefenders = getAttackingPieces(attackingPieceX, attackingPieceY, !color, true);
                for (auto check: potentialDefenders) {
                    if (check.first == x && check.second == y) {
                        possibleMoves |= Helper::coordinatesToBitmask(attackingPieceX, attackingPieceY);
                    }
                }

                attackingPieceX += normalizedStepX;
                attackingPieceY += normalizedStepY;
            }
        }


        return Helper::convertBitMaskToPoints(possibleMoves & pinnedMoves);
    }


    switch(piece) {
        case 1: {
            // this is the pawn testcase
            move lastMove = lastMoves.top();
            bool wasPawnMove = abs(board[lastMove.to.first][lastMove.to.second]) == 1;
            if (color) {
                if (x == 6 && !board[x - 1][y] && !board[x - 2][y]) {
                    possibleMoves |= Helper::coordinatesToBitmask(x-2,y);
                    //result.emplace_back(x - 2, y);
                }
                if (!board[x - 1][y]) {
                    possibleMoves |= Helper::coordinatesToBitmask(x-1,y);
                    //result.emplace_back(x - 1, y);
                }
                if (y > 0 && (board[x - 1][y - 1] < 0 || (wasPawnMove && lastMove.to.first == x && lastMove.to.second == (y-1)))) {
                    possibleMoves |= Helper::coordinatesToBitmask(x-1, y-1);
                    //result.emplace_back(x - 1, y - 1);
                }
                if (y < 7 && (board[x - 1][y + 1] < 0 || (wasPawnMove && lastMove.to.first == x && lastMove.to.second == (y+1)))) {
                    possibleMoves |= Helper::coordinatesToBitmask(x-1, y+1);
                    //result.emplace_back(x - 1, y + 1);
                }
            } else {
                if (x == 1 && !board[x + 1][y] && !board[x + 2][y]) {
                    possibleMoves |= Helper::coordinatesToBitmask(x+1, y);
                    //result.emplace_back(x + 2, y);
                }
                if (!board[x + 1][y]) {
                    possibleMoves |= Helper::coordinatesToBitmask(x+1, y);
                    //result.emplace_back(x + 1, y);
                }
                if (y > 0 && (board[x + 1][y - 1] > 0 || (wasPawnMove && lastMove.to.first == x && lastMove.to.second == (y-1)))) {
                    possibleMoves |= Helper::coordinatesToBitmask(x+1, y-1);
                    //result.emplace_back(x + 1, y - 1);
                }
                if (y < 7 && (board[x + 1][y + 1] > 0 || (wasPawnMove && lastMove.to.first == x && lastMove.to.second == (y+1)))) {
                    possibleMoves |= Helper::coordinatesToBitmask(x+1, y+1);
                    //result.emplace_back(x + 1, y + 1);
                }
            }
            break;
        }
        case 3: {
            // this is the knight test case
            vector<pair<int8_t, int8_t>> jumping;
            jumping.emplace_back(x - 2, y + 1);
            jumping.emplace_back(x - 1, y + 2);
            jumping.emplace_back(x + 1, y + 2);
            jumping.emplace_back(x + 2, y + 1);
            jumping.emplace_back(x + 2, y - 1);
            jumping.emplace_back(x + 1, y - 2);
            jumping.emplace_back(x - 1, y - 2);
            jumping.emplace_back(x - 2, y - 1);

            for (auto p: jumping) {
                int8_t x1 = p.first;
                int8_t y1 = p.second;

                if (x1 < 0 || x1 > 7 || y1 < 0 || y1 > 7) continue;
                int8_t current_piece = board[x1][y1];

                if (current_piece == 0 || color(current_piece) != color) possibleMoves |= Helper::coordinatesToBitmask(x1,y1);
            }
            break;
        }
        case 4: {
            // this is the bishop case
            fillBishopMoves(possibleMoves, x, y, color);
            break;
        }
        case 6: {
            // this is the rook case
            fillRookMoves(possibleMoves, x, y, color);
            break;
        }
        case 9:
            // those are the legal moves for the queen
            // we will be a bit cheesy here and call the methods for the rook and bishop
            fillBishopMoves(possibleMoves, x, y, color);
            fillRookMoves(possibleMoves, x, y, color);
            break;
        case 63: {
            // TODO dont castle when any of the squares are guarded

            // those are the legal moves for the king
            // we need to do one more thing for the king,
            // that is to check wether the square we move to is guarded by any of black pieces

            // let's review our options
            // 1. query every move the opponent could make in the next move, if any of them cover the square we want to go to, the move is illegal
            // 2. just do the move and lose, we just accept this, as our engine should avoid bad moves like this
            // 3. do a search from the square, so just go throgh the ranks and look if there is a piece on the diagonal or sth -> this seems most efficient

            vector<pair<int8_t, int8_t>> jumping;
            jumping.emplace_back(x + 1, y + 1);
            jumping.emplace_back(x + 1, y);
            jumping.emplace_back(x + 1, y - 1);
            jumping.emplace_back(x - 1, y + 1);
            jumping.emplace_back(x - 1, y - 1);
            jumping.emplace_back(x - 1, y);
            jumping.emplace_back(x, y + 1);
            jumping.emplace_back(x, y - 1);

            for (auto p: jumping) {
                int8_t x1 = p.first;
                int8_t y1 = p.second;

                if (x1 < 0 || x1 > 7 || y1 < 0 || y1 > 7) continue;
                int8_t current_piece = board[x1][y1];

                if ((color(current_piece) != color || !current_piece) && !isGuarded(x1,y1,color)) possibleMoves |= Helper::coordinatesToBitmask(x1,y1);
            }

            // we also check for en passant
            if(getEnPassant(color)) {
                // so king and rook haven't been moved
                // with this we can assume that the coordinates of the king are the ones from the start
                // the only other criteria is therefore space between the two figures
                if(!board[x][5] && !board[x][6]) {
                    //result.emplace_back(x, 6); // the rook logic will be implemented in the executeMove
                    possibleMoves |= Helper::coordinatesToBitmask(x, 6);
                }
            }

            break;
        }
        default:
            break;

    }
    return Helper::convertBitMaskToPoints(possibleMoves & pinnedMoves);
}

void Board::fillBishopMoves(uint64_t& possibleMoves, int8_t x, int8_t y, bool color) {
    int tmp1 = x + 1;
    int tmp2 = y + 1;
    for (int8_t i = max(x, y) + 1; i < 8; i++) {
        if (!board[tmp1][tmp2]) {
            possibleMoves |= Helper::coordinatesToBitmask(tmp1, tmp2);
        } else if (color(board[tmp1][tmp2]) != color) {
            possibleMoves |= Helper::coordinatesToBitmask(tmp1, tmp2);
            break;
        } else {
            break;
        }
        tmp1++;
        tmp2++;
    }

    tmp1 = x + 1;
    tmp2 = y - 1;

    for (int8_t i = max(x, (int8_t) (8 - y)) + 1; i < 8; i++) {
        if (!board[tmp1][tmp2]) {
            possibleMoves |= Helper::coordinatesToBitmask(tmp1, tmp2);
        } else if (color(board[tmp1][tmp2]) != color) {
            possibleMoves |= Helper::coordinatesToBitmask(tmp1, tmp2);
            break;
        } else {
            break;
        }
        tmp1++;
        tmp2--;
    }


    tmp1 = x - 1;
    tmp2 = y + 1;

    for (int8_t i = max((int8_t) (8 - x), y) + 1; i < 8; i++) {
        if (!board[tmp1][tmp2]) {
            possibleMoves |= Helper::coordinatesToBitmask(tmp1, tmp2);
        } else if (color(board[tmp1][tmp2]) != color) {
            possibleMoves |= Helper::coordinatesToBitmask(tmp1, tmp2);
            break;
        } else {
            break;
        }
        tmp1--;
        tmp2++;
    }

    tmp1 = x - 1;
    tmp2 = y - 1;

    for (int8_t i = max(8 - x, 8 - y) + 1; i < 8; i++) {
        if (!board[tmp1][tmp2]) {
            possibleMoves |= Helper::coordinatesToBitmask(tmp1, tmp2);
        } else if (color(board[tmp1][tmp2]) != color) {
            possibleMoves |= Helper::coordinatesToBitmask(tmp1, tmp2);
            break;
        } else {
            break;
        }
        tmp1--;
        tmp2--;
    }
}

void Board::fillRookMoves(uint64_t& possibleMoves, int8_t x, int8_t y, bool color) {
    int8_t tmp = x;
    while (tmp < 7 && !board[++tmp][y]) possibleMoves |= Helper::coordinatesToBitmask(tmp, y);
    if (color(board[tmp][y]) != color) possibleMoves |= Helper::coordinatesToBitmask(tmp, y);

    tmp = x;
    while (tmp > 0 && !board[--tmp][y]) possibleMoves |= Helper::coordinatesToBitmask(tmp, y);
    if (color(board[tmp][y]) != color) possibleMoves |= Helper::coordinatesToBitmask(tmp, y);

    tmp = y;
    while (tmp < 7 && !board[x][++tmp]) possibleMoves |= Helper::coordinatesToBitmask(x, tmp);
    if (color(board[x][tmp]) != color) possibleMoves |= Helper::coordinatesToBitmask(x, tmp);

    tmp = y;
    while (tmp > 0 && !board[x][--tmp]) possibleMoves |= Helper::coordinatesToBitmask(x, tmp);
    if (color(board[x][tmp]) != color) possibleMoves |= Helper::coordinatesToBitmask(x, tmp);
}



int Board::isGuarded(int8_t x, int8_t y, bool color) {
    // the return value will be the amount of pieces currently having the king in check
    // this way we can still use the value as a boolean (>0 is true) but it gives the opportunity
    // to check for double checks

    // we check every possible direction for the pieces
    // we will partially use the bitboards for that

    // let's start with knight checks since those can be done perfectly with bitboards
    // we could do the following: we define a constant pattern, the knight pattern and shift the middle
    // to the point we are looking at right now.
    // We get a problem with behaviour on left and right side
    // the bits are moven to a weird position

    // we will define a deletion pattern for the border cases
    uint64_t knightPattern = Helper::getPotentialKnightPattern(x, y);

    uint64_t pawnPattern;
    // they can be easily done with a bit pattern like the knights
    if(color) {
        pawnPattern = Helper::getPotentialPawnPattern(x-2, y);
    } else {
        pawnPattern = Helper::getPotentialPawnPattern(x, y);
    }

    // same thing for the king
    uint64_t kingPattern = Helper::getPotentialKingPattern(x, y);

    int result = 0;

    // now we can check for knights
    // it should be recognized here, that there is no way to have two checks from knights, that's why we
    // will only count one, regardless of the position
    if((color && (knightPattern & blackKnight)) || (!color && (knightPattern & whiteKnight))) result++;

    if((color && (pawnPattern & blackPawn)) || (!color && (pawnPattern & whitePawn))) result++;

    if((color && (kingPattern & blackKing)) || (!color && (kingPattern & whiteKing))) result++;


    // now we can check the diagonal and the rows for opposing pieces
    // we will do this with the doSteps method


    result += doSteps(x, y, step(-1,0),movingPiece(6), color);
    result += doSteps(x, y, step(-1,1),movingPiece(4), color);
    result += doSteps(x, y, step(0,1), movingPiece(6), color);
    result += doSteps(x, y, step(1,1), movingPiece(4), color);
    result += doSteps(x, y, step(1,0), movingPiece(6), color);
    result += doSteps(x, y, step(1,-1),movingPiece(4), color);
    result += doSteps(x, y, step(0,-1),movingPiece(6), color);
    result += doSteps(x, y, step(-1,-1),movingPiece(4), color);



    return result;
}

bool Board::doSteps(int8_t x, int8_t y, function<pair<int8_t, int8_t>(int8_t, int8_t)> nextStep,
                    function<bool(int8_t)> evaluate, bool color) {
    // we will do steps until we find a piece and we are in the boundary-> then return what evaluate would return

    pair<int8_t, int8_t> nextOnes = nextStep(x,y);
    x = nextOnes.first; y = nextOnes.second;

    while(x<8 && y<8 && x>=0 && y>=0) {
        if(board[x][y] && board[x][y] != (color ? 63:-63)) {
            return evaluate(board[x][y]);
        }
        nextOnes = nextStep(x,y);
        x = nextOnes.first; y = nextOnes.second;
    }

    return false;
}

bool Board::isMated(bool color) {
    // TODO should check for beating the rest (not really necessary i guess)
    /* this is legacy code, might use later
    pair<int8_t, int8_t> kingCoordinates = color ? Helper::getCoordinates(whiteKing) : Helper::getCoordinates(blackKing);
    //cout << int(kingCoordinates.first) << " " << int(kingCoordinates.second) << endl;
    return legalMoves(kingCoordinates.first, kingCoordinates.second).empty() && isGuarded(kingCoordinates.first, kingCoordinates.second, color);
     */

    // wont use i think
    return getAllLegalMoves(color).empty();
}

bool Board::parseMove(int8_t x1, int8_t y1, int8_t x2, int8_t y2) {
    // honestly not sure why i have this method, i guess for safety reasons currently
    // but i should assume the bot only chooses legal Moves
    auto moves = legalMoves(x1, y1);

    for(auto p: moves) {
        if(p.first == x2 && p.second == y2) {
            executeMove(x1, y1, x2, y2);
            return true;
        }
    }

    return false;
}

void Board::executeMove(int8_t x1, int8_t y1, int8_t x2, int8_t y2) {
    // this method will implement time and move based logic like en passant and castles

    // should be fairly simple with it being the only two cell king move and
    // the only sideways pawn motion without a piece on the moving square


    int8_t piece = board[x1][y1];
    int8_t beatenPiece = board[x2][y2];
    board[x2][y2] = board[x1][y1];
    board[x1][y1] = 0;


    move lastMove = {{x1,y1}, {x2,y2}, beatenPiece, false, false, false, whiteCastles, blackCastles};


    // this is the en passant implementation
    if(abs(piece) == 1 && beatenPiece == 0 && abs(y2-y1) == 1) {
        Helper::removeBitFromCoordinates(getBitmaskOfPiece(board[x1][y2]), x1, y2);
        piecesLeft--;
        board[x1][y2] = 0;
        lastMove.enPassant = true;
    }

    // this is the castles implementation
    if(abs(piece) == 63 && abs(y2-y1)>1) {
        // the king moves should be done by the following code
        // this indent is here to move the rook
        int8_t rookPiece = board[x1][7];
        Helper::moveBitFromTo(getBitmaskOfPiece(rookPiece), x1, 7, x1, 5); // changes bitmask
        board[x1][5] = rookPiece; // changes board
        board[x1][7] = 0;
        lastMove.castle = true;
    }



    Helper::moveBitFromTo(getBitmaskOfPiece(piece), x1, y1, x2, y2);
    if(beatenPiece) {
        Helper::removeBitFromCoordinates(getBitmaskOfPiece(beatenPiece), x2, y2);
        piecesLeft--;
    }


    // castles
    // we will make use of the fact that this method is called every move
    // so the rook based castle doesn't work <=> the square of the rook wasn't occupied by the right colored rook
    // same applies for the king

    // TODO this could also be implemented with the bitboards, probably more efficient
    // TODO also those ifs could be combined by checking who moved i guess, but look out because white could break blacks privilege
    if((board[7][7] != 6) || (board[7][4] != 63)) {
        whiteCastles = false;
    }
    if((board[0][7] != -6) || (board[0][4] != -63)) {
        blackCastles = false;
    }

    uint64_t whitePawns = whitePawn & whitePromoted;
    uint64_t blackPawns = blackPawn & blackPromoted;
    // Promotion
    if(whitePawns) {
        pair<int8_t, int8_t> coordinates = Helper::getCoordinates(whitePawns);
        board[coordinates.first][coordinates.second] = 9;

        whitePawn &= (~whitePawns);
        whiteQueen |= whitePawns;
        lastMove.promotion = true;
    }

    if(blackPawns) {
        pair<int8_t, int8_t> coordinates = Helper::getCoordinates(blackPawns);
        board[coordinates.first][coordinates.second] = -9;

        blackPawn &= (~blackPawns);
        blackQueen |= blackPawns;
        lastMove.promotion = true;
    }


    currentMove++;
    lastMoves.push(lastMove);
}

uint64_t &Board::getBitmaskOfPiece(int8_t piece) {
    bool color = color(piece);
    piece = abs(piece);
    switch(piece) {
        case 1: {
            return color ? whitePawn : blackPawn;
        }
        case 3: {
            return color ? whiteKnight : blackKnight;
        }
        case 4: {
            return color ? whiteBishop : blackBishop;
        }
        case 6: {
            return color ? whiteRook : blackRook;
        }
        case 9: {
            return color ? whiteQueen : blackQueen;
        }
        case 63: {
            return color ? whiteKing : blackKing;
        }
        default:
            // TODO some error message
            return errorCatcher;
    }
}

bool &Board::getEnPassant(bool color) {
    return color ? whiteCastles : blackCastles;
}

vector<uint32_t> Board::getAllLegalMoves(bool color) {

    vector<uint32_t> result;

    if(inCheck(color)) {
        // so this does the following
        // 1. get king coordinates of my color
        // 2. get everyAttacker on the king
        // 3. if there is more than one everyAttacker, we have to do a king move, there is no other way, we return
        // 4. if not we know there is exactly one everyAttacker
        // 5. we will do the following for every square from everyAttacker to king
        //  check which pieces could go onto this square, if they match with the piece we are at right now, we add it



        auto kingCoordinates = Helper::getCoordinates(color ? whiteKing:blackKing);
        auto everyAttacker = getAttackingPieces(kingCoordinates.first, kingCoordinates.second, color, false);


        auto x = legalMoves(kingCoordinates.first, kingCoordinates.second);

        if(everyAttacker.size() > 1) {
            for(auto j: x) result.push_back(Helper::convertMoveToBitmask(kingCoordinates.first, kingCoordinates.second, j.first, j.second));
            return result;
        }

        for(auto kingMoves: x) {
            result.push_back(Helper::convertMoveToBitmask(kingCoordinates.first, kingCoordinates.second, kingMoves.first, kingMoves.second));
        }


        auto attacker = everyAttacker[0];

        int8_t attackingPieceX = attacker.first;
        int8_t attackingPieceY = attacker.second;


        int8_t directionX = kingCoordinates.first-attackingPieceX; // so positive is everyAttacker up
        int8_t directionY = kingCoordinates.second-attackingPieceY; // so positive is everyAttacker left

        int8_t normalizedStepX = normalize(directionX);
        int8_t normalizedStepY = normalize(directionY);

        if(abs(board[attackingPieceX][attackingPieceY]) == 3) {
            // knight implementation
            // the attack can't be blocked, only taking the knight is an option
            auto potentialDefenders = getAttackingPieces(attackingPieceX, attackingPieceY, !color, false);
            for (auto check: potentialDefenders) {
                auto pinningPiece = pinnedPiece(color, check.first, check.second);
                uint64_t pinnedMoves = Helper::possiblePinnedMoves(pinningPiece.first, pinningPiece.second, kingCoordinates.first, kingCoordinates.second);
                if(pinnedMoves & Helper::coordinatesToBitmask(attackingPieceX, attackingPieceY)) result.push_back(Helper::convertMoveToBitmask(check.first, check.second, attackingPieceX, attackingPieceY));
            }
        } else {
            // first iteration will be done manually do distinguish between pawns blocking and taking
            auto potentialDefenders = getAttackingPieces(attackingPieceX, attackingPieceY, !color, false);
            for (auto check: potentialDefenders) {
                auto pinningPiece = pinnedPiece(color, check.first, check.second);
                uint64_t pinnedMoves = Helper::possiblePinnedMoves(pinningPiece.first, pinningPiece.second, kingCoordinates.first, kingCoordinates.second);
                if(pinnedMoves & Helper::coordinatesToBitmask(attackingPieceX, attackingPieceY)) result.push_back(Helper::convertMoveToBitmask(check.first, check.second, attackingPieceX, attackingPieceY));
            }

            attackingPieceX += normalizedStepX;
            attackingPieceY += normalizedStepY;

            while ((attackingPieceX != kingCoordinates.first) || (attackingPieceY != kingCoordinates.second)) {
                potentialDefenders = getAttackingPieces(attackingPieceX, attackingPieceY, !color, true);
                for (auto check: potentialDefenders) {
                    auto pinningPiece = pinnedPiece(color, check.first, check.second);
                    uint64_t pinnedMoves = Helper::possiblePinnedMoves(pinningPiece.first, pinningPiece.second, kingCoordinates.first, kingCoordinates.second);
                    if(pinnedMoves & Helper::coordinatesToBitmask(attackingPieceX, attackingPieceY)) result.push_back(Helper::convertMoveToBitmask(check.first, check.second, attackingPieceX, attackingPieceY));
                }

                attackingPieceX += normalizedStepX;
                attackingPieceY += normalizedStepY;
            }
        }


        return result;
    }



    for(int i=0; i<8; i++) {
        for(int u=0; u<8; u++) {
            if(board[i][u] && color(board[i][u]) == color) {
                auto tmp = legalMoves(i, u);
                for(auto p: tmp) {
                    uint32_t toAdd = (i<<24) | (u << 16) | (p.first << 8) | p.second;
                    result.push_back(toAdd);
                }
            }
        }
    }
    return result;
}

bool Board::inCheck(bool color) {
    pair<int8_t, int8_t> coordinates = Helper::getCoordinates(getBitmaskOfPiece(63 * (color ? 1:-1)));
    return isGuarded(coordinates.first, coordinates.second, color);
}

vector<pair<int8_t, int8_t>> Board::getAttackingPieces(int8_t x, int8_t y, bool color, bool block) {
    uint64_t attackingKnights = Helper::getPotentialKnightPattern(x,y) & (color ? blackKnight : whiteKnight);
    int numberOfKnights = __builtin_popcountll(attackingKnights);

    vector<pair<int8_t, int8_t>> result;

    // get every knight location
    for (int i = 0; i < numberOfKnights; i++) {
        auto tmp = Helper::getCoordinates(attackingKnights);
        auto deletion = Helper::coordinatesToBitmask(tmp.first, tmp.second);
        result.push_back(tmp);
        attackingKnights &= (~deletion);
    }

    uint64_t attackingPawns;
    if(!block) {
        int8_t xBias = (color ? x - 2 : x);
        attackingPawns = Helper::getPotentialPawnPattern(xBias, y) & (color ? blackPawn : whitePawn);
    } else {
        uint64_t pawnPattern = color ? Helper::coordinatesToBitmask(x-1,y) : Helper::coordinatesToBitmask(x+1,y);
        if(color && x == 4) pawnPattern |= Helper::coordinatesToBitmask(x-2,y);
        else if(!color && x == 3) pawnPattern |= Helper::coordinatesToBitmask(x+2,y);
        attackingPawns = pawnPattern & (color ? blackPawn : whitePawn);
    }

    int numberOfPawns = __builtin_popcountll(attackingPawns);
    // get every pawn location
    for (int i = 0; i < numberOfPawns; i++) {
        auto tmp = Helper::getCoordinates(attackingPawns);
        auto deletion = Helper::coordinatesToBitmask(tmp.first, tmp.second);
        result.push_back(tmp);
        attackingPawns &= (~deletion);
    }

    // same for bishop/rook/queen
    /*
    // iterate through all possible iterations (farthest piece is at most 7 squares away)
    // TODO doesn't really work: now every piece that possibly has something in it's way is also added
    for(int8_t i=1; i<8; i++) {
        uint64_t attackingBishops = Helper::getPotentialBishopPattern(x,y,i) & (color ? blackBishop : whiteBishop);
        uint64_t attackingRooks = Helper::getPotentialBishopPattern(x,y,i) & (color ? blackRook : whiteRook);
        uint64_t attackingQueens = Helper::getPotentialBishopPattern(x,y,i) & (color ? blackQueen : whiteQueen);

        int numberOfBishops = __builtin_popcountll(attackingBishops);
        for(int u=0; u<numberOfBishops; u++) {
            auto tmp = Helper::getCoordinates(attackingBishops);
            auto deletion = Helper::coordinatesToBitmask(tmp.first, tmp.second);
            result.push_back(tmp);
            attackingBishops &= (~deletion);
        }

        int numberOfRooks = __builtin_popcountll(attackingRooks);
        for(int u=0; u<numberOfRooks; u++) {
            auto tmp = Helper::getCoordinates(attackingRooks);
            auto deletion = Helper::coordinatesToBitmask(tmp.first, tmp.second);
            result.push_back(tmp);
            attackingRooks &= (~deletion);
        }

        int numberOfQueens = __builtin_popcountll(attackingQueens);
        for(int i=0; i<numberOfQueens; i++) {
            auto tmp = Helper::getCoordinates(attackingQueens);
            auto deletion = Helper::coordinatesToBitmask(tmp.first, tmp.second);
            result.push_back(tmp);
            attackingQueens &= (~deletion);
        }

    }*/


    // we know use the findNext method to determine the knights for every other location
    auto pairInDirection = findNext(x, y, step(-1, 0), movingPiece(6), color);
    if (pairInDirection.first != -1) result.push_back(pairInDirection);
    pairInDirection = findNext(x, y, step(-1, 1), movingPiece(4), color);
    if (pairInDirection.first != -1) result.push_back(pairInDirection);
    pairInDirection = findNext(x, y, step(0, 1), movingPiece(6), color);
    if (pairInDirection.first != -1) result.push_back(pairInDirection);
    pairInDirection = findNext(x, y, step(1, 1), movingPiece(4), color);
    if (pairInDirection.first != -1) result.push_back(pairInDirection);
    pairInDirection = findNext(x, y, step(1, 0), movingPiece(6), color);
    if (pairInDirection.first != -1) result.push_back(pairInDirection);
    pairInDirection = findNext(x, y, step(1, -1), movingPiece(4), color);
    if (pairInDirection.first != -1) result.push_back(pairInDirection);
    pairInDirection = findNext(x, y, step(0, -1), movingPiece(6), color);
    if (pairInDirection.first != -1) result.push_back(pairInDirection);
    pairInDirection = findNext(x, y, step(-1, -1), movingPiece(4), color);
    if (pairInDirection.first != -1) result.push_back(pairInDirection);


    return result;
}

pair<int8_t, int8_t> Board::findNext(int8_t x, int8_t y, function<pair<int8_t, int8_t>(int8_t, int8_t)> nextStep,
                                     function<bool(int8_t)> evaluate, bool color) {
    pair<int8_t, int8_t> nextOnes = nextStep(x,y);
    x = nextOnes.first; y = nextOnes.second;

    while(x<8 && y<8 && x>=0 && y>=0) {
        if(board[x][y] && board[x][y] != (color ? 63:-63)) {
            if(evaluate(board[x][y])) return make_pair(x,y);
            else return make_pair(-1, -1);
        }
        nextOnes = nextStep(x,y);
        x = nextOnes.first; y = nextOnes.second;
    }

    return make_pair(-1, -1);
}

/**
 * This function evaluates if the given piece is pinned and returns the coordinates of the attacker that pins the piece, allowing the piece to move relatively
 * @param color
 * @param x
 * @param y
 * @return coordinates of pinning piece or -1,-1 if the piece is not pinned
 */
pair<int8_t, int8_t> Board::pinnedPiece(bool color, int8_t x, int8_t y) {
    auto kingCoordinates = Helper::getCoordinates(color ? whiteKing : blackKing);

    // detect if there is any piece between the king and the piece and
    // if this piece is even in a place where it could be pinned

    auto xdif = x - kingCoordinates.first;
    auto ydif = y - kingCoordinates.second;

    if(abs(xdif) == abs(ydif) || !xdif || !ydif) {

        auto everyAttacker = getAttackingPieces(x, y, color, false);
        for(auto attacker: everyAttacker) {
            // check if it's relative position is same as to king
            auto attDifX = attacker.first - x;
            auto attDifY = attacker.second - y;
            auto typeAttacker = board[attacker.first][attacker.second];

            switch(abs(typeAttacker)) {
                case 4:
                case 6:
                case 9:
                    if(sameSign(attDifX, xdif) && sameSign(attDifY, ydif)) {
                        return attacker;
                    }
            }
        }
    }


    return pair<int8_t, int8_t>(-1,-1);
}



int Board::evaluatePosition() {
    // for starters this function will just decide upon the pieces
    int whiteWeight = __builtin_popcountll(whitePawn) + __builtin_popcountll(whiteKnight) * 3 + __builtin_popcountll(whiteBishop) * 4 + __builtin_popcountll(whiteRook) * 6 + __builtin_popcountll(whiteQueen) * 9;
    int blackWeight = __builtin_popcountll(blackPawn) + __builtin_popcountll(blackKnight) * 3 + __builtin_popcountll(blackBishop) * 4 + __builtin_popcountll(blackRook) * 6 + __builtin_popcountll(blackQueen) * 9;
    return whiteWeight - blackWeight;
}



pair<uint32_t, int> Board::bestMove(int depth, bool color) {
    if(depth == 0) {
        return make_pair(0, evaluatePosition());
    }


    pair<uint32_t, int> result = make_pair(0, color ? INT_MIN:INT_MAX);
    auto tryouts = getAllLegalMoves(color);

    // mate or draw
    if(tryouts.empty()) {
        if(inCheck(color)) return make_pair(0, color ? INT_MIN:INT_MAX); // mated
        else return make_pair(0, 0); // stalemate
    }

    // TODO contemplate to return instantly if there is only one move


    for(auto performingMove: tryouts) {
        int8_t x1 = performingMove >> 24;
        int8_t y1 = (performingMove & 0xff0000) >> 16;
        int8_t x2 = (performingMove & 0xff00) >> 8;
        int8_t y2 = performingMove & 0xff;


        executeMove(x1, y1, x2, y2);
        auto thisRes = bestMove(depth-1, !color);

        if(color && thisRes.second >= result.second) {
            result = make_pair(performingMove, thisRes.second);
        }

        if(!color && thisRes.second <= result.second) {
            result = make_pair(performingMove, thisRes.second);
        }
        undoLastMove();

    }


    return result;

}

Board::Board(const Board &rhs) {
    // Perform a deep copy of the board
    board = new int8_t*[8];
    for (int i = 0; i < 8; i++) {
        board[i] = new int8_t[8];
        for (int j = 0; j < 8; j++) {
            board[i][j] = rhs.board[i][j];
        }
    }

    // Copy other member variables
    errorCatcher = rhs.errorCatcher;
    piecesLeft = rhs.piecesLeft;
    whitePawn = rhs.whitePawn;
    whiteKnight = rhs.whiteKnight;
    whiteBishop = rhs.whiteBishop;
    whiteRook = rhs.whiteRook;
    whiteQueen = rhs.whiteQueen;
    whiteKing = rhs.whiteKing;
    blackPawn = rhs.blackPawn;
    blackKnight = rhs.blackKnight;
    blackBishop = rhs.blackBishop;
    blackRook = rhs.blackRook;
    blackQueen = rhs.blackQueen;
    blackKing = rhs.blackKing;
    currentMove = rhs.currentMove;
    lastPawnMoveTime = rhs.lastPawnMoveTime;
    lastPawnMove = rhs.lastPawnMove;
    whiteCastles = rhs.whiteCastles;
    blackCastles = rhs.blackCastles;
}

Board &Board::operator=(const Board &rhs) {
    if (this == &rhs) {
        return *this;
    }

    // Cleanup existing resources
    for (int i = 0; i < 8; i++) {
        delete[] board[i];
    }
    delete[] board;

    // Perform a deep copy similar to the copy constructor
    board = new int8_t*[8];
    for (int i = 0; i < 8; i++) {
        board[i] = new int8_t[8];
        for (int j = 0; j < 8; j++) {
            board[i][j] = rhs.board[i][j];
        }
    }

    // Copy other member variables
    errorCatcher = rhs.errorCatcher;
    piecesLeft = rhs.piecesLeft;
    whitePawn = rhs.whitePawn;
    whiteKnight = rhs.whiteKnight;
    whiteBishop = rhs.whiteBishop;
    whiteRook = rhs.whiteRook;
    whiteQueen = rhs.whiteQueen;
    whiteKing = rhs.whiteKing;
    blackPawn = rhs.blackPawn;
    blackKnight = rhs.blackKnight;
    blackBishop = rhs.blackBishop;
    blackRook = rhs.blackRook;
    blackQueen = rhs.blackQueen;
    blackKing = rhs.blackKing;
    currentMove = rhs.currentMove;
    lastPawnMoveTime = rhs.lastPawnMoveTime;
    lastPawnMove = rhs.lastPawnMove;
    whiteCastles = rhs.whiteCastles;
    blackCastles = rhs.blackCastles;

    return *this;
}

void Board::undoLastMove() {
    auto lastMove = lastMoves.top();
    lastMoves.pop();

    whiteCastles = lastMove.preWhiteCastle;
    blackCastles = lastMove.preBlackCastle;
    currentMove--;

    if(lastMove.castle) {
        // castle moves are represented by either 7 4 -> 7 6 (white)
        // or 0 4 -> 0 6 (black)
        // we are just going to adjust the rook, the king will be adjusted for us

        int currentX = lastMove.to.first;
        board[currentX][7] = board[currentX][5];
        board[currentX][5] = 0;
        Helper::moveBitFromTo(getBitmaskOfPiece(board[currentX][7]), currentX, 5, currentX, 7);

        (currentX ? whiteCastles:blackCastles) = true;

    } else if(lastMove.enPassant) {
        // if the last move was en Passant, we know we beat the pawn that now stands behind us
        // the pawn that beat it will be reset either way
        // we can exactly determine the beaten pawn was on: x is same as fromX and y is toY
        int8_t beatenPawn = -board[lastMove.to.first][lastMove.to.second];
        board[lastMove.from.first][lastMove.to.second] = beatenPawn;
        getBitmaskOfPiece(beatenPawn) |= Helper::coordinatesToBitmask(lastMove.from.first, lastMove.to.second);
        piecesLeft++;
    }

    if(lastMove.promotion) {
        // if we promoted, we cannot reset the pawn normally, so we need to do a complete distinct case analysis
        // we basically just put the normalized piece on the square it came from
        int8_t pastPawn = normalize(board[lastMove.to.first][lastMove.to.second]);
        board[lastMove.from.first][lastMove.from.second] = pastPawn;
        board[lastMove.to.first][lastMove.to.second] = lastMove.beatenPiece;
        getBitmaskOfPiece(pastPawn) |= Helper::coordinatesToBitmask(lastMove.from.first, lastMove.from.second);
        getBitmaskOfPiece(9*pastPawn) &= (~Helper::coordinatesToBitmask(lastMove.to.first, lastMove.to.second));

        if(lastMove.beatenPiece) piecesLeft++;
        getBitmaskOfPiece(lastMove.beatenPiece) |= Helper::coordinatesToBitmask(lastMove.to.first, lastMove.to.second);

        return;
    }

    if(lastMove.beatenPiece) piecesLeft++;

    int8_t movedPiece = board[lastMove.to.first][lastMove.to.second];
    board[lastMove.from.first][lastMove.from.second] = movedPiece;
    board[lastMove.to.first][lastMove.to.second] = lastMove.beatenPiece;

    Helper::moveBitFromTo(getBitmaskOfPiece(movedPiece), lastMove.to.first, lastMove.to.second, lastMove.from.first, lastMove.from.second);
    getBitmaskOfPiece(lastMove.beatenPiece) |= Helper::coordinatesToBitmask(lastMove.to.first, lastMove.to.second);

}

