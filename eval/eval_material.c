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

#include "../rodent.h"
#include "../data.h"
#include "../bitboard/bitboard.h"
#include "eval.h"

	static const int BN_wb[64] = {
		0,    0,  15,  30,  45,  60,  85, 100,
		0,   15,  30,  45,  60,  85, 100,  85,
		15,  30,  45,  60,  85, 100,  85,  60,
		30,  45,  60,  85, 100,  85,  60,  45,
		45,  60,  85, 100,  85,  60,  45,  30,
		60,  85, 100,  85,  60,  45,  30,  15,
		85, 100,  85,  60,  45,  30,  15,   0,
	   100,  85,  60,  45,  30,  15,   0,   0
	};

	static const int BN_bb[64] = {
		100, 85,  60,  45,  30,  15,   0,   0,
		85, 100,  85,  60,  45,  30,  15,   0,
		60,  85, 100,  85,  60,  45,  30,  15,
		45,  60,  85, 100,  85,  60,  45,  30,
		30,  45,  60,  85, 100,  85,  60,  45,
		15,  30,  45,  60,  85, 100,  85,  60,
		0,   15,  30,  45,  60,  85, 100,  85,
		0,   0,   15,  30,  45,  60,  85, 100
	};

// TODO: move to data struct
static const int pawnMat[9] = {-40, 90, 180, 270, 360, 450, 540, 630, 720};

//  Crafty-like material imbalance table indexed by difference         
//  in major piece material (R) and minor piece material (n) :          

#define A  80 // advantage in both major and minor pieces
#define Rk 50 // advantage in major pieces only
#define Nt 40 // advantage in minor pieces only
#define Ex 10 // exchange disadvantage, tuned within 5 cp
#define Mm 60 // two minors for a rook 

static const int imbalance[9][9] = {
/* n=-4  n=-3  n=-2  n=-1  n=0   n=+1  n=+2  n=+3  n=+4 */
  {  -A,   -A,   -A,   -A,  -Rk,    0,    0,    0,    0 }, // R = -4 
  {  -A,   -A,   -A,   -A,  -Rk,    0,    0,    0,    0 }, // R = -3 
  {  -A,   -A,   -A,   -A,  -Rk,    0,    0,    0,    0 }, // R = -2 
  {  -A,   -A,   -A,   -A,  -Rk,  -Ex,   Mm,    0,    0 }, // R = -1 
  { -Nt,   -Nt, -Nt,  -Nt,    0,   Nt,   Nt,   Nt,   Nt }, // R =  0 
  {   0,    0,  -Mm,   Ex,   Rk,    A,    A,    A,    A }, // R = +1 
  {   0,    0,    0,    0,   Rk,    A,    A,    A,    A }, // R = +2 
  {   0,    0,    0,    0,   Rk,    A,    A,    A,    A }, // R = +3 
  {   0,    0,    0,    0,   Rk,    A,    A,    A,    A }  // R = +4 
};

#define NP 6 // values as close as possible to LK formula (1/16 and 1/32 of a pawn)
#define RP 3

static const int N_adj[9] = { -4*NP, -3*NP, -2*NP, -NP,  0,  NP,  2*NP,  3*NP,  4*NP };
static const int R_adj[9] = {  4*RP,  3*RP,  2*RP,  RP,  0, -RP, -2*RP, -3*RP, -4*RP };

int sEvaluator::GetMaterialScore(sPosition *p) 
{
  // piece material
  int score = p->pieceMat[WHITE] - p->pieceMat[BLACK];

  // pawn material
  score += pawnMat[p->pcCount[WHITE][P]];
  score -= pawnMat[p->pcCount[BLACK][P]];

  // bishop pair
  if ( p->pcCount[WHITE][B] > 1) score += Data.bishopPair;
  if ( p->pcCount[BLACK][B] > 1) score -= Data.bishopPair;

  // rook/knight adjustment based on no. of pawns
  score += ( N_adj[ p->pcCount[WHITE][P] ] * p->pcCount[WHITE] [N] );
  score -= ( N_adj[ p->pcCount[BLACK][P] ] * p->pcCount[BLACK] [N] );
  score += ( R_adj[ p->pcCount[WHITE][P] ] * p->pcCount[WHITE] [R] );
  score -= ( R_adj[ p->pcCount[BLACK][P] ] * p->pcCount[BLACK] [R] );

  // material imbalance table
  int minorBalance = p->pcCount[WHITE][N] - p->pcCount[BLACK][N] + p->pcCount[WHITE][B]   - p->pcCount[BLACK][B];
  int majorBalance = p->pcCount[WHITE][R] - p->pcCount[BLACK][R] + 2*p->pcCount[WHITE][Q] - 2*p->pcCount[BLACK][Q];

  int x = Max( majorBalance + 4, 0 );
  if (x > 8) x = 8;

  int y = Max(minorBalance + 4, 0);
  if (y > 8) y = 8;

  score += imbalance [x] [y];

  return score;
}

int sEvaluator::CheckmateHelper(sPosition *p) 
{
  int result = 0;

  if (p->pcCount[WHITE][P] == 0
  &&  p->pcCount[BLACK][P] == 0) {

	  // incentive for the final simplification (so that KRR vs KR is won)
      if ( p->pieceMat[WHITE] == 0 ) result -= 50;
      if ( p->pieceMat[BLACK] == 0 ) result += 50;

	  if ( (p->pieceMat[WHITE] - p->pieceMat[BLACK]) > 100
	  && p->pieceMat[BLACK] < 600)
	  {
      // drive enemy king towards the edge
	  result += (40 - Data.pstEg[BLACK][K][p->kingSquare[BLACK]] + Data.distance[p->kingSquare[WHITE]] [p->kingSquare[BLACK]]);

	  if (MaterialBN(p, WHITE) ) { // mate with bishop and knight
		  if ( bbPc(p, WHITE, B) & bbWhiteSq) result -= 2*BN_bb[p->kingSquare[BLACK]];
		  if ( bbPc(p, WHITE, B) & bbBlackSq) result -= 2*BN_wb[p->kingSquare[BLACK]];
	    }
	  }

	  if ((p->pieceMat[WHITE] - p->pieceMat[BLACK]) < -100
	  && p->pieceMat[WHITE] < 600)
	  {
      // drive enemy king towards the edge
	  result -= (40 - Data.pstEg[WHITE][K][p->kingSquare[WHITE]] + Data.distance[p->kingSquare[WHITE]] [p->kingSquare[BLACK]]);

      if (MaterialBN(p, BLACK) ) { // mate with bishop and knight
			if ( bbPc(p, BLACK, B) & bbWhiteSq) result += 2*BN_bb[p->kingSquare[WHITE]];
			if ( bbPc(p, BLACK, B) & bbBlackSq) result += 2*BN_wb[p->kingSquare[WHITE]];
	    }
	  }
  }

  return result;
}