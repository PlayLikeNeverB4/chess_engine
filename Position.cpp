
#include "Position.h"

unsigned long long Position::pieceHash[2][6][8][8];
unsigned long long Position::turnHash[2];

Position::Position() {
}

Position::Position(string FEN) {
    stringstream ss(FEN);
    string str;

    // initializations
    king[0] = king[1] = NULL;
//"6rk/6b1/8/4B3/5Q1K/8/8/8 w - - 0 1"
    // the pieces
    ss >> str;
    size_t j = 0;
    for(int i = 7; i >= 0; i--) {
        int k = 0;
        while(j < str.size() && str[j] != '/') {
            if(isdigit(str[j]))
                for(int cnt = 0; cnt < str[j] - '0'; cnt++)
                    board[i][k++] = NULL;
            if(isalpha(str[j])) {
                if(islower(str[j])) {
                    board[i][k] = new Piece(Piece::getType(str[j]), BLACK, i, k);
                    pieces[BLACK].push_back(board[i][k]);
                }
                else {
                    board[i][k] = new Piece(Piece::getType(str[j]), WHITE, i, k);
                    pieces[WHITE].push_back(board[i][k]);
                }

                if(board[i][k]->type == KING)
                    king[board[i][k]->color] = board[i][k];

                k++;
            }
            j++;
        }

        if(k != 8 || (j == str.size() && i != 0)) {
            printf("FEN notation is not correct!\n");
            throw new exception();
        }

        j++; // jump over '/' character
    }

    // checks
    if(king[0] == NULL || king[1] == NULL) {
        printf("No king?!\n");
        throw new exception();
    }

    // who's turn
    ss >> str;
    turn = str[0] == 'w' ? WHITE : BLACK;

    // castling rights
    ss >> str;
    // we artificially mark the rooks as to have moved
    if(str.find('K') == string::npos && this->board[0][7] != NULL) this->board[0][7]->timesMoved = 1;
    if(str.find('Q') == string::npos && this->board[0][0] != NULL) this->board[0][0]->timesMoved = 1;
    if(str.find('k') == string::npos && this->board[7][7] != NULL) this->board[7][7]->timesMoved = 1;
    if(str.find('q') == string::npos && this->board[7][0] != NULL) this->board[7][0]->timesMoved = 1;

    if((king[WHITE]->squareRow != 0 || king[WHITE]->squareCol != 4)
       && (str.find('K') != string::npos || str.find('Q') != string::npos)) {
        printf("Something is wrong with castling rights in FEN! (white)\n");
        throw new exception;
    }
    if((king[BLACK]->squareRow != 7 || king[BLACK]->squareCol != 4)
       && (str.find('k') != string::npos || str.find('q') != string::npos)) {
        printf("Something is wrong with castling rights in FEN! (black)\n");
        throw new exception;
    }

    // is en-passant available?
    ss >> str;
    epRow = epCol = -1;
    if(str != "-") {
        epRow = str[1] - '1';
        epCol = str[0] - 'a';
    }

    // half-move clock
    ss >> str;
    halfMoveClock = atoi(str.c_str());

    // move number
    ss >> str;
    moveNumber = atoi(str.c_str());

    lastCapturedPiece = NULL;

    piecesGivingCheck(givingCheck);

    generateRandomTable();

    computeHashValue();
}

Position::Position(const Position &p) {
//    printf("Copy constructor!\n");
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            if(p.board[i][j] == NULL)
                this->board[i][j] = NULL;
            else {
                Piece *cpy = new Piece(*(p.board[i][j]));
                this->board[i][j] = cpy;
                this->pieces[cpy->color].push_back(cpy);
            }

    this->turn = p.turn;

    this->epRow = p.epRow;
    this->epCol = p.epCol;

    this->halfMoveClock = p.halfMoveClock;

    this->moveNumber = p.moveNumber;

    for(list<Piece*> :: const_iterator it = p.givingCheck.begin(); it != p.givingCheck.end(); it++)
        this->givingCheck.push_back(this->board[(*it)->squareRow][(*it)->squareCol]);

    this->king[0] = this->board[p.king[0]->squareRow][p.king[0]->squareCol];
    this->king[1] = this->board[p.king[1]->squareRow][p.king[1]->squareCol];

    // no need to copy prevGivingCheck, lastCapturedPiece etc. (will never use)
    this->prevGivingCheck.clear();

    this->lastCapturedPiece = NULL;

    this->hashValue = p.hashValue;

    this->halfMoveClock = p.halfMoveClock;

    this->moveNumber = p.moveNumber;
}

Position::~Position() {
    for(int i = 0; i < 2; i++)
        for(list<Piece*> :: iterator it = pieces[i].begin(); it != pieces[i].end(); it++)
            delete *it;
}

bool Position::equals(const Position &p) {
    if(this->turn != p.turn)
        return false;

    if(this->epRow != p.epRow || this->epCol != p.epCol) {
//        printf("EP squares not equal");
//        throw new exception();
        return false;
    }

    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++) {
            if((this->board[i][j] == NULL) != (p.board[i][j] == NULL))
                return false;
            if(this->board[i][j] == NULL)
                continue;
            if(this->board[i][j]->color != p.board[i][j]->color
               || this->board[i][j]->type != p.board[i][j]->type)
               return false;
        }

    string castle1 = "";
    if(this->board[0][7] != NULL && this->board[0][7]->timesMoved == 0) castle1 += "K";
    if(this->board[0][0] != NULL && this->board[0][0]->timesMoved == 0) castle1 += "Q";
    if(this->board[7][7] != NULL && this->board[7][7]->timesMoved == 0) castle1 += "k";
    if(this->board[7][0] != NULL && this->board[7][0]->timesMoved == 0) castle1 += "q";

    string castle2 = "";
    if(p.board[0][7] != NULL && p.board[0][7]->timesMoved == 0) castle1 += "K";
    if(p.board[0][0] != NULL && p.board[0][0]->timesMoved == 0) castle1 += "Q";
    if(p.board[7][7] != NULL && p.board[7][7]->timesMoved == 0) castle1 += "k";
    if(p.board[7][0] != NULL && p.board[7][0]->timesMoved == 0) castle1 += "q";

    if(castle1 != castle2) {
//        printf("Castling rights not equal");
//        throw new exception();
        return false;
    }

    return true;
}

void Position::generateRandomTable() {
    tr1::minstd_rand randomEngine;
    tr1::uniform_int<unsigned long long> dist(0, (unsigned long long)(-1));
    //randomEngine.seed(time(NULL));
    randomEngine.seed(13);

    for(int c = 0; c < 2; c++)
        for(int i = 0; i < 6; i++)
            for(int j = 0; j < 8; j++)
                for(int k = 0; k < 8; k++) {
                    pieceHash[c][i][j][k] = dist(randomEngine);
//                    printf("%d\n", hash[i][j][k]);
                }
    for(int c = 0; c < 2; c++)
        turnHash[c] = dist(randomEngine);
}

unsigned long long Position::getPieceHashValue(Piece* piece) {
    return pieceHash[piece->color][piece->type][piece->squareRow][piece->squareCol];
}

void Position::computeHashValue() {
    this->hashValue = 0;
    for(int side = 0; side < 2; side++)
        for(list<Piece*> :: iterator it = this->pieces[side].begin(); it != this->pieces[side].end(); it++)
            hashValue ^= getPieceHashValue(*it);
    hashValue ^= turnHash[turn];
}

string Position::toFEN() {
    int num;
    string ret = "";
    char aux[5];

    for(int i = 7; i >= 0; i--) {
		for(int j = 0; j <= 7; j++) {
			num = 0;
			while(j <= 7 && board[i][j] == NULL) {
				num++;
				j++;
			}
			if(num > 0) {
			    sprintf(aux, "%d", num);
				ret += string(aux);
				j--;
			}
			else ret += board[i][j]->getNotation();
		}
		if(i > 0)
			ret += "/";
	}

    ret += string(" ") + (turn == WHITE ? "w" : "b");

    ret += " ";

    string castle = "";
    if(this->board[0][7] != NULL && this->board[0][7]->timesMoved == 0) castle += "K";
    if(this->board[0][0] != NULL && this->board[0][0]->timesMoved == 0) castle += "Q";
    if(this->board[7][7] != NULL && this->board[7][7]->timesMoved == 0) castle += "k";
    if(this->board[7][0] != NULL && this->board[7][0]->timesMoved == 0) castle += "q";
    if(castle.size() == 0)
        castle = "-";
    ret += castle;

    ret += " ";
    if(epRow == -1)
        ret += "-";
    else {
        ret += (char)('a' + epCol);
        ret += (char)('1' + epRow);
    }

    sprintf(aux, " %d", halfMoveClock);
    ret += string(aux);

    sprintf(aux, " %d", moveNumber);
    ret += string(aux);

    return ret;
}

void Position::performMove(Move *m) {
    // saving stuff so we can revert
    lastMove = m;

    prevHashValue = hashValue;

    prevEpRow = epRow;
    prevEpCol = epCol;

    prevHalfMoveClock = halfMoveClock;
    prevMoveNumber = moveNumber;

    // assume EP will be off
    epRow = epCol = -1;

    halfMoveClock++;
    // if a pawn moved or it was a capture then the clock must be set to 0
    if(this->board[m->fromRow][m->fromCol]->type == PAWN || this->board[m->toRow][m->toCol] != NULL)
        halfMoveClock = 0;

    if(turn == BLACK)
        moveNumber++;

    if(!m->isSpecial) { // "normal" moves
        // deleting the current piece from the hash value
        hashValue ^= getPieceHashValue(this->board[m->fromRow][m->fromCol]);

        // is it a double pawn advance? => en-passant will become available
        if(this->board[m->fromRow][m->fromCol]->type == PAWN && abs(m->fromRow - m->toRow) == 2) {
            // between fromRow and toRow
            epRow = (m->fromRow + m->toRow) / 2;
            epCol = m->fromCol;
        }

        lastCapturedPiece = this->board[m->toRow][m->toCol];

        // making the move
        if(lastCapturedPiece != NULL) {
            pieces[1 - turn].erase(find(pieces[1 - turn].begin(), pieces[1 - turn].end(), lastCapturedPiece));

            // delete from hash
            hashValue ^= getPieceHashValue(lastCapturedPiece);
        }
        this->board[m->toRow][m->toCol] = this->board[m->fromRow][m->fromCol];
        this->board[m->fromRow][m->fromCol] = NULL;

        this->board[m->toRow][m->toCol]->squareRow = m->toRow;
        this->board[m->toRow][m->toCol]->squareCol = m->toCol;
        this->board[m->toRow][m->toCol]->timesMoved++;

        // add to hash
        hashValue ^= getPieceHashValue(this->board[m->toRow][m->toCol]);
    }
    else { // special moves

        if(m->special == KINGSIDE_CASTLE || m->special == QUEENSIDE_CASTLE) {
            lastCapturedPiece = NULL;

            // deleting the king from the hash value
            hashValue ^= getPieceHashValue(this->board[m->fromRow][m->fromCol]);

            // move the king
            this->board[m->toRow][m->toCol] = this->board[m->fromRow][m->fromCol];
            this->board[m->fromRow][m->fromCol] = NULL;

            this->board[m->toRow][m->toCol]->squareRow = m->toRow;
            this->board[m->toRow][m->toCol]->squareCol = m->toCol;
            this->board[m->toRow][m->toCol]->timesMoved++;

            // add the king to the hash value
            hashValue ^= getPieceHashValue(this->board[m->toRow][m->toCol]);

            // move the rook
            int rookFromCol = (m->special == KINGSIDE_CASTLE ? 7 : 0);
            int rookToCol = (m->special == KINGSIDE_CASTLE ? m->fromCol + 1 : m->fromCol - 1);

            // deleting the rook from the hash value
            hashValue ^= getPieceHashValue(this->board[m->fromRow][rookFromCol]);

            this->board[m->toRow][rookToCol] = this->board[m->fromRow][rookFromCol];
            this->board[m->fromRow][rookFromCol] = NULL;

            this->board[m->toRow][rookToCol]->squareRow = m->toRow;
            this->board[m->toRow][rookToCol]->squareCol = rookToCol;
            this->board[m->toRow][rookToCol]->timesMoved++;

            // add the rook to the hash value
            hashValue ^= getPieceHashValue(this->board[m->toRow][rookToCol]);
        }
        else if(m->special == EN_PASSANT) {
            if(prevEpRow != 2 && prevEpRow != 5) {
                printf("Something wrong with prevEP: row %d, col %d\n", prevEpRow, prevEpCol);
                throw new exception();
            }
            int advDir = this->board[m->fromRow][m->fromCol]->color == WHITE ? 1 : -1;

            // capturing the other pawn
            lastCapturedPiece = this->board[m->toRow - advDir][m->toCol];
            this->board[m->toRow - advDir][m->toCol] = NULL;
            pieces[1 - turn].erase(find(pieces[1 - turn].begin(), pieces[1 - turn].end(), lastCapturedPiece));

            // deleting the moving pawn from the hash value
            hashValue ^= getPieceHashValue(this->board[m->fromRow][m->fromCol]);
            // deleting the captured pawn from the hash value
            hashValue ^= getPieceHashValue(lastCapturedPiece);

            // moving the pawn
            this->board[m->toRow][m->toCol] = this->board[m->fromRow][m->fromCol];
            this->board[m->fromRow][m->fromCol] = NULL;

            this->board[m->toRow][m->toCol]->squareRow = m->toRow;
            this->board[m->toRow][m->toCol]->squareCol = m->toCol;
            this->board[m->toRow][m->toCol]->timesMoved++;

            // add the moving pawn to the hash value
            hashValue ^= getPieceHashValue(this->board[m->toRow][m->toCol]);
        }
        else if(m->special == PROMOTION) {
            lastCapturedPiece = this->board[m->toRow][m->toCol];
            lastPromotedPawn = this->board[m->fromRow][m->fromCol];

            // delete promoting pawn from hash
            hashValue ^= getPieceHashValue(lastPromotedPawn);

            // making the move
            if(lastCapturedPiece != NULL) {
                pieces[1 - turn].erase(find(pieces[1 - turn].begin(), pieces[1 - turn].end(), lastCapturedPiece));

                // delete captured piece from hash
                hashValue ^= getPieceHashValue(lastCapturedPiece);
            }
            // erase the pawn
            pieces[turn].erase(find(pieces[turn].begin(), pieces[turn].end(), lastPromotedPawn));

            // promoting to the selected piece
            this->board[m->toRow][m->toCol] = new Piece(m->promotionTo, turn, m->toRow, m->toCol);
            this->board[m->fromRow][m->fromCol] = NULL;
            pieces[turn].push_back(this->board[m->toRow][m->toCol]);

            // adding the new piece to the hash value
            hashValue ^= getPieceHashValue(this->board[m->toRow][m->toCol]);
        }
    }

    hashValue ^= turnHash[turn];
    hashValue ^= turnHash[1 - turn];

    // it's the opponent's turn
    turn = (color_t)(1 - turn);

    prevGivingCheck.clear();
    prevGivingCheck = givingCheck;
    givingCheck.clear();
    piecesGivingCheck(givingCheck);
}

void Position::revertLastMove() {
    if(lastMove == NULL) {
        printf("Last move doesn't exist!\n");
        throw new exception();
    }

    if(lastMove->fromRow == -1) {
        printf("Invalid last move!\n");
        throw new exception();
    }

    if(!lastMove->isSpecial) { // "normal" moves
        this->board[lastMove->fromRow][lastMove->fromCol] = this->board[lastMove->toRow][lastMove->toCol];
        this->board[lastMove->toRow][lastMove->toCol] = lastCapturedPiece;

        if(lastCapturedPiece != NULL)
            pieces[turn].push_back(lastCapturedPiece);

        this->board[lastMove->fromRow][lastMove->fromCol]->squareRow = lastMove->fromRow;
        this->board[lastMove->fromRow][lastMove->fromCol]->squareCol = lastMove->fromCol;
        this->board[lastMove->fromRow][lastMove->fromCol]->timesMoved--;
    }
    else { // special moves

        if(lastMove->special == KINGSIDE_CASTLE || lastMove->special == QUEENSIDE_CASTLE) {
            // unmove the king
            this->board[lastMove->fromRow][lastMove->fromCol] = this->board[lastMove->toRow][lastMove->toCol];
            this->board[lastMove->toRow][lastMove->toCol] = NULL;

            this->board[lastMove->fromRow][lastMove->fromCol]->squareRow = lastMove->fromRow;
            this->board[lastMove->fromRow][lastMove->fromCol]->squareCol = lastMove->fromCol;
            this->board[lastMove->fromRow][lastMove->fromCol]->timesMoved--;

            // unmove the rook
            int rookFromCol = (lastMove->special == KINGSIDE_CASTLE ? 7 : 0);
            int rookToCol = (lastMove->special == KINGSIDE_CASTLE ? lastMove->fromCol + 1 : lastMove->fromCol - 1);

            this->board[lastMove->fromRow][rookFromCol] = this->board[lastMove->toRow][rookToCol];
            this->board[lastMove->toRow][rookToCol] = NULL;

            this->board[lastMove->fromRow][rookFromCol]->squareRow = lastMove->fromRow;
            this->board[lastMove->fromRow][rookFromCol]->squareCol = rookFromCol;
            this->board[lastMove->fromRow][rookFromCol]->timesMoved--;
        }
        else if(lastMove->special == EN_PASSANT) {
            int advDir = this->board[lastMove->toRow][lastMove->toCol]->color == WHITE ? 1 : -1;

            // uncapturing the other pawn
            this->board[lastMove->toRow - advDir][lastMove->toCol] = lastCapturedPiece;
            pieces[turn].push_back(lastCapturedPiece);

            // unmoving the pawn
            this->board[lastMove->fromRow][lastMove->fromCol] = this->board[lastMove->toRow][lastMove->toCol];
            this->board[lastMove->toRow][lastMove->toCol] = NULL;

            this->board[lastMove->fromRow][lastMove->fromCol]->squareRow = lastMove->fromRow;
            this->board[lastMove->fromRow][lastMove->fromCol]->squareCol = lastMove->fromCol;
            this->board[lastMove->fromRow][lastMove->fromCol]->timesMoved--;

        }
        else if(lastMove->special == PROMOTION) {
            // erasing the newly created piece
            //delete this->board[lastMove->toRow][lastMove->toCol];
            pieces[1 - turn].erase(find(pieces[1 - turn].begin(), pieces[1 - turn].end(), this->board[lastMove->toRow][lastMove->toCol]));

            if(lastCapturedPiece != NULL)
                pieces[turn].push_back(lastCapturedPiece);
            this->board[lastMove->toRow][lastMove->toCol] = lastCapturedPiece;

            // reseting the pawn
            this->board[lastMove->fromRow][lastMove->fromCol] = lastPromotedPawn;
            pieces[1 - turn].push_back(lastPromotedPawn);
        }
    }

    // it was your turn
    turn = (color_t)(1 - turn);

    givingCheck = prevGivingCheck;

    epRow = prevEpRow;
    epCol = prevEpCol;

    halfMoveClock = prevHalfMoveClock;
    moveNumber = prevMoveNumber;

    hashValue = prevHashValue;
}

bool Position::isDrawByRules() {
    for(int side = 0; side < 2; side++)
        if(this->pieces[side].size() == 1) { // this side has just the king
            if(this->pieces[1 - side].size() == 1) // lone king
                return true;
            if(this->pieces[1 - side].size() == 2) { // king + minor piece
                for(list<Piece*> :: iterator it = this->pieces[1 - side].begin(); it != this->pieces[1 - side].end(); it++)
                    if((*it)->getValue() == 3)
                        return true;
            }
        }

    // 50-move rule
    if(halfMoveClock >= 50)
        return true;

    return false;
}

int Position::getNumberOfPieces() {
    return (int)(pieces[WHITE].size() + pieces[BLACK].size());
}

void Position::debug_printBoard() {
    printf("BOARD (turn = %d):\n", this->turn);
    for(int i = 7; i >= 0; i--) {
        for(int j = 0; j < 8; j++)
            if(this->board[i][j] == NULL)
                printf("_ ");
            else
                printf("%c ", this->board[i][j]->getNotation());
        printf("\n");
    }
}

double Position::getEstimatedScore(difficulty_t difficulty) {
    double score = 0.0;

    // material balance / count
    int whiteMaterial = 0;
    int blackMaterial = 0;
    for(int side = 0; side < 2; side++)
        for(list<Piece*> :: iterator it = pieces[side].begin(); it != pieces[side].end(); it++) {
            if((*it)->type != KING && (*it)->type != PAWN) {
                if(side == WHITE)
                    whiteMaterial += (*it)->getValue();
                else
                    blackMaterial -= (*it)->getValue();
            }

            if(side == turn)
                score += (*it)->getValue();
            else
                score -= (*it)->getValue();
        }

    // endgame: when both sides have less that or equal to 10 material count left ?!?
    // (not counting kings and pawns)
    bool isEndgame = (whiteMaterial <= ENDGAME_BOUND && blackMaterial <= ENDGAME_BOUND);

    if(!isEndgame) {
        for(int side = 0; side < 2; side++)
            for(list<Piece*> :: iterator it = pieces[side].begin(); it != pieces[side].end(); it++) {
                // developed pieces bonus
                if((*it)->type != KING && !((*it)->isOnHomeSquare())) {
                    if(side == turn)
                        score += this->getPieceDevelopmentBonus(*it);
                    else
                        score -= this->getPieceDevelopmentBonus(*it);
                }

                // controlling center bonus
                if(controlsCenter(*it)) {
                    if(side == turn)
                        score += CONTROL_CENTER_BONUS;
                    else
                        score -= CONTROL_CENTER_BONUS;
                }
            }

        // safe king bonus
        for(int side = 0; side < 2; side++)
            if(kingIsSafe((color_t)side)) {
                if(side == turn)
                    score += SAFE_KING_BONUS;
                else
                    score -= SAFE_KING_BONUS;
            }

        if(difficulty >= MEDIUM) {
            // connected rooks
            for(int side = 0; side < 2; side++) {
                Piece *rook1 = NULL;
                Piece *rook2 = NULL;

                for(list<Piece*> :: iterator it = pieces[side].begin(); it != pieces[side].end(); it++)
                    if((*it)->type == ROOK) {
                        if(rook1 == NULL)
                            rook1 = *it;
                        else {
                            rook2 = *it;
                            break;
                        }
                    }

                if(rook1 != NULL && rook2 != NULL
                && this->pieceDirectlyAttacksSquare(rook1, rook2->squareRow, rook2->squareCol)) {
                    if(side == turn)
                        score += CONNECTED_ROOKS_BONUS;
                    else
                        score -= CONNECTED_ROOKS_BONUS;
                }
            }
        }
    }
    else { // add endgame bonuses if it's the case
        // the kings' distance to center
        score += KING_DISTANCE_TO_CENTER_BONUS * pieceDistanceToCenter(king[1 - turn]);
        score -= KING_DISTANCE_TO_CENTER_BONUS * pieceDistanceToCenter(king[turn]);

        if(difficulty >= MEDIUM) {
            bool occupiedCols[8];
            int pawnIslands;

            for(int side = 0; side < 2; side++) {
                memset(occupiedCols, 0, sizeof(occupiedCols));

                for(list<Piece*> :: iterator it = pieces[side].begin(); it != pieces[side].end(); it++) {
                    // passed pawns
                    if((*it)->type == PAWN && pawnIsPassed(*it)) {
                        int promotionRow = ((*it)->color == WHITE ? 7 : 0);
                        double value = PASSED_PAWN_BONUS;

                        // if it's close to promoting, it's more valuable
                        if(abs(promotionRow - (*it)->squareRow) < 2)
                            value += 1.0;

                        if(side == turn)
                            score += value;
                        else
                            score -= value;
                    }

                    if((*it)->type == PAWN)
                        occupiedCols[(*it)->squareCol] = true;
                }

                // pawn islands
                pawnIslands = 0;
                for(int i = 0; i < 8; ) {
                    while(i < 8 && !occupiedCols[i])
                        i++;

                    if(i < 8) {
                        while(i < 8 && occupiedCols[i])
                            i++;
                        pawnIslands++;
                    }
                }

                if(side == turn)
                        score -= pawnIslands * 0.1;
                    else
                        score -= pawnIslands * 0.1;
            }
        }
    }

    // general advantages
    if(difficulty >= MEDIUM) {
        int numBishops;
        for(int side = 0; side < 2; side++) {
            numBishops = 0;
            for(list<Piece*> :: iterator it = pieces[side].begin(); it != pieces[side].end(); it++) {
                if((*it)->type == BISHOP)
                    numBishops++;
            }

            // bishop pair
            if(numBishops >= 2) {
                if(side == turn)
                    score += BISHOP_PAIR_BONUS;
                else
                    score -= BISHOP_PAIR_BONUS;
            }
        }
    }

    return score;
}

double Position::getPieceDevelopmentBonus(Piece *piece) {
    if(piece->type == KING)
        return 0.0;

    vector<Move*> moves;
    getPieceAvailableMoves(piece, moves);

    double bonus = 0.0;

    if(piece->type == QUEEN)
        bonus = (double)moves.size() * 0.02;
    else if(piece->type == ROOK) {
        // rook on 2nd/7th
        bonus = (double)moves.size() * 0.05;
        if(piece->squareRow == (piece->color == WHITE ? 6 : 1))
            bonus += ROOK_PIG_BONUS;
    }
    else if(piece->type == BISHOP)
        bonus = (double)moves.size() * 0.07;
    else if(piece->type == KNIGHT) {
        bonus = (double)moves.size() * 0.05;
        // if it's more advanced, it's more valuable
        bonus += (abs(piece->squareRow - (piece->color == WHITE ? 0 : 7))) * 0.05;
    }
    else if(piece->type == PAWN) {
        // how much it advanced
        bonus = (piece->squareRow - (piece->color == WHITE ? 1 : 6)) * 0.05;
    }

    deallocateMoveList(moves);

    return bonus;
}

bool Position::kingIsSafe(color_t side) {
    int safeRow = side == WHITE ? 0 : 7;

    if(king[side]->squareRow != safeRow)
        return false;

    int forward = side == WHITE ? 1 : -1;

    // he must have pieces in front of him
    for(int j = king[side]->squareCol - 1; j <= king[side]->squareCol + 1; j++)
        if(j >= 0 && j < 8 && this->board[king[side]->squareRow + forward][j] == NULL)
            return false;

    return true;
}

bool Position::controlsCenter(Piece *piece) {
    for(int pi = 3; pi <= 4; pi++)
        for(int pj = 3; pj <= 4; pj++)
            if(pieceDirectlyAttacksSquare(piece, pi, pj))
                return true;
    return false;
}

bool Position::pawnIsPassed(Piece *piece) {
    int advDir = piece->color == WHITE ? 1 : -1;

    // check for pawns on the files to the left/right
    for(int d = -1; d <= 1; d += 2)
        if(piece->squareCol + d >= 0 && piece->squareCol + d <= 7) {
            for(int pi = piece->squareRow + advDir; pi >= 1 && pi <= 6; pi += advDir)
                if(this->board[pi][piece->squareCol + d] != NULL
                   && this->board[pi][piece->squareCol + d]->type == PAWN
                   && this->board[pi][piece->squareCol + d]->color == 1 - turn)
                   return false;
        }

    return true;
}

int Position::pieceDistanceToCenter(Piece *piece) {
    if(piece == NULL) {
        printf("NULL piece!\n");
        throw new exception;
    }
    int rows = min(abs(piece->squareRow - 3), abs(piece->squareRow - 4));
    int cols = min(abs(piece->squareCol - 3), abs(piece->squareCol - 4));
    return rows + cols;
}

pair<int, int> Position::getRelativePosition(Piece *p1, Piece *p2) {
    // compute relative position
    int diri = p1->squareRow - p2->squareRow;
    int dirj = p1->squareCol - p2->squareCol;

    // normalize
    if(diri < 0) diri = -1;
    if(diri > 0) diri = 1;
    if(dirj < 0) dirj = -1;
    if(dirj > 0) dirj = 1;

    return make_pair(diri, dirj);
}

pair<int, int> Position::getRelativePosition(Piece *p1, int row, int col) {
    // compute relative position
    int diri = p1->squareRow - row;
    int dirj = p1->squareCol - col;

    // normalize
    if(diri < 0) diri = -1;
    if(diri > 0) diri = 1;
    if(dirj < 0) dirj = -1;
    if(dirj > 0) dirj = 1;

    return make_pair(diri, dirj);
}

bool Position::isSquareAttacked(int row, int col) {
    for(list<Piece*> :: iterator it = pieces[1 - turn].begin(); it != pieces[1 - turn].end(); it++)
        if(pieceDirectlyAttacksSquare(*it, row, col))
            return true;
    return false;
}

void Position::piecesAttackingSquare(int row, int col, list<Piece*> &attackingSquare) {
    for(list<Piece*> :: iterator it = pieces[1 - turn].begin(); it != pieces[1 - turn].end(); it++)
        if(pieceDirectlyAttacksSquare(*it, row, col))
            attackingSquare.push_back(*it);
}

bool Position::isInCheck() {
    // check if the current player's king is attacked by any of the opponent's pieces
    return isSquareAttacked(king[turn]->squareRow, king[turn]->squareCol);
}

void Position::piecesGivingCheck(list<Piece*> &givingCheck) {
    // check which of the opponent's pieces attacks the king
    return piecesAttackingSquare(king[turn]->squareRow, king[turn]->squareCol, givingCheck);
}

pair<int, int> Position::isPinned(Piece *piece) {
    // compute relative position towards the king
    int diri = king[turn]->squareRow - piece->squareRow;
    int dirj = king[turn]->squareCol - piece->squareCol;

    if(!(diri != 0 && dirj != 0 && abs(diri) != abs(dirj))) { // may be pinned
        // normalize
        if(diri > 0) diri = 1;
        if(diri < 0) diri = -1;
        if(dirj > 0) dirj = 1;
        if(dirj < 0) dirj = -1;

        // check if the path between this piece and the king is clear
        int pi = piece->squareRow + diri;
        int pj = piece->squareCol + dirj;

        while((pi != king[turn]->squareRow || pj != king[turn]->squareCol) && this->board[pi][pj] == NULL) {
            pi += diri;
            pj += dirj;
        }

        if(pi != king[turn]->squareRow || pj != king[turn]->squareCol) // we found a piece before the king
            return make_pair(NOT_PIN, NOT_PIN);

        // find the first piece on the opposite side
        diri = -diri;
        dirj = -dirj;

        pi = piece->squareRow + diri;
        pj = piece->squareCol + dirj;

        while(pi >= 0 && pi < 8 && pj >= 0 && pj < 8 && this->board[pi][pj] == NULL) {
            pi += diri;
            pj += dirj;
        }

        if(pi >= 0 && pi < 8 && pj >= 0 && pj < 8) { // a piece was found
            if(diri != 0 && dirj != 0) {
                // it's a diagonal => we need bishops or queens
                if((this->board[pi][pj]->type == QUEEN || this->board[pi][pj]->type == BISHOP)
                   && this->board[pi][pj]->color == (color_t)(1 - turn))
                   return make_pair(diri, dirj);
            }
            else {
                // it's a rank of file => we need rooks or queens
                if((this->board[pi][pj]->type == QUEEN || this->board[pi][pj]->type == ROOK)
                   && this->board[pi][pj]->color == (color_t)(1 - turn))
                   return make_pair(diri, dirj);
            }
        }
    }

    return make_pair(NOT_PIN, NOT_PIN);
}

bool Position::pieceDirectlyAttacksSquare(Piece *piece, int row, int col) {
    if(piece->type == KNIGHT || piece->type == KING)
        return piece->possiblyReachableSquare(row, col);

    if(piece->type == PAWN) {
        int advDir = piece->color == WHITE ? 1 : -1;
        return piece->squareRow + advDir == row && abs(piece->squareCol - col) == 1;
    }

    if(piece->possiblyReachableSquare(row, col)) { // pre-check
        pair<int, int> dir = getRelativePosition(piece, row, col);
        int diri = -dir.first;
        int dirj = -dir.second;

        // it's own square doesn't count
        if(diri == 0 && dirj == 0)
            return false;

        int pi = piece->squareRow + diri;
        int pj = piece->squareCol + dirj;

        // go along the path
        while((pi != row || pj != col) && this->board[pi][pj] == NULL) {
            pi += diri;
            pj += dirj;
        }

        if(pi != row || pj != col) // didn't reach destination
            return false;
        else
            return true;
    }

    return false;
}

void Position::getKingAvailableCastlingMoves(color_t side, vector<Move*> &moves) {
    // the king must not have moved to be able castle
    if(king[side]->timesMoved == 0 && king[side]->squareCol == 4) {
        int row = (side == WHITE ? 0 : 7);

        // king side castling
        if(this->board[row][7] != NULL && this->board[row][7]->type == ROOK
           && this->board[row][7]->color == WHITE && this->board[row][7]->timesMoved == 0
           && pieceDirectlyAttacksSquare(this->board[row][7], king[side]->squareRow, king[side]->squareCol)) {
                // and last, check if the king moves through check
                bool throughCheck = false;
                for(int col = king[side]->squareCol; col <= king[side]->squareCol + 2; col++)
                    if(isSquareAttacked(row, col)) {
                        throughCheck = true;
                        break;
                    }

                if(!throughCheck)
                    moves.push_back(new Move(king[side]->squareRow, king[side]->squareCol,
                                             king[side]->squareRow, king[side]->squareCol + 2, KINGSIDE_CASTLE));
           }

        // queen side castling
        if(this->board[row][0] != NULL && this->board[row][0]->type == ROOK
           && this->board[row][0]->color == WHITE && this->board[row][0]->timesMoved == 0
           && pieceDirectlyAttacksSquare(this->board[row][0], king[side]->squareRow, king[side]->squareCol)) {
                // and last, check if the king moves through check
                bool throughCheck = false;
                for(int col = king[side]->squareCol - 2; col <= king[side]->squareCol; col++)
                    if(isSquareAttacked(row, col)) {
                        throughCheck = true;
                        break;
                    }

                if(!throughCheck)
                    moves.push_back(new Move(king[side]->squareRow, king[side]->squareCol,
                                             king[side]->squareRow, king[side]->squareCol - 2, QUEENSIDE_CASTLE));
           }
    }
}

void Position::getPieceAvailableMoves(Piece *piece, vector<Move*> &moves, bool justQCaptures) {
    if(piece->type == KING) {
        // the king must not move into check

        int ni, nj;
        for(int d = 0; d < 8; d++) {
            ni = piece->squareRow + Piece::di[QUEEN][d];
            nj = piece->squareCol + Piece::dj[QUEEN][d];

            if(ni >= 0 && ni < 8 && nj >= 0 && nj < 8
               && (this->board[ni][nj] == NULL || this->board[ni][nj]->color == (color_t)(1 - turn))) {

                // temporarily delete the king
                this->board[piece->squareRow][piece->squareCol] = NULL;

                if(!isSquareAttacked(ni, nj)
                   && (this->board[ni][nj] == NULL || this->board[ni][nj]->color == (color_t)(1 - turn))) {
                    if(!(justQCaptures && (this->board[ni][nj] == NULL || this->board[ni][nj]->type == PAWN)))
                        moves.push_back(new Move(piece->squareRow, piece->squareCol, ni, nj));
                }

                // reset the king
                this->board[piece->squareRow][piece->squareCol] = piece;
            }
        }

        if(givingCheck.size() == 0) // if not check
            getKingAvailableCastlingMoves(turn, moves);
    }
    else { // this piece is not a king
        if(givingCheck.size() > 0) {
            if(isPinned(piece).first == NOT_PIN) {
                // this piece must either block the check or capture the piece that is giving check
                // if there are more than 1 pieces giving check then the king must move (pieces can't)
                if(givingCheck.size() == 1) {
                    // if the piece giving check is a knight or a pawn then we MUST capture him
                    Piece *other = *(givingCheck.begin());
                    if(other->type == KNIGHT || other->type == PAWN) {
                        if(pieceDirectlyAttacksSquare(piece, other->squareRow, other->squareCol)) {
                            // if this is a pawn (and checking piece is knight), it might promote
                            int promotionRow = (piece->color == WHITE ? 7 : 0);
                            if(piece->type == PAWN && other->squareRow == promotionRow) {
                                for(int pieceIndex = 1; pieceIndex <= 4; pieceIndex++)
                                    moves.push_back(new Move(piece->squareRow, piece->squareCol,
                                                              other->squareRow, other->squareCol, (piece_t)pieceIndex));
                            } // else, just capture
                            else moves.push_back(new Move(piece->squareRow, piece->squareCol,
                                                          other->squareRow, other->squareCol));
                        }

                        // en-passant
                        int advDir = piece->color == WHITE ? 1 : -1;
                        if(piece->type == PAWN && epRow == other->squareRow - advDir && epCol == other->squareCol
                           && piece->squareRow + 1 == epRow && abs(piece->squareCol - epCol) == 1)
                            moves.push_back(new Move(piece->squareRow, piece->squareCol,
                                                     other->squareRow - advDir, other->squareCol, EN_PASSANT));
                    }
                    else {
                        // check which moves block or capture
                        // (en-passant can't happen here)

                        pair<int, int> dir = getRelativePosition(other, king[turn]);
                        int diri = dir.first;
                        int dirj = dir.second;

                        int pi = king[turn]->squareRow;
                        int pj = king[turn]->squareCol;

                        // search on path between king - attacking piece
                        // see which square this piece can reach
                        do {
                            pi += diri;
                            pj += dirj;

                            if(piece->type == PAWN) {
                                int advDir = piece->color == WHITE ? 1 : -1;
                                bool isGood = false;

                                // single advance
                                if(!justQCaptures) {
                                    if(piece->squareCol == pj && piece->squareRow + advDir == pi && this->board[pi][pj] == NULL)
                                        isGood = true;
                                    // double advance (only 2nd / 7th rank)
                                    if(piece->squareRow == (piece->color == WHITE ? 1 : 6)
                                        && piece->squareRow == 1 && piece->squareCol == pj && piece->squareRow + 2 * advDir == pi
                                        && this->board[piece->squareRow + advDir][pj] == NULL && this->board[pi][pj] == NULL)
                                        isGood = true;
                                }

                                // capture
                                if(pi == other->squareRow && pj == other->squareCol
                                   && abs(piece->squareCol - pj) == 1 && piece->squareRow + advDir == pi)
                                    isGood = true;

                                if(isGood) { // a valid move was found
                                    // check if the pawn promotes
                                    int promotionRow = (piece->color == WHITE ? 7 : 0);
                                    if(pi == promotionRow) {
                                        for(int pieceIndex = 1; pieceIndex <= 4; pieceIndex++)
                                            moves.push_back(new Move(piece->squareRow, piece->squareCol, pi, pj,
                                                                     (piece_t)pieceIndex));
                                    }
                                    else
                                        moves.push_back(new Move(piece->squareRow, piece->squareCol, pi, pj));
                                }
                            } // this piece isn't pawn
                            else if(pieceDirectlyAttacksSquare(piece, pi, pj)) {
                                if(!(justQCaptures && (this->board[pi][pj] == NULL || this->board[pi][pj]->type == PAWN)))
                                    moves.push_back(new Move(piece->squareRow, piece->squareCol, pi, pj));
                            }
                        } while(pi != other->squareRow || pj != other->squareCol);
                    }
                }
            }
        }
        else { // it's not check
            // generate all possible moves

            pair<int, int> dirPin = isPinned(piece);
            bool isPin = dirPin.first != NOT_PIN;

            // the pawn is a little special
            if(piece->type == PAWN) {
                // white pawns go up and black ones go down
                int advDir = (piece->color == WHITE ? 1 : -1);
                int promotionRow = (piece->color == WHITE ? 7 : 0);

                if(piece->squareRow + advDir >= 0 && piece->squareRow + advDir < 8) {
                    // advance, must not be pinned diagonally or laterally
                    if(!(isPin && !(dirPin == make_pair(1, 0) || dirPin == make_pair(-1, 0))) && !justQCaptures) {
                        // single advance
                        if(this->board[piece->squareRow + advDir][piece->squareCol] == NULL) {
                            // the pawn might promote
                            if(piece->squareRow + advDir == promotionRow) {
                                for(int pieceIndex = 1; pieceIndex <= 4; pieceIndex++)
                                    moves.push_back(new Move(piece->squareRow, piece->squareCol,
                                                             piece->squareRow + advDir, piece->squareCol,
                                                            (piece_t)pieceIndex));
                            }
                            else
                                moves.push_back(new Move(piece->squareRow, piece->squareCol,
                                                         piece->squareRow + advDir, piece->squareCol));
                        }
                        // double advance (only 2nd / 7th rank)
                        if(piece->squareRow == (piece->color == WHITE ? 1 : 6)
                            && this->board[piece->squareRow + advDir][piece->squareCol] == NULL
                            && this->board[piece->squareRow + 2 * advDir][piece->squareCol] == NULL)
                            moves.push_back(new Move(piece->squareRow, piece->squareCol,
                                                     piece->squareRow + 2 * advDir, piece->squareCol));
                    }

                    // capture, must not pe pinned in other direction than the pinning piece
                    for(int j = -1; j <= 1; j += 2) {
                        if(piece->squareCol + j >= 0 && piece->squareCol + j < 8 && !(isPin && dirPin != make_pair(advDir, j))) {
                            // ordinary capture
                            if(this->board[piece->squareRow + advDir][piece->squareCol + j] != NULL
                            && this->board[piece->squareRow + advDir][piece->squareCol + j]->color == (color_t)(1 - turn)) {
                                // the pawn might promote
                                if(piece->squareRow + advDir == promotionRow) {
                                    for(int pieceIndex = 1; pieceIndex <= 4; pieceIndex++)
                                        moves.push_back(new Move(piece->squareRow, piece->squareCol,
                                                                 piece->squareRow + advDir, piece->squareCol + j,
                                                                 (piece_t)pieceIndex));
                                }
                                else
                                    moves.push_back(new Move(piece->squareRow, piece->squareCol,
                                                            piece->squareRow + advDir, piece->squareCol + j));
                            }

                            // en-passant
                            if(piece->squareRow + advDir == epRow && piece->squareCol + j == epCol)
                                moves.push_back(new Move(piece->squareRow, piece->squareCol,
                                                         epRow, epCol, EN_PASSANT));
                        }
                    }
                }
            }
            else { // it's not a pawn
                for(size_t d = 0; d < Piece::di[piece->type].size(); d++) {
                    int diri = Piece::di[piece->type][d];
                    int dirj = Piece::dj[piece->type][d];

                    // must not be pinned in other direction
                    if(isPin && dirPin != make_pair(diri, dirj))
                        continue;

                    int pi = piece->squareRow + diri;
                    int pj = piece->squareCol + dirj;

                    if(pi >= 0 && pi < 8 && pj >= 0 && pj < 8
                        && (this->board[pi][pj] == NULL || this->board[pi][pj]->color == (color_t)(1 - turn))) {
                        if(!(justQCaptures && (this->board[pi][pj] == NULL || this->board[pi][pj]->type == PAWN)))
                            moves.push_back(new Move(piece->squareRow, piece->squareCol, pi, pj));

                        if(piece->type != KNIGHT && this->board[pi][pj] == NULL) {
                            // we can go further
                            pi += diri;
                            pj += dirj;
                            while(pi >= 0 && pi < 8 && pj >= 0 && pj < 8 && this->board[pi][pj] == NULL) {
                                if(!justQCaptures)
                                    moves.push_back(new Move(piece->squareRow, piece->squareCol, pi, pj));
                                pi += diri;
                                pj += dirj;
                            }

                            if(pi >= 0 && pi < 8 && pj >= 0 && pj < 8 && this->board[pi][pj]->color == (color_t)(1 - turn)) {
                                // we reached an enemy piece
                                if(!(justQCaptures && this->board[pi][pj]->type == PAWN))
                                    moves.push_back(new Move(piece->squareRow, piece->squareCol, pi, pj));
                            }
                        }
                    }
                }
            }
        }
    }
}

void Position::getAllAvailableMoves(vector<Move*> &availableMoves) {
    for(list<Piece*> :: iterator it = pieces[turn].begin(); it != pieces[turn].end(); it++)
        getPieceAvailableMoves(*it, availableMoves);
}

void Position::getQCaptureMoves(vector<Move*> &captureMoves) {
    for(list<Piece*> :: iterator it = pieces[turn].begin(); it != pieces[turn].end(); it++)
        getPieceAvailableMoves(*it, captureMoves, true);
}

void Position::deallocateMoveList(vector<Move*> &moveList) {
    for(vector<Move*> :: iterator it = moveList.begin(); it != moveList.end(); it++)
        delete *it;
}

// not part of the class but very important
bool cmp(const Move* m1, const Move* m2) {
    if(m1 == m2)
        return false;

    if(m1->hashFlag != m2->hashFlag)
        return m1->hashFlag > m2->hashFlag;

    Position *p = m1->pos;

    bool isF1 = m1->isForcing();
    bool isF2 = m2->isForcing();

    if(isF1 != isF2)
        return isF1 > isF2;

    if(isF1 == true) { // they are both forcing
        // if the moving piece is less valuable, it's better
        int v1 = p->board[m1->fromRow][m1->fromCol]->getValue();
        int v2 = p->board[m2->fromRow][m2->fromCol]->getValue();

        if(v1 != v2)
            return v1 < v2;
        else {
            // if the captured piece is more valuable, it's better
            int c1 = p->board[m1->toRow][m1->toCol] != NULL ? p->board[m1->toRow][m1->toCol]->getValue() : -1;
            int c2 = p->board[m2->toRow][m2->toCol] != NULL ? p->board[m2->toRow][m2->toCol]->getValue() : -1;
            if(c1 != c2)
                return c1 > c2;
            else {
                // stupid comparison just to obtain a strict weak ordering
                if(m1->fromRow != m2->fromRow) return m1->fromRow < m2->fromRow;
                if(m1->fromCol != m2->fromCol) return m1->fromCol < m2->fromCol;
                if(m1->toRow != m2->toRow) return m1->toRow < m2->toRow;
                if(m1->toCol != m2->toCol) return m1->toCol < m2->toCol;
                if(m1->isSpecial != m2->isSpecial) return m1->isSpecial > m2->isSpecial;
                if(m1->special != m2->special) return m1->special < m2->special;
                if(m1->promotionTo != m2->promotionTo) return m1->promotionTo < m2->promotionTo;
                printf("Something went wrong while sorting the moves!\n");
                printf("Move1: %s\n", m1->toString().c_str());
                printf("Move2: %s\n", m2->toString().c_str());
                throw new exception();
            }
        }
    }
    else { // they are both unforcing
        // if the moving piece is more valuable, it's better
        int v1 = p->board[m1->fromRow][m1->fromCol]->getValue();
        int v2 = p->board[m2->fromRow][m2->fromCol]->getValue();

        if(v1 != v2)
            return v1 > v2;
        else {
            // stupid comparison just to obtain a strict weak ordering
            if(m1->fromRow != m2->fromRow) return m1->fromRow < m2->fromRow;
            if(m1->fromCol != m2->fromCol) return m1->fromCol < m2->fromCol;
            if(m1->toRow != m2->toRow) return m1->toRow < m2->toRow;
            if(m1->toCol != m2->toCol) return m1->toCol < m2->toCol;
            if(m1->isSpecial != m2->isSpecial) return m1->isSpecial > m2->isSpecial;
            if(m1->special != m2->special) return m1->special < m2->special;
            if(m1->promotionTo != m2->promotionTo) return m1->promotionTo < m2->promotionTo;
            printf("Something went wrong while sorting the moves!\n");
            printf("Move1: %s\n", m1->toString().c_str());
            printf("Move2: %s\n", m2->toString().c_str());
            throw new exception();
        }
    }

    // the function shouldn't reach this point
    printf("Something went wrong while sorting the moves!\n");
    printf("End of function reached!\n");
    throw new exception();
    return true;
}

void Position::sortMoves(vector<Move*> &moveList) {
    // sort in decreasing order of "importance"
    for(vector<Move*> :: iterator it = moveList.begin(); it != moveList.end(); it++)
        (*it)->setPosition(this);
    sort(moveList.begin(), moveList.end(), cmp);
}

void Position::debug_consistencyCheck() {
    int pi, pj;
    for(int i = 0; i < 2; i++)
        for(list<Piece*> :: iterator it = pieces[i].begin(); it != pieces[i].end(); it++) {
            pi = (*it)->squareRow;
            pj = (*it)->squareCol;
            if(this->board[pi][pj] != *it) {
                printf("Consistency check failed!\n");
                throw new exception();
            }
        }
}
