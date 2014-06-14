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

#include "../bitboard/bitboard.h"
#include "../data.h"
#include "../rodent.h"
#include "eval.h"

const int centDefense = 5;
const int doubledPawn [2] [8]= { {-25, -25, -25, -25, -25, -25, -25, -25 }, {-15, -17, -19, -19, -19, -19, -17, -15 } };
const int pawnIsolatedOnOpen = -15;
const int pawnBackwardOnOpen = -15;

void sEvaluator::EvalPawns(sPosition *p)
{
   int pawnHash = p->pawnKey % PAWN_HASH_SIZE;

   // try reading score from pawn hashtable
   if ( PawnTT[pawnHash].pawnKey == p->pawnKey) {
      mgScore += PawnTT[pawnHash].mgPawns;
      egScore += PawnTT[pawnHash].egPawns;
      mgScore += PawnTT[pawnHash].mgPassers;
      egScore += PawnTT[pawnHash].egPassers;
   }
   else
   {
      SinglePawnScore(p, WHITE);
      SinglePawnScore(p, BLACK);
      EvalPawnCenter(p, WHITE);
      EvalPawnCenter(p, BLACK);
 
      mgScore += ( ( (pawnScoreMg[WHITE] - pawnScoreMg[BLACK]) * Data.pawnStruct ) / 100 );
      egScore += ( ( (pawnScoreEg[WHITE] - pawnScoreEg[BLACK]) * Data.pawnStruct ) / 100 );
      mgScore += ( ( (passerScoreMg[WHITE] - passerScoreMg[BLACK]) * Data.passedPawns ) / 100 );
      egScore += ( ( (passerScoreEg[WHITE] - passerScoreEg[BLACK]) * Data.passedPawns ) / 100 );

      // save score to pawn hashtable
      PawnTT[pawnHash].pawnKey = p->pawnKey;
      PawnTT[pawnHash].mgPawns   = ( ( (pawnScoreMg[WHITE] - pawnScoreMg[BLACK])* Data.pawnStruct ) / 100 );
      PawnTT[pawnHash].egPawns   = ( ( (pawnScoreEg[WHITE] - pawnScoreEg[BLACK])* Data.pawnStruct ) / 100 );
      PawnTT[pawnHash].mgPassers = ( ( (passerScoreMg[WHITE] - passerScoreMg[BLACK])* Data.passedPawns ) / 100 );
      PawnTT[pawnHash].egPassers = ( ( (passerScoreEg[WHITE] - passerScoreEg[BLACK])* Data.passedPawns ) / 100 );
   }
}

void sEvaluator::EvalPawnCenter(sPosition *p, int side)
{
   int oppo = Opp(side);	  
   if (bbPc(p, side, P) & RelSqBb(D4,side) ) {
      // defend central pawns (pawns side by side evaluated as "phalanx")
      if (bbPc(p, side, P) & RelSqBb(E3,side) )  pawnScoreMg[side] += centDefense;
      if (bbPc(p, side, P) & RelSqBb(C3,side) )  pawnScoreMg[side] += centDefense;
   }

   if (bbPc(p, side, P) & RelSqBb(E4,side) ) {
      if ( bbPc(p, side, P) & RelSqBb(D3,side) ) pawnScoreMg[side] += centDefense;
   }
}

void sEvaluator::SinglePawnScore(sPosition *p, int side)
{
  int sq, flagIsOpen, flagIsWeak;
  int oppo = Opp(side);
  U64 flagIsPhalanx, flagPhalanx2, flagIsDoubled;
  U64 bbPieces = bbPc(p, side, P);
  U64 bbOwnPawns = bbPieces;
  U64 bbFrontSpan;
 
  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);

    // gather information about a pawn that is evaluated
	bbFrontSpan   = GetFrontSpan(SqBb(sq), side );
	flagIsDoubled = bbFrontSpan & bbOwnPawns;
    flagIsOpen    = !( bbFrontSpan & bbPc(p, oppo, P) );
	flagIsPhalanx = ShiftEast(SqBb(sq) ) & bbOwnPawns;
	flagPhalanx2  = ShiftWest(SqBb(sq) ) & bbOwnPawns;
	flagIsWeak    = !( bbPawnSupport[side][sq] & bbOwnPawns );
	
	if (flagIsDoubled) {
	    pawnScoreMg[side] += doubledPawn[MG][File(sq)];
	    pawnScoreEg[side] += doubledPawn[EG][File(sq)];
    }

	if (flagIsPhalanx) {
		pawnScoreMg[side] += Data.pawnProperty[PHALANX][MG][side][sq];
		//pawnScoreEg[side] += Data.phalanxEg[side][sq];
	}

	if (flagIsOpen) {
		U64 bbObstacles = bbPassedMask[side][sq] & bbPc(p, oppo, P);
		
	    // passed pawn (some more eval will be done in ScoreP() in eval_pieces.c)
		if (!bbObstacles) {
			if (flagIsDoubled) AddPasserScore(CANDIDATE,side,sq); // back doubled passer is scored like candidate,
			else               AddPasserScore(PASSED,side,sq);    // only frontmost passer gets full credit
		}
		
	    // candidate passer
		if (flagIsPhalanx || flagPhalanx2) { // test lower bonus when !flagIsWeak
		    if (PopCntSparse(bbObstacles) == 1) AddPasserScore(CANDIDATE,side,sq);
	    }
	}
    
	if (flagIsWeak) {
		if (!(bbAdjacentMask[File(sq)] & bbOwnPawns)) // isolated
		{ 
		   AddPawnProperty(ISOLATED,side,sq);
		   if (flagIsOpen) pawnScoreMg[side] += pawnIsolatedOnOpen;
		}
		else // backward
		{
		   AddPawnProperty(BACKWARD,side,sq);
		   if (flagIsOpen) pawnScoreMg[side] += pawnBackwardOnOpen;
		}
	}
  }
}

void sEvaluator::AddPawnProperty(int pawnProperty, int side, int sq)
{
     pawnScoreMg[side] += Data.pawnProperty[pawnProperty][MG][side][sq]; 
	 pawnScoreEg[side] += Data.pawnProperty[pawnProperty][EG][side][sq];
}

void sEvaluator::AddPasserScore(int pawnProperty, int side, int sq)
{
     passerScoreMg[side] += Data.pawnProperty[pawnProperty][MG][side][sq]; 
	 passerScoreEg[side] += Data.pawnProperty[pawnProperty][EG][side][sq];
}
