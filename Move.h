
#ifndef _MOVE_H_
#define _MOVE_H_

#include "types.h"

using namespace std;

class Move {
public:
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
    bool isSpecial;
    spec_move_t special; // in case it's a special move, which one
    piece_t promotionTo; // in case it's a promotion, to what
    Position *pos;
    bool hashFlag;

    // default constructor
    Move();

    // constructor
    Move(int fromRow, int fromCol, int toRow, int toCol);

    // special move constructor
    Move(int fromRow, int fromCol, int toRow, int toCol, spec_move_t special);

    // promotion constructor
    Move(int fromRow, int fromCol, int toRow, int toCol, piece_t promotionTo);

    // copy constructor
    Move(const Move &m);

    // = operator
    Move& operator =(const Move &m);

    // build move from a string
    Move(string str);

    // == operator
    bool operator ==(const Move &m);

    // destructor
    ~Move();

    // set the position this move belongs to
    void setPosition(Position *p);

    // test if this move is forcing (for sorting)
    bool isForcing() const;

    // test if this move is a capture
    bool isCapture() const;

    // for debugging purposes
    string toString() const;

private:
    // initializes members (called in constructors)
    void init(int fromRow, int fromCol, int toRow, int toCol);
};


#endif


