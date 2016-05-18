
#ifndef _HASH_TABLE_ENTRY_H_
#define _HASH_TABLE_ENTRY_H_

#include "Position.h"

class HashTableEntry {
public:
    Position pos;
    score_t scoreType;
    double score;
    Move move;
    int depth;
    bool used;

    HashTableEntry(Position *pos, score_t scoreType, double score, Move move, int depth);
};

#endif
