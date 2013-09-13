/*
  Rodent, a UCI chess playing engine derived from Sungorus 1.4
  Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
  Copyright (C) 2011-2013 Pawel Koziol

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

#include "rodent.h"
#include "data.h"
#include "bitboard/bitboard.h"

const int passerMg    = 10;
const int passerEg    = 13;
const int neutral[8]  = {-3, -1, 1, 3, 3, 1, -1, -3};
const int biased[8]   = {-3, -1, 0, 1, 1, 0, -1, -3};
const int knightEg[8] = {-4, -2, 0, 1, 1, 0, -2, -4};
const int pawnAdv[8]  = { 0,  1, 1, 3, 5, 8, 12,  0};

const int pstKnightMg[64] = 
{
   -50, -40, -30, -25, -25, -30, -40, -50,
   -35, -25, -15, -10, -10, -15, -25, -35,
   -20, -10,   0,   5,   5,   0, -10, -20, 
   -10,   0,  10,  15,  15,  10,   0, -10, 
	-5,   5,  15,  20,  20,  15,   5,  -5,
    -5,   5,  15,  20,  20,  15,   5,  -5,
   -20, -10,   0,   5,   5,   0, -10, -20,
  -135, -25, -15, -10, -10, -15, -25, -135
};

const int pstBishopMg[64] =  
{
    -5,  -5, -16,  -5,  -5, -16,  -5,  -5,
    -5,   3,   3,   3,   3,   3,   3,  -5,
    -5,   5,   5,   5,   5,   5,   5,  -5,
    -5,   0,   5,  10,  10,   5,   0,  -5,
    -5,   0,   5,  10,  10,   5,   0,  -5,
    -5,   0,   5,   5,   5,   5,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5
};

const int pstRookMg[64] = 
{  
  	 0,   0,   2,   4,   4,   2,   0,   0,
    -4,  -2,   0,   2,   2,   0,  -2,  -4,
    -4,  -2,   0,   2,   2,   0,  -2,  -4,
    -4,  -2,   0,   2,   2,   0,  -2,  -4,
    -4,  -2,   0,   2,   2,   0,  -2,  -4,
    -4,  -2,   0,   2,   2,   0,  -2,  -4,
    -4,  -2,   0,   2,   2,   0,  -2,  -4,
     4,   4,   4,   4,   4,   4,   4,   4
};

const int pstQueenEg[64] = 
{
   -24, -16, -12,  -8,  -8, -12, -16, -24,
   -16, -12,  -4,   0,   0,  -4, -12, -16,
   -12,  -4,   0,   4,   4,   0,  -4, -12,
    -8,   0,   4,   8,   8,   4,   0,  -8,
    -8,   0,   4,   8,   8,   4,   0,  -8,
   -12,  -4,   0,   4,   4,   0,  -4, -12,
   -16, -12,  -4,   0,   0,  -4, -12, -16,
   -24, -16, -12,  -8,  -8, -12, -16, -24
};

const int pstKingMg[64] = 
{  
    40,  50,  30,  10,  10,  30,  50,  40,
    30,  40,  20,   0,   0,  20,  40,  30,
    10,  20,   0, -20, -20,   0,  20,  10,
     0,  10, -10, -30, -30, -10,  10,   0,
   -10,   0, -20, -40, -40, -20,   0, -10,
   -20, -10, -30, -50, -50, -30, -10, -20,
   -30, -20, -40, -60, -60, -40, -20, -30,
   -40, -30, -50, -70, -70, -50, -30, -40
};

const int pstIsolatedMg[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstIsolatedEg[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
   -16, -18, -20, -22, -22, -20, -18, -16,
   -16, -18, -20, -22, -22, -20, -18, -16,
   -16, -18, -20, -22, -22, -20, -18, -16,
   -16, -18, -20, -22, -22, -20, -18, -16,
   -16, -18, -20, -22, -22, -20, -18, -16,
   -16, -18, -20, -22, -22, -20, -18, -16,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstBackwardMg[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
    -7,  -8,  -9, -10, -10,  -9,  -8,  -7,
    -7,  -8,  -9, -10, -10,  -9,  -8,  -7,
    -7,  -8,  -9, -10, -10,  -9,  -8,  -7,
    -7,  -8,  -9, -10, -10,  -9,  -8,  -7,
    -7,  -8,  -9, -10, -10,  -9,  -8,  -7,
    -7,  -8,  -9, -10, -10,  -9,  -8,  -7,
	 0,   0,   0,   0,   0,   0,   0,   0
};

const int pstBackwardEg[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
    -8,  -9, -10, -12, -12, -10,  -9,  -8,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstKnightOutpost[64] = 
{   
	 0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   2,   3,   6,   6,   3,   2,   0,
     0,   2,   6,   9,   9,   6,   2,   0,
     0,   0,   9,  12,  12,   9,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstBishopOutpost[64] = 
{   
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   1,   2,   4,   4,   2,   1,   0,
     0,   1,   4,   6,   6,   4,   1,   0,
     0,   0,   6,   8,   8,   6,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstRookOutpost[64] = 
{   
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   1,   1,   2,   2,   1,   1,   0,
     8,   8,   6,   4,   4,   6,   8,   8,
    10,  10,   8,   6,   6,   8,  10,  10,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstBadBishop[64] =  
{
	 0,   0, -20,   0,   0, -20,   0,   0, 
   -10,  -5,  -5,  -5,  -5,  -5,  -5, -10,
    -5, -10,  -5,  -5,  -5,  -5, -10,  -5,
     0,   0,  -3,   0,   0,  -3,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

void sData::InitPstValues(void)
{
  for (int sq = 0; sq < 64; sq++) {  
	for (int side = 0; side < 2; side++) {

	  pstMg[side][P][REL_SQ(sq,side)] = GetPawnMgPst(sq);
	  pstMg[side][N][REL_SQ(sq,side)] = pstKnightMg[sq];
	  pstMg[side][B][REL_SQ(sq,side)] = pstBishopMg[sq];
	  pstMg[side][R][REL_SQ(sq,side)] = pstRookMg[sq];
	  pstMg[side][Q][REL_SQ(sq,side)] = GetQueenMgPst(sq);
	  pstMg[side][K][REL_SQ(sq,side)] = pstKingMg[sq];

	  pstEg[side][P][REL_SQ(sq,side)] = GetPawnEgPst(sq);
	  pstEg[side][N][REL_SQ(sq,side)] = GetKnightEgPst(sq);
	  pstEg[side][B][REL_SQ(sq,side)] = GetBishopEgPst(sq);
	  pstEg[side][R][REL_SQ(sq,side)] = GetRookEgPst(sq);
	  pstEg[side][Q][REL_SQ(sq,side)] = pstQueenEg[sq];
	  pstEg[side][K][REL_SQ(sq,side)] = 12 * ( biased[Rank(sq)] + biased[File(sq)] );

	  pawnProperty[PHALANX]  [MG] [side] [REL_SQ(sq,side)]  = GetPhalanxPstMg(sq);
	  pawnProperty[PHALANX]  [EG] [side] [REL_SQ(sq,side)]  = 0; // on modifying please uncomment relevant line in eval_pawns.c
	  pawnProperty[PASSED]   [MG] [side] [REL_SQ(sq,side)]  = passerMg * pawnAdv[Rank(sq)];
	  pawnProperty[PASSED]   [EG] [side] [REL_SQ(sq,side)]  = passerEg * pawnAdv[Rank(sq)];
	  pawnProperty[CANDIDATE][MG] [side] [REL_SQ(sq,side)]  = ( passerMg * pawnAdv[Rank(sq)] ) / 3;
	  pawnProperty[CANDIDATE][EG] [side] [REL_SQ(sq,side)]  = ( passerEg * pawnAdv[Rank(sq)] ) / 3;
	  pawnProperty[ISOLATED] [MG] [side] [REL_SQ(sq,side)] = pstIsolatedMg[sq];
	  pawnProperty[ISOLATED] [EG] [side] [REL_SQ(sq,side)] = pstIsolatedEg[sq];
	  pawnProperty[BACKWARD] [MG] [side] [REL_SQ(sq,side)] = pstBackwardMg[sq];
	  pawnProperty[BACKWARD] [EG] [side] [REL_SQ(sq,side)] = pstBackwardEg[sq];

	  outpost[side][N][REL_SQ(sq,side)] = pstKnightOutpost[sq];
	  outpost[side][B][REL_SQ(sq,side)] = pstBishopOutpost[sq];
	  outpost[side][R][REL_SQ(sq,side)]   = pstRookOutpost[sq];
	  badBishopPenalty[side] [REL_SQ(sq,side)] = pstBadBishop[sq];
    }
  }
}

int sData::GetPawnMgPst(int sq)
{
	if ( sq == D2 || sq == E2 ) return 5;
	if ( sq == D4 || sq == E4 ) return 25;
	if ( sq == D6 || sq == E6 ) return 5;
	if ( sq == D7 || sq == E7 ) return 5;

    return neutral[File(sq)] * 5;
}

int sData::GetPawnEgPst(int sq)
{
    return 10;
}

int sData::GetKnightEgPst(int sq)
{
    return 5 * ( knightEg[Rank(sq)] + knightEg[File(sq)] );
}

int sData::GetBishopEgPst(int sq)
{
    if ( Rank(sq) == RANK_1 || Rank(sq) == RANK_8 || File(sq) == FILE_A || File(sq) == FILE_H) return -5;
	if ( Rank(sq) == RANK_2 || Rank(sq) == RANK_7 || File(sq) == FILE_B || File(sq) == FILE_G) return  0;
	if ( Rank(sq) == RANK_3 || Rank(sq) == RANK_6 || File(sq) == FILE_C || File(sq) == FILE_F) return  5;
	return 10;
}

int sData::GetRookEgPst(int sq)
{
    return 0;
}

int sData::GetQueenMgPst(int sq)
{
    if ( Rank(sq) == RANK_1) return -5;
	else                     return 0;
}

int sData::GetPhalanxPstMg(int sq)
{
    if (sq == D4) return 15;             // D4/E4 pawns
	if (sq == C4 || sq == E4) return 10; // C4/D4 or E4/F4 pawns
	return 0;
}