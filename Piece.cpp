
#include "Piece.h"
#include "Move.h"

vector<int> Piece::di[6];
vector<int> Piece::dj[6];
bool Piece::directionsInitialized = false;

void Piece::init(piece_t type, color_t color, int row, int col) {
    this->type = type;
    this->color = color;
    this->squareRow = row;
    this->squareCol = col;
    this->timesMoved = 0;

    // if this is the first time using a Piece
    if(!directionsInitialized) {
        // pawn
        int pawni[] = { 1 };
        int pawnj[] = { 0 };
        di[PAWN].assign(pawni, pawni + 1);
        dj[PAWN].assign(pawnj, pawnj + 1);

        // knight
        int knighti[] = { 2, 1,  2,  1, -1, -2, -1, -2 };
        int knightj[] = { 1, 2, -1, -2,  2,  1, -2, -1 };
        di[KNIGHT].assign(knighti, knighti + 8);
        dj[KNIGHT].assign(knightj, knightj + 8);

        // bishop
        int bishopi[] = { 1,  1, -1, -1 };
        int bishopj[] = { 1, -1,  1, -1 };
        di[BISHOP].assign(bishopi, bishopi + 4);
        dj[BISHOP].assign(bishopj, bishopj + 4);

        // rook
        int rooki[] = { 1,  -1,  0, 0 };
        int rookj[] = { 0,   0, -1, 1 };
        di[ROOK].assign(rooki, rooki + 4);
        dj[ROOK].assign(rookj, rookj + 4);

        // queen
        int queeni[] = { 0, -1, -1, -1,  0,  1, 1, 1 };
        int queenj[] = { 1,  1,  0, -1, -1, -1, 0, 1 };
        di[QUEEN].assign(queeni, queeni + 8);
        dj[QUEEN].assign(queenj, queenj + 8);

        // king uses queen's

        directionsInitialized = true;
    }
}

Piece::Piece(piece_t type, color_t color, int row, int col) {
    init(type, color, row, col);
}

Piece::Piece(const Piece &p) {
    init(p.type, p.color, p.squareRow, p.squareCol);
    this->timesMoved = p.timesMoved;
}

Piece::~Piece() {
}

bool Piece::possiblyReachableSquare(int row, int col) {
    int advDir, vi, vj;
    switch(type) {
    case PAWN:  advDir = (this->color == WHITE ? 1 : -1);
                return this->squareRow + advDir == row && abs(this->squareCol - col) <= 1;
    case KNIGHT: /*for(int d = 0; d < 8; d++)
                    if(this->squareRow + di[KNIGHT][d] == row && this->squareCol + dj[KNIGHT][d] == col)
                        return true;
                 return false;*/
                 vi = abs(this->squareRow - row);
                 vj = abs(this->squareCol - col);
                 return (vi == 1 && vj == 2) || (vi == 2 && vj == 1);
    case BISHOP: return abs(this->squareRow - row) == abs(this->squareCol - col);
    case ROOK:   return this->squareRow == row || this->squareCol == col;
    case QUEEN:  return this->squareRow == row || this->squareCol == col
                || abs(this->squareRow - row) == abs(this->squareCol - col);
    case KING:   return abs(this->squareRow - row) <= 1 && abs(this->squareCol - col) <= 1;
    }

    printf("Invalid piece!\n");
    throw new exception();
    return false;
}

char Piece::getNotation() {
    char ret;

    switch(type) {
        case KING : ret = 'k'; break;
        case QUEEN : ret = 'q'; break;
        case ROOK : ret = 'r'; break;
        case BISHOP : ret = 'b'; break;
        case KNIGHT : ret = 'n'; break;
        case PAWN : ret = 'p'; break;
        default :   // unknown piece
                    printf("Invalid piece!\n");
                    throw new exception();
    }

    if(color == WHITE)
        ret = toupper(ret);

    return ret;
}

char Piece::getNotation(piece_t pieceType) {
    return Piece(pieceType, WHITE, -1, -1).getNotation();
}

piece_t Piece::getType(char c) {
    c = toupper(c);
    switch(c) {
        case 'K' : return KING;
        case 'Q' : return QUEEN;
        case 'R' : return ROOK;
        case 'B' : return BISHOP;
        case 'N' : return KNIGHT;
        case 'P' : return PAWN;
        default:    // unknown piece
                    printf("Invalid piece notation!\n");
                    throw new exception();
    }

    return KING;
}

int Piece::getValue() {
    switch(type) {
        case KING : return 1000;
        case QUEEN : return 9;
        case ROOK : return 5;
        case BISHOP : return 3;
        case KNIGHT : return 3;
        case PAWN : return 1;
        default :   // unknown piece
                    printf("Invalid piece!\n");
                    throw new exception();
    }

    return -1;
}

bool Piece::isOnHomeSquare() {
    int firstRow = this->color == WHITE ? 0 : 7;
    int secondRow = this->color == WHITE ? 1 : 6;

    switch(type) {
        case KING : return this->squareRow == firstRow && this->squareCol == 4;
        case QUEEN : return this->squareRow == firstRow && this->squareCol == 3;
        case ROOK : return this->squareRow == firstRow && (this->squareCol == 0 || this->squareCol == 7);
        case BISHOP : return this->squareRow == firstRow && (this->squareCol == 2 || this->squareCol == 5);
        case KNIGHT : return this->squareRow == firstRow && (this->squareCol == 1 || this->squareCol == 6);
        case PAWN : return this->squareRow == secondRow;
        default :   // unknown piece
                    printf("Invalid piece!\n");
                    throw new exception();
    }
}
