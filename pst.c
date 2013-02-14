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

#define PE    10 // endgame material gain of a pawn

int pstPawnMg[64] = {
	0,   0,   0,   0,   0,   0,   0,   0,
  -15,  -5,   0,   5,   5,   0,  -5, -15,
  -15,  -5,   5,  15,  15,   5,  -5, -15,
  -15,  -5,   5,  25,  25,   5,  -5, -15,
  -15,  -5,   5,  15,  15,   5,  -5, -15,
  -15,  -5,   5,   5,   5,   5,  -5, -15,
  -15,  -5,   5,   5,   5,   5,  -5, -15,
    0,   0,   0,   0,   0,   0,   0,   0
};

// material endgame bonus only
const int pstPawnEg[64] = 
{    
    PE,  PE,  PE,  PE,  PE,  PE,  PE,  PE,
    PE,  PE,  PE,  PE,  PE,  PE,  PE,  PE,
    PE,  PE,  PE,  PE,  PE,  PE,  PE,  PE,
    PE,  PE,  PE,  PE,  PE,  PE,  PE,  PE,
    PE,  PE,  PE,  PE,  PE,  PE,  PE,  PE,
    PE,  PE,  PE,  PE,  PE,  PE,  PE,  PE,
    PE,  PE,  PE,  PE,  PE,  PE,  PE,  PE,
    PE,  PE,  PE,  PE,  PE,  PE,  PE,  PE
};

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

const int pstKnightEg[64] = 
{
   -40, -30, -20, -15, -15, -20, -30, -40,
   -30, -20, -10,  -5,  -5, -10, -20, -30,
   -20, -10,   0,   5,   5,   0, -10, -20,
   -15,  -5,   5,  10,  10,   5,  -5, -15,
   -15,  -5,   5,  10,  10,   5,  -5, -15,
   -20, -10,   0,   5,   5,   0, -10, -20,
   -30, -20, -10,  -5,  -5, -10, -20, -30,
   -40, -30, -20, -15, -15, -20, -30, -40
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

const int pstBishopEg[64] = 
{
    -5,  -5, - 5,  -5,  -5,  -5,  -5,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   5,   5,   5,   5,   0,  -5,
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

const int pstRookEg[64] =  
{
	 0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
	 0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstQueenMg[64] = 
{ 
    -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
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

const int pstKingEg[64] = 
{ 
   -72, -48, -36, -24, -24, -36, -48, -72,
   -48, -24, -12,   0,   0, -12, -24, -48,
   -36, -12,   0,  12,  12,   0, -12, -36,
   -24,   0,  12,  24,  24,  12,   0, -24,
   -24,   0,  12,  24,  24,  12,   0, -24,
   -36, -12,   0,  12,  12,   0, -12, -36,
   -48, -24, -12,   0,   0, -12, -24, -48,
   -72, -48, -36, -24, -24, -36, -48, -72
};

const int pstPhalanxMg[64] = 
{ 
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,  10,  15,  10,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstPhalanxEg[64] = // on modifying this table please uncomment a line in eval_pawns.c
{ 
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
	 0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstPasserMg[64] = 
{   
	 0,   0,   0,   0,  0,    0,   0,   0,
    10,  10,  10,  10,  10,  10,  10,  10,
    10,  10,  10,  10,  10,  10,  10,  10,
    30,  30,  30,  30,  30,  30,  30,  30,
    50,  50,  50,  50,  50,  50,  50,  50,
    80,  80,  80,  80,  80,  80,  80,  80,
   120, 120, 120, 120, 120, 120, 120, 120,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int pstPasserEg[64] = 
{   
	 0,   0,   0,   0,  0,    0,   0,   0,
    12,  12,  12,  12,  12,  12,  12,  12,
    12,  12,  12,  12,  12,  12,  12,  12,
    38,  38,  38,  38,  38,  38,  38,  38,
    63,  63,  63,  63,  63,  63,  63,  63,
   100, 100, 100, 100, 100, 100, 100, 100,
   150, 150, 150, 150, 150, 150, 150, 150,
     0,   0,   0,   0,   0,   0,   0,   0
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
  for (int i = 0; i < 64; i++) {  
	for (int side = 0; side < 2; side++) {

	  pstMg[side][P][REL_SQ(i,side)] = pstPawnMg[i];
	  pstMg[side][N][REL_SQ(i,side)] = pstKnightMg[i];
	  pstMg[side][B][REL_SQ(i,side)] = pstBishopMg[i];
	  pstMg[side][R][REL_SQ(i,side)] = pstRookMg[i];
	  pstMg[side][Q][REL_SQ(i,side)] = pstQueenMg[i];
	  pstMg[side][K][REL_SQ(i,side)] = pstKingMg[i];

	  pstEg[side][P][REL_SQ(i,side)] = pstPawnEg[i];
	  pstEg[side][N][REL_SQ(i,side)] = pstKnightEg[i];
	  pstEg[side][B][REL_SQ(i,side)] = pstBishopEg[i];
	  pstEg[side][R][REL_SQ(i,side)] = pstRookEg[i];
	  pstEg[side][Q][REL_SQ(i,side)] = pstQueenEg[i];
	  pstEg[side][K][REL_SQ(i,side)] = pstKingEg[i];

	  phalanxMg[side][REL_SQ(i,side)]  = pstPhalanxMg[i];
	  passersMg[side][REL_SQ(i,side)]  = pstPasserMg[i];
	  passersEg[side][REL_SQ(i,side)]  = pstPasserEg[i];
	  candidateMg[side][REL_SQ(i,side)]  = pstPasserMg[i] / 3;
	  candidateEg[side][REL_SQ(i,side)]  = pstPasserEg[i] / 3;
	  isolatedMg[side][REL_SQ(i,side)] = pstIsolatedMg[i];
	  isolatedEg[side][REL_SQ(i,side)] = pstIsolatedEg[i];
	  backwardMg[side][REL_SQ(i,side)] = pstBackwardMg[i];
	  backwardEg[side][REL_SQ(i,side)] = pstBackwardEg[i];

	  outpost[side][N][REL_SQ(i,side)] = pstKnightOutpost[i];
	  outpost[side][B][REL_SQ(i,side)] = pstBishopOutpost[i];
	  outpost[side][R][REL_SQ(i,side)]   = pstRookOutpost[i];
	  badBishopPenalty[side] [REL_SQ(i,side)] = pstBadBishop[i];
    }
  }
}