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

// Array of "equivalent squares", used to  initialize
// pst tables and to make certain parts of evaluation 
// color-independent.

const int relativeSq[2][64] = {
 {A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8},

{ A8, B8, C8, D8, E8, F8, G8, H8,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A1, B1, C1, D1, E1, F1, G1, H1}
};

#define PA   -15 // -15 best so far, -12 is worse
#define PB    -5
#define PC     5
#define PD    10 // 15 is worse
#define PE    10 // endgame material gain of a pawn

/**
const int pstPawnMg[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
    PA,  PB,   0,  -5,  -5,   0,  PB,  PA,
    PA,  PB,  PC,  PD,  PD,  PC,  PB,  PA,
    PA,  PB,  PC,  PD,  PD,  PC,  PB,  PA,
    PA,  PB,  PC,  PD,  PD,  PC,  PB,  PA,
    PA,  PB,  PC,  PD,  PD,  PC,  PB,  PA,
    PA,  PB,  PC,  PD,  PD,  PC,  PB,  PA,
     0,   0,   0,   0,   0,   0,   0,   0
};
/**/

/**/
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
/**/

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
	15,  25,  15, -15,  0,  -15,  25,  15,
   -10, -10, -10, -15, -15, -10, -10, -10,
   -20, -20, -20, -20, -20, -20, -20, -20,
   -30, -30, -30, -30, -30, -30, -30, -30,
   -40, -40, -40, -40, -40, -40, -40, -40,
   -40, -40, -40, -40, -40, -40, -40, -40,
   -40, -40, -40, -40, -40, -40, -40, -40,
   -40, -40, -40, -40, -40, -40, -40, -40
};

const int pstKingEg[64] = 
{ 
   -10,  10,  20,  30,  30,  20,  10, -10,
    10,  20,  30,  40,  40,  30,  20,  10,
    20,  30,  40,  50,  50,  40,  30,  20,
    30,  40,  50,  60,  60,  50,  40,  30,
    30,  40,  50,  60,  60,  50,  40,  30,
    20,  30,  40,  50,  50,  40,  30,  20,
    10,  20,  30,  40,  40,  30,  20,  10,
   -10,  10,  20,  30,  30,  20,  10, -10
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

void sData::InitPstValues(void) {

  for (int i = 0; i < 64; i++) {  
	for (int side = 0; side < 2; side++) {

      pstMg[side][P][relativeSq[side][i]] = pstPawnMg[i];
      pstMg[side][N][relativeSq[side][i]] = pstKnightMg[i];
      pstMg[side][B][relativeSq[side][i]] = pstBishopMg[i];
	  pstMg[side][R][relativeSq[side][i]] = pstRookMg[i];
      pstMg[side][Q][relativeSq[side][i]] = pstQueenMg[i];
      pstMg[side][K][relativeSq[side][i]] = pstKingMg[i];

      pstEg[side][P][relativeSq[side][i]] = pstPawnEg[i];
      pstEg[side][N][relativeSq[side][i]] = pstKnightEg[i];
      pstEg[side][B][relativeSq[side][i]] = pstBishopEg[i];
      pstEg[side][R][relativeSq[side][i]] = pstRookEg[i];
      pstEg[side][Q][relativeSq[side][i]] = pstQueenEg[i];
      pstEg[side][K][relativeSq[side][i]] = pstKingEg[i];

	  phalanxMg[side][relativeSq[side][i]]  = pstPhalanxMg[i];
	  passersMg[side][relativeSq[side][i]]  = pstPasserMg[i];
	  passersEg[side][relativeSq[side][i]]  = pstPasserEg[i];
	  candidateMg[side][relativeSq[side][i]]  = pstPasserMg[i] / 3;
	  candidateEg[side][relativeSq[side][i]]  = pstPasserEg[i] / 3;
      isolatedMg[side][relativeSq[side][i]] = pstIsolatedMg[i];
	  isolatedEg[side][relativeSq[side][i]] = pstIsolatedEg[i];
	  backwardMg[side][relativeSq[side][i]] = pstBackwardMg[i];
	  backwardEg[side][relativeSq[side][i]] = pstBackwardEg[i];

	  outpost[side][N][relativeSq[side][i]] = pstKnightOutpost[i];
	  outpost[side][B][relativeSq[side][i]] = pstBishopOutpost[i];
	  outpost[side][R][relativeSq[side][i]]   = pstRookOutpost[i];
	  badBishopPenalty[side] [relativeSq[side][i]] = pstBadBishop[i];
    }
  }
}