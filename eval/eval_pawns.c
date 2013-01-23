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
  if ( PawnTT[pawnHash].pawnKey == p->pawnKey ) 
  {
	  mgScore += PawnTT[pawnHash].mgPawns;
	  egScore += PawnTT[pawnHash].egPawns;
  }
  else
  {
      PawnScore(p, WHITE); // pawn eval
      PawnScore(p, BLACK);  

      if (bbPc(p, WHITE, P) & SqBb(D4)) 
      {
          if (bbPc(p, WHITE, P) & SqBb(E3))  pawnScoreMg[WHITE] += CENT_DEFENSE;
          if (bbPc(p, WHITE, P) & SqBb(C3))  pawnScoreMg[WHITE] += CENT_DEFENSE;
      }

      if (bbPc(p, WHITE, P) & SqBb(E4) && bbPc(p, WHITE, P) & SqBb(D3) ) 
         pawnScoreMg[WHITE] += CENT_DEFENSE;

      if (bbPc(p, BLACK, P) & SqBb(D5)) 
      {
         if (bbPc(p, BLACK, P) & SqBb(E6))  pawnScoreMg[BLACK] += CENT_DEFENSE;
         if (bbPc(p, BLACK, P) & SqBb(C6))  pawnScoreMg[BLACK] += CENT_DEFENSE;
      }

      if (bbPc(p, BLACK, P) & SqBb(E5) && bbPc(p, BLACK, P) & SqBb(D6) ) 
         pawnScoreMg[BLACK] += CENT_DEFENSE;
 
      mgScore += (pawnScoreMg[WHITE] - pawnScoreMg[BLACK]);
      egScore += (pawnScoreEg[WHITE] - pawnScoreEg[BLACK]);

      // save score to pawn hashtable
      PawnTT[pawnHash].pawnKey = p->pawnKey;
      PawnTT[pawnHash].mgPawns = (pawnScoreMg[WHITE] - pawnScoreMg[BLACK]);
      PawnTT[pawnHash].egPawns = (pawnScoreEg[WHITE] - pawnScoreEg[BLACK]);
  }
}

void sEvaluator::PawnScore(sPosition *p, int side)
{
  int sq; 
  int flagIsOpen, flagIsWeak;
  U64 flagIsPhalanx;
  U64 bbPieces = bbPc(p, side, P);
  U64 bbFrontSpan;
 
  while (bbPieces) 
  {
    sq = FirstOne(bbPieces);

    // gather information about a pawn
	bbFrontSpan    = GetFrontSpan(SqBb(sq), side );
    flagIsOpen     = ( ( bbFrontSpan & bbPc(p, Opp(side), P) ) == 0 );
	flagIsPhalanx  = ShiftEast(SqBb(sq) ) & bbPc(p, side, P);
	flagIsWeak     = ( ( bbPawnSupport[side][sq] & bbPc(p, side, P) ) == 0);

	if (flagIsPhalanx) 
	{
		pawnScoreMg[side] += Data.phalanxMg[side][sq];
		//pawnScoreEg[side] += Data.phalanxEg[side][sq];
	}
	
	// doubled pawn
	if ( bbFrontSpan & bbPc(p, side, P) ) 
	{
	  pawnScoreMg[side] += Data.doubledPawnMg;
	  pawnScoreEg[side] += Data.doubledPawnEg;
    }

	if (flagIsOpen) 
	{
		U64 bbObstacles = bbPassedMask[side][sq] & bbPc(p, Opp(side), P);
		
	    if (!bbObstacles)  // passed pawn
	    {
	       pawnScoreMg[side] += Data.passersMg[side][sq];	  
           pawnScoreEg[side] += Data.passersEg[side][sq];	
	    }

	    if (flagIsPhalanx) // can be a candidate passer
	    {
		    if (PopCntSparse(bbObstacles) == 1) 
			{
           	    pawnScoreMg[side] += Data.candidateMg[side][sq];
                pawnScoreEg[side] += Data.candidateEg[side][sq];
		    }
	    }
	}
    
	if (flagIsWeak)        // weak pawn
	{
		if (!(bbAdjacentMask[File(sq)] & bbPc(p, side, P))) // isolated
		{ 
		   pawnScoreMg[side] += Data.isolatedMg[side][sq];
		   if (flagIsOpen) 
			   pawnScoreMg[side] += Data.pawnIsolatedOnOpen;
		   pawnScoreEg[side] += Data.isolatedEg[side][sq];
		}
		else // backward
		{
		   pawnScoreMg[side] += Data.backwardMg[side][sq];
		   if (flagIsOpen) 
			   pawnScoreMg[side] += Data.pawnBackwardOnOpen;
		   pawnScoreEg[side] += Data.backwardEg[side][sq];
		}
	}
	
	bbPieces &= bbPieces - 1;
  }
}