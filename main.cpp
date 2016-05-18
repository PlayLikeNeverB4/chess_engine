#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <bitset>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctype.h>
#include <cstring>
#include <ctime>

using namespace std;

#include "Position.h"
#include "Engine.h"


int main(int argc, char **argv) {
    if(argc != 3) {
        printf("Usage: [FEN notation] [DIFFICULTY: 0-3]\n");
        return 1;
    }

    int aux = atoi(argv[2]);
    if(aux < 0 || aux > 3) {
        printf("The difficulty should be in [0, 3]\n");
        return 1;
    }
    difficulty_t difficulty = (difficulty_t)aux;

    string FEN(argv[1]);
    Position pos(FEN);

    clock_t start;
    clock_t end;

    start = clock();
    Engine e(difficulty, 1 << 20);
    e.loadOpeningBook("geobot_opening_book.txt");
    end = clock();
    //cout << "Initialized engine in: " << fixed << setprecision(2) << (double)(end - start) / CLOCKS_PER_SEC << endl;

    start = clock();
    pair<Move, pair<double, int> > res = e.getBestMove(&pos);
    end = clock();

    cout << res.first.toString() << endl;

    FILE *log = fopen("log.txt", "a");
    if(log == NULL) {
        printf("Can't open log file!\n");
        return 1;
    }

    fprintf(log, "%s => ", argv[1]);

    if(res.second.first == BOOK_MOVE)
        fprintf(log, "(BM) ");
    else
        fprintf(log, "(%.2lf, D=%d) ", res.second.first, res.second.second);

    fprintf(log, "%s\n", res.first.toString().c_str());
    fclose(log);

    //cout << "Score: " << fixed << setprecision(2) << res.second << endl;
    //cout << "Time: " << fixed << setprecision(2) << (double)(end - start) / CLOCKS_PER_SEC << endl;
    //cout << "Evaluated nodes: " << Engine::evaluatedNodes << endl;
    //cout << "Speed: " << fixed << setprecision(2) << Engine::evaluatedNodes / ((double)(end - start) / CLOCKS_PER_SEC)
    //        << " nodes / second"<< endl;
    //cout << "Found in hash table: " << Engine::totalFound << endl;
    //cout << "Collisions in hash table: " << Engine::totalCollisions << endl;

    return 0;
}
