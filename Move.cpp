
#include "Move.h"
#include "Piece.h"
#include "Position.h"

void Move::init(int fromRow, int fromCol, int toRow, int toCol) {
    this->fromRow = fromRow;
    this->fromCol = fromCol;
    this->toRow = toRow;
    this->toCol = toCol;
    this->isSpecial = false;
    this->pos = NULL;
    this->hashFlag = false;
}

Move::Move() {
    init(-1, -1, -1, -1);
}

Move::Move(int fromRow, int fromCol, int toRow, int toCol) {
    init(fromRow, fromCol, toRow, toCol);
}

Move::Move(int fromRow, int fromCol, int toRow, int toCol, spec_move_t special) {
    init(fromRow, fromCol, toRow, toCol);
    this->isSpecial = true;
    this->special = special;
}

Move::Move(int fromRow, int fromCol, int toRow, int toCol, piece_t promotionTo) {
    init(fromRow, fromCol, toRow, toCol);
    this->isSpecial = true;
    this->special = PROMOTION;
    this->promotionTo = promotionTo;

    if(promotionTo == PAWN || promotionTo == KING) {
        printf("Cannot promote to that piece!\n");
        throw new exception();
    }
}

Move::Move(const Move &m) {
    init(m.fromRow, m.fromCol, m.toRow, m.toCol);
    if(m.isSpecial) {
        this->isSpecial = true;
        this->special = m.special;
        this->promotionTo = m.promotionTo;
    }
}

Move& Move::operator =(const Move &m) {
    init(m.fromRow, m.fromCol, m.toRow, m.toCol);
    if(m.isSpecial) {
        this->isSpecial = true;
        this->special = m.special;
        this->promotionTo = m.promotionTo;
    }
    return *this;
}

Move::Move(string str) {
    // format: "A3 B4 EP"

    if(str.size() < 4) {
        printf("Error in the move string initializer!\n");
        throw new exception();
    }

    init(-1, -1, -1, -1);

    string fromStr = str.substr(0, 2);
    string toStr = str.substr(3, 2);
    string specialStr = "";

    if(str.size() > 6)
        specialStr = str.substr(6);

    this->fromRow = fromStr[1] - '1';
    this->fromCol = fromStr[0] - 'A';
    this->toRow = toStr[1] - '1';
    this->toCol = toStr[0] - 'A';

    if(specialStr.size() > 0) {
        this->isSpecial = true;

        if(specialStr == "EP")
            this->special = EN_PASSANT;
        if(specialStr == "CASTLE") {
            if(this->fromCol < this->toCol)
                this->special = KINGSIDE_CASTLE;
            else
                this->special = QUEENSIDE_CASTLE;
        }
        if(specialStr.size() == 1) {
            this->special = PROMOTION;
            this->promotionTo = Piece::getType(specialStr[0]);
        }
    }
}

bool Move::operator ==(const Move &m) {
//    printf("== operator!\n");
    if(this->fromRow != m.fromRow) return false;
    if(this->fromCol != m.fromCol) return false;
    if(this->toRow != m.toRow) return false;
    if(this->toCol != m.toCol) return false;
    if(this->isSpecial != m.isSpecial) return false;
    if(this->isSpecial) {
        if(this->special != m.special) return false;
        if(this->special == PROMOTION && this->promotionTo != m.promotionTo) return false;
    }
    return true;
}

Move::~Move() { }

void Move::setPosition(Position *p) {
    this->pos = p;
}

bool Move::isForcing() const {
    // it's a capture
    if(pos->board[this->toRow][this->toCol] != NULL)
        return true;

    // is it check ?
    // temporarily puts the piece to it's destination
    // no need to modify the pos->board
    pos->board[this->fromRow][this->fromCol]->squareRow = this->toRow;
    pos->board[this->fromRow][this->fromCol]->squareCol = this->toCol;

    bool isCheck = false;
    if(pos->pieceDirectlyAttacksSquare(pos->board[this->fromRow][this->fromCol],
                                       pos->king[1 - pos->turn]->squareRow, pos->king[1 - pos->turn]->squareCol))
        isCheck = true;

    pos->board[this->fromRow][this->fromCol]->squareRow = this->fromRow;
    pos->board[this->fromRow][this->fromCol]->squareCol = this->fromCol;

    return isCheck;
}

bool Move::isCapture() const {
    return pos->board[this->toRow][this->toCol] != NULL;
}

string Move::toString() const {
    char aux[50];
    sprintf(aux, "%c%c %c%c", fromCol + 'A', fromRow + '1', toCol + 'A', toRow + '1');
    string ret(aux);

    if(this->isSpecial && this->special == PROMOTION)
        ret += string(" ") + Piece::getNotation(promotionTo);

    return ret;
}
