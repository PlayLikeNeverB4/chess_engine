
#ifndef _POSITION_H_
#define _POSITION_H_

#include "types.h"
#include "Piece.h"
#include "Move.h"

using namespace std;

class Position {
public:
    Piece* board[8][8];
    list<Piece*> pieces[2];
    Piece* king[2];
    int epRow;
    int epCol; // if not, both -1
    int halfMoveClock; // # of half-moves since the last capture/pawn advance
    int moveNumber; // # of moves since the beginning of the game
    color_t turn;
    unsigned long long hashValue; // Zobrist hash

    // build a null position
    Position();

    // build position from FEN notation
    Position(string FEN);

    // copy constructor
    Position(const Position &p);

    // destructor
    ~Position();

    // tests if two positions are the same
    bool equals(const Position &p);

    // build the FEN notation of the current position
    string toFEN();

    // performs a move
    void performMove(Move *m);

    // takes back the last move
    // must be called after a move perform!
    void revertLastMove();

    // returns the bonus score for the development of the piece
    double getPieceDevelopmentBonus(Piece *piece);

    // estimates the score of the position (when the search is cut)
    double getEstimatedScore(difficulty_t difficulty);

    // test if a piece directly attacks a square (the square of the piece doesn't count)
    bool pieceDirectlyAttacksSquare(Piece *piece, int row, int col);

    // puts all the posible moves of a piece in a list
    void getPieceAvailableMoves(Piece *piece, vector<Move*> &moves, bool justCaptures = false);

    // puts all valid moves in a list
    void getAllAvailableMoves(vector<Move*> &availableMoves);

    // puts important capture moves in a list
    void getQCaptureMoves(vector<Move*> &availableMoves);

    // a list of enemy pieces currently giving check
    list<Piece*> givingCheck;

    // dealocate dynamicly allocated moves
    static void deallocateMoveList(vector<Move*> &moveList);

    // sort moves that belong this position by the order of "importance"
    void sortMoves(vector<Move*> &moveList);

    // tests if the game's rules say that this position is a draw
    bool isDrawByRules();

    // returns the number of pieces no the board
    int getNumberOfPieces();

    void debug_printBoard();
private:
    // checks if a king is safe
    bool kingIsSafe(color_t side);

    // checks if a piece controls the center
    bool controlsCenter(Piece *piece);

    // checks if a pawn is passed
    bool pawnIsPassed(Piece *piece);

    // returns the king's distance to the center
    int pieceDistanceToCenter(Piece *piece);

    // returns a normalized pair = the relative position of p1 towards p2
    static pair<int, int> getRelativePosition(Piece *p1, Piece *p2);

    // returns a normalized pair = the relative position of p1 towards square (row, col)
    static pair<int, int> getRelativePosition(Piece *p1, int row, int col);

    // tests if the square is attacked by any of the opponent's pieces
    bool isSquareAttacked(int row, int col);

    // test if the player to move is in check
    bool isInCheck();

    // returns a list of the opponent's pieces attacking a square
    void piecesAttackingSquare(int row, int col, list<Piece*> &attackingSquare);

    // returns a list of the opponent's pieces giving check
    void piecesGivingCheck(list<Piece*> &givingCheck);

    // tests if a piece is pinned (cannot move)
    pair<int, int> isPinned(Piece *piece);

    // check if a move escapes check
    bool escapesCheck(Move *m, list<Piece*> &currentlyGivingCheck);

    // puts all the posible castling moves of the "side" king in a list
    void getKingAvailableCastlingMoves(color_t side, vector<Move*> &moves);

    // generates a table of random numbers used for hashing
    void generateRandomTable();

    // returns the hash value of the piece
    unsigned long long getPieceHashValue(Piece* piece);
public:
    // generates the hash value of the position
    void computeHashValue();
private:
    // debug
    void debug_consistencyCheck();

    // information needed for taking back a move
    Piece* lastCapturedPiece;
    Piece* lastPromotedPawn;
    Move* lastMove;
    list<Piece*> prevGivingCheck;
    int prevEpRow;
    int prevEpCol;
    int prevHalfMoveClock;
    int prevMoveNumber;
    unsigned long long prevHashValue;

    // a table of random numbers used for hashing
    static unsigned long long pieceHash[2][6][8][8];
    static unsigned long long turnHash[2];

    bool isEndgame;
};

#endif
