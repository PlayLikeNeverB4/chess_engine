
#include "Engine.h"

int Engine::evaluatedNodes;
int Engine::totalFound;
int Engine::totalCollisions;
int Engine::difficultyToDepth[] = { 3, 4, 4, 4 };

void Engine::init(int depth, int hashTableSize) {
    if(hashTableSize <= 0 || hashTableSize > 200000000) {
        printf("The hash table size is not correct!\n");
        throw new exception();
    }

    if((hashTableSize & (hashTableSize -1)) > 0) {
        printf("The hash table size is not correct (not a power of two)!\n");
        throw new exception();
    }

    this->depth = depth;
    this->hashTableSize = hashTableSize;
    this->hashBitMask = hashTableSize - 1;
    hashTable = new HashTableEntry*[hashTableSize];
}

Engine::Engine(difficulty_t difficulty, int hashTableSize) {
    this->difficulty = difficulty;
    init(difficultyToDepth[difficulty], hashTableSize);
}

Engine::~Engine() {
    // delete the entries from the hash table

    for(int i = 0; i < hashTableSize; i++)
        if(hashTable[i] != NULL)
            delete hashTable[i];
    delete hashTable;
}

void Engine::loadOpeningBook(string filename) {
    ifstream input(filename.c_str());

    if(!input) {
        printf("Opening book file (\"%s\") can't be opened!\n", filename.c_str());
        throw new exception();
    }

    string aux, line, FEN, moveStr;

    while(input) {
        getline(input, line);
        stringstream ss(line);

        if(line.size() < 20) // blank line or short comment
            continue;

        FEN = "";
        for(int part = 0; part < 6; part++) {
            ss >> aux;
            FEN += aux;
            if(aux == "//")
                break;
            if(part < 5)
                FEN += " ";
        }

        if(FEN == "//") // this line is a comment
            continue;

        moveStr = "";
        ss >> moveStr;
        ss >> aux;
        moveStr = moveStr + " " + aux;

        aux = "";
        ss >> aux;
        if(aux.size() > 0)
            moveStr = moveStr + " " + aux;

        if(openingBook.count(FEN) == 0) {
            vector<Move> v;
            v.push_back(Move(moveStr));
            openingBook[FEN] = v;
        }
        else
            openingBook[FEN].push_back(Move(moveStr));
    }
}

pair<Move, pair<double, int> > Engine::getBestMove(Position *pos) {
    string FEN = pos->toFEN();
    //printf("%s\n", FEN.c_str());

    if(openingBook.count(FEN) > 0) {
        srand(time(NULL));
        unsigned int randomMove = rand() % openingBook[FEN].size();
//        printf("%d %d\n", openingBook[FEN].size(), randomMove);
        return make_pair(openingBook[FEN][randomMove], make_pair(BOOK_MOVE, 0));
    }

    int runOnDepth = this->depth;

    if(difficulty >= HARD) {
        if(pos->getNumberOfPieces() <= 20) {
            runOnDepth = 5;
            if(pos->getNumberOfPieces() <= 14) {
                runOnDepth = 6;
                if(pos->getNumberOfPieces() <= 10) {
                    runOnDepth = 7;
                    if(pos->getNumberOfPieces() <= 8) {
                        runOnDepth = 8;
                    }
                }
            }
        }
//        if(pos->getNumberOfPieces() <= 14) {
//            runOnDepth = 6;
//            if(pos->getNumberOfPieces() <= 8) {
//                runOnDepth = 8;
//            }
//        }
    }

    if(difficulty == FORCE)
        runOnDepth++;

    pair<Move, double> res = miniMax(pos, -INF * 2, +INF * 2, runOnDepth);
    pair<Move, pair<double, int> > ret = make_pair(res.first, make_pair(res.second, runOnDepth));
    return ret;
}

#define __HASH_TABLE
pair<Move, double> Engine::miniMax(Position *p, double alpha, double beta, int depthLeft) {
    pair<Move, double> best = make_pair(Move(), -INF);
    pair<Move, double> crt;
    vector<Move*> availableMoves;

    /*evaluatedNodes++;
    if(evaluatedNodes % 1000000 == 0)
        printf("%d\n", evaluatedNodes);
    */

    if(p->isDrawByRules())
        return make_pair(Move(), 0.0);

    // find valid moves
    p->getAllAvailableMoves(availableMoves);

    if(availableMoves.size() == 0) { // the game is over
        if(p->givingCheck.size() > 0) { // it's checkmate
            best.second -= depthLeft; // (not perfect, but better)
            return best;
        }
        else                       // it's stalemate
            return make_pair(Move(), 0.0);
    }

    // cut the search
    if(depthLeft == 0) {
        Position::deallocateMoveList(availableMoves);
        return make_pair(Move(), quiescence(p, alpha, beta, 1));
    }

#ifdef __HASH_TABLE
    int hashTablePos = p->hashValue & this->hashBitMask;
    bool foundInTable = false;

    if(hashTable[hashTablePos] != NULL && hashTable[hashTablePos]->pos.equals(*p) && hashTable[hashTablePos]->depth >= depthLeft) {
//        printf("Found %d!\n", hashTable[hashTablePos]->scoreType);
        totalFound++;
        foundInTable = true;
        hashTable[hashTablePos]->used = true;

        if(hashTable[hashTablePos]->scoreType == EXACT) {
            Position::deallocateMoveList(availableMoves);
            return make_pair(hashTable[hashTablePos]->move, hashTable[hashTablePos]->score);
        }
        else // LOWER_BOUND or not sufficient depth
            alpha = max(alpha, hashTable[hashTablePos]->score);

        vector<Move*> :: iterator m = availableMoves.begin();
        for(; m != availableMoves.end(); m++)
            if(hashTable[hashTablePos]->move == **m)
                break;

        if(m == availableMoves.end()) {
            printf("Move not found, something wrong with the hash table!\n");
            printf("move: %s\n", hashTable[hashTablePos]->move.toString().c_str());
            p->debug_printBoard();
            throw new exception();
        }

        // highest priority in the move order
        (*m)->hashFlag = true;
    }
#endif

    // auxiliary object on which we perform the moves
    Position next(*p);

    next.sortMoves(availableMoves);

    score_t scoreType = EXACT;

    for(vector<Move*> :: iterator it = availableMoves.begin(); it != availableMoves.end(); it++) {
        next.performMove(*it);

        //fprintf(stderr, "%s\n", (*it)->toString().c_str());

        crt = miniMax(&next, -beta, -alpha, depthLeft - 1);
        crt.second = -crt.second;

        next.revertLastMove();

        alpha = max(alpha, crt.second);

        if(crt.second > best.second || best.first.fromRow == -1) {
            best.first = Move(**it);
            best.second = crt.second;
        }

        if(best.second >= beta) {
            scoreType = LOWER_BOUND;
            break;
        }
    }

    Position::deallocateMoveList(availableMoves);

#ifdef __HASH_TABLE
    if(foundInTable) {
        // we try to update

        if(!hashTable[hashTablePos]->pos.equals(*p))
            throw new exception();

        hashTable[hashTablePos]->scoreType = scoreType;
        hashTable[hashTablePos]->score = best.second;
        hashTable[hashTablePos]->depth = depthLeft;
    }
    else {
        // try to put it in the table
        if((hashTable[hashTablePos] == NULL) || (!hashTable[hashTablePos]->used)) {
            if(hashTable[hashTablePos] != NULL) {
                if(hashTable[hashTablePos]->pos.equals(*p) && depthLeft < hashTable[hashTablePos]->depth) {
                    printf("p: %d\n", depthLeft);
                    printf("HT: %d\n", hashTable[hashTablePos]->depth);
                    throw new exception();
                }
//                printf("Collision!\n");
//                p->debug_printBoard();
//                hashTable[hashTablePos]->pos.debug_printBoard();
//                printf("%d\n", hashTable[hashTablePos]->pos.equals(*p));
                totalCollisions++;
                delete hashTable[hashTablePos];
            }
            hashTable[hashTablePos] = new HashTableEntry(p, scoreType, best.second, best.first, depthLeft);
        }
    }
#endif

    return best;
}

double Engine::quiescence(Position *p, double alpha, double beta, int depth) {
    double crt;
    vector<Move*> captureMoves;

    if(p->isDrawByRules())
        return 0.0;

    double standPad = p->getEstimatedScore(this->difficulty);

    if(standPad >= beta)
        return beta;
    alpha = max(alpha, standPad);

    // find only captures
    p->getQCaptureMoves(captureMoves);

    if(captureMoves.size() == 0) { // the position is quiet
        if(p->givingCheck.size() > 0) {
            vector<Move*> availableMoves;
            // could be optimized
            p->getAllAvailableMoves(availableMoves);

            if(availableMoves.size() == 0) // it's checkmate
                return -INF - (100 - depth); // (not perfect, but better)
            else
                return standPad;
        }
        else
            return standPad;
    }

    /*evaluatedNodes++;
    if(evaluatedNodes % 1000000 == 0)
        printf("%d\n", evaluatedNodes);
    */

    // auxiliary object
    Position next(*p);

    next.sortMoves(captureMoves);
//    printf("Q: %d\n", depth);
    for(vector<Move*> :: iterator it = captureMoves.begin(); it != captureMoves.end(); it++) {
        next.performMove(*it);

        //printf("%s // %d\n", (*it)->toString().c_str(), depth);
        crt = quiescence(&next, -beta, -alpha, depth + 1);
        crt = -crt;

        next.revertLastMove();

        alpha = max(alpha, crt);

        if(crt >= beta)
            break;
    }

    Position::deallocateMoveList(captureMoves);

    return alpha;
}

