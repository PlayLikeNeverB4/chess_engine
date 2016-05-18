
#include "HashTableEntry.h"

HashTableEntry::HashTableEntry(Position *pos, score_t scoreType, double score, Move move, int depth) : pos(*pos) {
    this->scoreType = scoreType;
    this->score = score;
    this->move = move;
    this->depth = depth;
    this->used = false;
}

