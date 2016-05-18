
#ifndef _PIECE_H
#define _PIECE_H

#include "types.h"

using namespace std;

class Piece {
public:
    piece_t type;
    color_t color;
    int squareRow; // 0 - based
    int squareCol;
    int timesMoved; // used for castling rights

    // constructor
    Piece(piece_t type, color_t color, int row, int col);

    // copy constructor
    Piece(const Piece &p);

    // destructor
    ~Piece();

    // test if this piece can theoretically reach a square
    bool possiblyReachableSquare(int row, int col);

    // returns a char containing the FEN notation of the piece
    char getNotation();

    // returns a char containing the FEN notation of a piece (static version)
    static char getNotation(piece_t pieceType);

    // converts FEN notation of piece into piece_t
    static piece_t getType(char c);

    // returns the value of the current piece
    int getValue();

    // checks if this piece is on home square (probably)
    bool isOnHomeSquare();

    // the directions that pieces can move towards
    static vector<int> di[6];
    static vector<int> dj[6];

private:
    static bool directionsInitialized;

    // initializes members (called in constructors)
    void init(piece_t type, color_t color, int row, int col);
};


#endif

