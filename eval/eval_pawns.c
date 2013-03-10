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

#include "../bitboard/bitboard.h"
#include "../data.h"
#include "../rodent.h"
#include "eval.h"

#define CENT_DEFENSE 5

void sEvaluator::EvalPawns(sPosition *p)
{
  int pawnHash = p->pawnKey % PAWN_HASH_SIZE;

  // try reading score from pawn hashtable
  if ( PawnTT[pawnHash].pawnKey == p->pawnKey ) {
	  mgScore += PawnTT[pawnHash].mgPawns;
	  egScore += PawnTT[pawnHash].egPawns;
  }
  else
  {
      SinglePawnScore(p, WHITE);
      SinglePawnScore(p, BLACK);
	  EvalPawnCenter(p, WHITE);
	  EvalPawnCenter(p, BLACK);
 
      mgScore += (pawnScoreMg[WHITE] - pawnScoreMg[BLACK]);
      egScore += (pawnScoreEg[WHITE] - pawnScoreEg[BLACK]);

      // save score to pawn hashtable
      PawnTT[pawnHash].pawnKey = p->pawnKey;
      PawnTT[pawnHash].mgPawns = (pawnScoreMg[WHITE] - pawnScoreMg[BLACK]);
      PawnTT[pawnHash].egPawns = (pawnScoreEg[WHITE] - pawnScoreEg[BLACK]);
  }
}

void sEvaluator::EvalPawnCenter(sPosition *p, int side)
{
      if (bbPc(p, side, P) & RelSqBb(D4,side) ) {
          if (bbPc(p, side, P) & RelSqBb(E3,side) )  pawnScoreMg[side] += CENT_DEFENSE;
          if (bbPc(p, side, P) & RelSqBb(C3,side) )  pawnScoreMg[side] += CENT_DEFENSE;
      }

      if (bbPc(p, side, P) & RelSqBb(E4,side) && bbPc(p, side, P) & RelSqBb(D3,side) ) 
         pawnScoreMg[side] += CENT_DEFENSE;	
}

void sEvaluator::SinglePawnScore(sPosition *p, int side)
{
  int sq; 
  int flagIsOpen, flagIsWeak;
  U64 flagIsPhalanx;
  U64 bbPieces = bbPc(p, side, P);
  U64 bbOwnPawns = bbPieces;
  U64 bbFrontSpan;
 
  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);

    // gather information about a pawn that is evaluated
	bbFrontSpan    = GetFrontSpan(SqBb(sq), side );
    flagIsOpen     = ( ( bbFrontSpan & bbPc(p, Opp(side), P) ) == 0 );
	flagIsPhalanx  = ShiftEast(SqBb(sq) ) & bbOwnPawns;
	flagIsWeak     = ( ( bbPawnSupport[side][sq] & bbOwnPawns ) == 0);
	
	// doubled pawn
	if ( bbFrontSpan & bbOwnPawns ) {
	    pawnScoreMg[side] += Data.doubledPawn[MG];
	    pawnScoreEg[side] += Data.doubledPawn[EG];
    }

	if (flagIsPhalanx) {
		pawnScoreMg[side] += Data.pawnProperty[PHALANX][MG][side][sq];
		//pawnScoreEg[side] += Data.phalanxEg[side][sq];
	}

	if (flagIsOpen) {
		U64 bbObstacles = bbPassedMask[side][sq] & bbPc(p, Opp(side), P);
		
	    // passed pawn
		if (!bbObstacles) AddPawnProperty(PASSED,side,sq);

	    // candidate passer
		if (flagIsPhalanx) {
		    if (PopCntSparse(bbObstacles) == 1) AddPawnProperty(CANDIDATE,side,sq);
	    }
	}
    
	if (flagIsWeak) {
		if (!(bbAdjacentMask[File(sq)] & bbOwnPawns)) // isolated
		{ 
		   AddPawnProperty(ISOLATED,side,sq);
		   if (flagIsOpen) pawnScoreMg[side] += Data.pawnIsolatedOnOpen;
		}
		else // backward
		{
		   AddPawnProperty(BACKWARD,side,sq);
		   if (flagIsOpen) pawnScoreMg[side] += Data.pawnBackwardOnOpen;
		}
	}
  }
}

void sEvaluator::AddPawnProperty(int pawnProperty, int side, int sq)
{
     pawnScoreMg[side] += Data.pawnProperty[pawnProperty][MG][side][sq]; 
	 pawnScoreEg[side] += Data.pawnProperty[pawnProperty][EG][side][sq];
}