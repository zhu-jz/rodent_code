/*
  Rodent, a UCI chess playing engine derived from Sungorus 1.4
  Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
  Copyright (C) 2011-2014 Pawel Koziol

  Rodent is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published 
  by the Free Software Foundation, either version 3 of the License, 
  or (at your option) any later version.

  Rodent is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty 
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#define BUILD 25
#define BENCH_8 788536

#undef CDECL

#if defined(_WIN32) || defined(_WIN64)
   #define llu_format  "%I64u"
   typedef unsigned long long U64;
   typedef unsigned long      U32;
   #define CDECL __cdecl
#else
   #include <stdint.h>
   #define llu_format  "%llu"
   typedef uint64_t U64;
   typedef uint32_t U32;
   #define CDECL
#endif

#define MAX_INT 2147483646
#define MAX_ELO 2600
#define NO_PC 12

enum eColor {WHITE, BLACK, NO_CL};
enum ePiece {P, N, B, R, Q, K, NO_TP};
enum eDir   {HOR, VER, DIAG_AH, DIAG_HA};
enum eFile  {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
enum eRank  {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};
enum eMoveType {NORMAL, CASTLE, EP_CAP, EP_SET, N_PROM, B_PROM, R_PROM, Q_PROM};
enum eCastleFlag { W_KS = 1, W_QS = 2, B_KS = 4, B_QS = 8};
enum eGamePhase {MG, EG};
enum eHashEntry {NONE, UPPER, LOWER, EXACT};
enum eProtocol {PROTO_UCI, PROTO_WB, PROTO_TXT};

enum eSquare {
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8,
  NO_SQ
};

enum eMoveFlag {
   FLAG_NORMAL_MOVE = 0,
   FLAG_NULL_EVASION,
   FLAG_HASH_MOVE,
   FLAG_KILLER_MOVE,
   FLAG_GOOD_CAPTURE,
   FLAG_BAD_CAPTURE,
};

// move macros
#define Fsq(move)				((move) & 63)				// "from" square of a move "x"
#define Tsq(move)				(((move) >> 6) & 63)		// "to" square of a move "x"
#define MoveType(move)			((move) >> 12)				// type of a move "x" (see eMoveType)
#define IsProm(move)			((move) & 0x4000)			// is this move a promotion?
#define PromType(move)			(((move) >> 12) - 3)		// kind of promoted piece
#define SetMove(flag,fr,to)     ( (flag<<12)|(to<<6)|fr )

// search depth variables
#define ONE_PLY         4
#define MAX_PLY         64
#define MAX_MOVES       256
#define INF             32765 // was 32767, we save some space for flags
#define INVALID         -32767
#define MATE            32000
#define MAX_EVAL        29999

#define SIDE_RANDOM     (~((U64)0))

#define START_POS       "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"

#define Cl(x)           ((x) & 1)
#define Tp(x)           ((x) >> 1)
#define Pc(x, y)        (((y) << 1) | (x))
#define Opp(x)          ((x) ^ 1)           // other color

#define File(x)         ((x) & 7)           // file on which square "x" is placed
#define Rank(x)         ((x) >> 3)          // rank on which square "x" is placed
#define Sq(x, y)        (((y) << 3) | (x))  // square with a given coordinates

#define Abs(x)          ((x) > 0 ? (x) : -(x))
#define Max(x, y)       ((x) > (y) ? (x) : (y))
#define Min(x, y)       ((x) < (y) ? (x) : (y))
#define Map0x88(x)      (((x) & 7) | (((x) & ~7) << 1))
#define Unmap0x88(x)    (((x) & 7) | (((x) & ~7) >> 1))
#define Sq0x88Off(x)    ((unsigned)(x) & 0x88)

#define InCheck(p)      IsAttacked(p, KingSq(p, p->side), Opp(p->side))
#define IllegalPosition(p)  IsAttacked(p, KingSq(p, Opp(p->side)), p->side)

#define bbPc(p, x, y)   ((p)->bbCl[x] & (p)->bbTp[y])
#define OccBb(p)        ((p)->bbCl[WHITE] | (p)->bbCl[BLACK])
#define UnoccBb(p)      (~OccBb(p))
#define TpOnSq(p, x)    (Tp((p)->pc[x]))
#define KingSq(p, x)    ((p)->kingSquare[x])

#define RankIndex(o, x) (((o) >> ((070 & (x)) + 1)) & 63)
#define FileIndex(o, x) (((bbFILE_A & ((o) >> File(x))) * (U64)0x0204081020408000) >> 58)
#define DiagIndex(o, x) ((((o) & bbLineMask[2][x]) * bbFILE_B) >> 58)
#define AntiIndex(o, x) ((((o) & bbLineMask[3][x]) * bbFILE_B) >> 58)

#define HorAttacks(o, x) attacks[HOR]    [x][RankIndex(o, x)]
#define VerAttacks(o, x) attacks[VER]    [x][FileIndex(o, x)]
#define AHDAttacks(o, x) attacks[DIAG_AH][x][DiagIndex(o, x)]
#define HADAttacks(o, x) attacks[DIAG_HA][x][AntiIndex(o, x)]

#define RAttacks(o, x)  (HorAttacks(o, x) | VerAttacks(o, x))
#define BAttacks(o, x)  (AHDAttacks(o, x) | HADAttacks(o, x))
#define QAttacks(o, x)  (RAttacks(o, x)   | BAttacks(o, x))

typedef struct         // board representation:
{
  U64 bbCl[2];         // color bitboard
  U64 bbTp[6];         // piece type bitboard
  int pcCount[2][6];   // count of pieces of a given color and type
  int phase;           // incrementally calculated game phase
  int pc[64];          // piece type on a given square
  int kingSquare[2];   // king location for each side
  int pieceMat[2];     // non-pawn material for each side
  int pstMg[2];        // incrementally updated midgame pst score
  int pstEg[2];        // incrementally updated endgame pst score
  int side;            // side to move
  int castleFlags;     // castling flags
  int epSquare;        // square where an en passant capture can be made (or invalid)
  int reversibleMoves; // no. of reversible moves played in a row (not captures, not pawn moves)
  int head;
  U64 hashKey;
  U64 pawnKey;
  U64 repetitionList[256];
} sPosition;

typedef struct  // set of move lists subdivided into move classes
{
  sPosition *p;
  int phase;
  int transMove;
  int killer1;
  int killer2;
  int refutation;
  int *next;
  int *last;
  int move[MAX_MOVES];
  int value[MAX_MOVES];
  int *badp;
  int bad[MAX_MOVES];
} MOVES;

struct sSelector // class that holds move lists and returns moves in predefined order
{
private:
	MOVES m[1];  // move list
	int MvvLva(sPosition *p, int move);
	void ScoreCaptures(int hashMove);
	void ScoreQuiet(int refutationSq);
	int PickBestMove(void);
public:
	int CaptureIsBad(sPosition *p, int move);
	void InitCaptureList(sPosition *p, int hashMove);
	void InitMoveList(sPosition *p, int refMove, int contMove, int transMove, int ply);
	int NextMove(int refutationSq, int *flag);
	int NextCapture(void);
};  // implemented in selector.c

typedef struct // move list without classification, but with scoring methods, used at root
{
  int moves[MAX_MOVES];
  int value[MAX_MOVES];
  int used[MAX_MOVES];
  int bestMove;
  int bestVal;
  int nOfMoves;
  int currMoveIndex;
  void Init(sPosition *p);
  void AddMove(int move);
  void ClearUsed(int bestMove);
  void ScoreLastMove(int move, int val);
  int GetNextMove(void);
} sFlatMoveList; // implemented in selector.c

typedef struct 
{
  int ttp;
  int castleFlags;
  int epSquare;
  int reversibleMoves;
  U64 hashKey;
  U64 pawnKey;
} UNDO;

typedef struct 
{
  void DoMove(sPosition *p, int move, UNDO *u);
  void DoNull(sPosition *p, UNDO *u);
  void UndoMove(sPosition *p, int move, UNDO *u);
  void UndoNull(sPosition *p, UNDO *u);
} sManipulator;

extern sManipulator Manipulator;

int IsAttacked(sPosition *p, int sq, int side);
U64 AttacksFrom(sPosition *p, int sq);
U64 AttacksTo(sPosition *p, int sq);
void BuildPv(int *dst, int *src, int move);
int *GenerateCaptures(sPosition *p, int *list);
int *GenerateQuiet(sPosition *p, int *list);
void Init(void);
int InputAvailable(void);
int IsLegal(sPosition *p, int move);
void MoveToStr(int move, char *moveString);
void PrintMove(int move);
void PvToStr(int *pv, char *pv_str);
U64 Random64(void);
void SetPosition(sPosition *p, char *epd);
int StrToMove(sPosition *p, char *moveString);
int Swap(sPosition *p, int from, int to);
U64 atoull(const char *s);

extern U64 bbPawnSupport[2][64];
extern U64 bbRAttacksOnEmpty[64];
extern U64 bbBAttacksOnEmpty[64];
extern U64 bbRCanAttack[64][64];
extern U64 bbBCanAttack[64][64];
extern U64 bbQCanAttack[64][64];
extern U64 bbLineMask[4][64];

extern U64 attacks[4][64][64];
/* first slot denotes direction: 0 is -, 1 is |, 2 is / and 3 is \
   second slot = start square, third slot = occupancy mask */

extern U64 bbPawnAttacks[2][64];
extern U64 bbKnightAttacks[64];
extern U64 bbKingAttacks[64];
extern U64 bbKingZone[2][64];

extern U64 bbPassedMask[2][64];
extern U64 bbAdjacentMask[8];

extern int castleMask[64];
extern const int bitTable[64];
extern U64 zobPiece[12][64];
extern U64 zobCastle[16];
extern U64 zobEp[8];
extern int pondering;
extern char ponder_str[6];
extern int flagProtocol;
