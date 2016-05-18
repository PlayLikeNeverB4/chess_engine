
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "types.h"
#include "Move.h"
#include "Position.h"
#include "HashTableEntry.h"

using namespace std;
using namespace std::tr1;

class Engine {
public:
    // initializes the engine
    Engine(difficulty_t difficulty, int hashTableSize);

    // deallocated the hash table etc.
    ~Engine();

    // loads the opening book from a file (with our format)
    void loadOpeningBook(string filename);

    // search for the best move (min-max algo + alpha-beta)
    pair<Move, pair<double, int> > getBestMove(Position *pos);

    static int evaluatedNodes;
    static int totalFound;
    static int totalCollisions;
private:
    int depth;
    int hashTableSize;
    int hashBitMask;
    difficulty_t difficulty;
    static int difficultyToDepth[];

    // size defined at runtime
    HashTableEntry** hashTable;

    tr1::unordered_map<string, vector<Move> > openingBook;

    void init(int depth, int hashTableSize);

    pair<Move, double> miniMax(Position *p, double alpha, double beta, int depthLeft);

    double quiescence(Position *p, double alpha, double beta, int depth);
};

#endif
