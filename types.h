
#ifndef _MYTYPES_H_
#define _MYTYPES_H_

#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <cstring>
#include <utility>

#include <tr1/random>
#include <tr1/unordered_map>

typedef enum { WHITE, BLACK } color_t;
typedef enum { KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN } piece_t;
typedef enum { KINGSIDE_CASTLE, QUEENSIDE_CASTLE, PROMOTION, EN_PASSANT } spec_move_t;
typedef enum { EXACT, LOWER_BOUND } score_t;
typedef enum { EASY, MEDIUM, HARD, FORCE } difficulty_t;

// declaring all existing classes
class Move;
class Piece;
class Position; // an arrangement of the pieces on the board

const double INF = 1000.0;
const double BOOK_MOVE = 123456789.0;
const int NOT_PIN = -2;

// maximum number of material count in an endgame
const int ENDGAME_BOUND = 10;

const double KING_DISTANCE_TO_CENTER_BONUS = 0.025;
const double PASSED_PAWN_BONUS = 0.1;
const double ROOK_PIG_BONUS = 0.25;
const double CONNECTED_ROOKS_BONUS = 0.3;
const double BISHOP_PAIR_BONUS = 0.2;
//const double DEVELOPED_PIECE_BONUS = 0.1;
const double CONTROL_CENTER_BONUS = 0.05;
const double SAFE_KING_BONUS = 0.3;

#endif
