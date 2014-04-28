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

#include "../rodent.h"
#include "../bitboard/bitboard.h"
#include "search.h"

int KPKdraw(sPosition *p, int stronger);

int sSearcher::RecognizeDraw(sPosition *p) 
{
  if ( p->pcCount[WHITE][P] == 0 && p->pcCount[BLACK][P] == 0 ) {		  

	  if ( p->pieceMat[WHITE] + p->pieceMat[BLACK] < 400 ) {
	     if ( !IllegalPosition(p) ) return 1;
	  }  // bare kings or Km vs K 
	   
	  if (p->pieceMat[WHITE] < 400 
	  &&  p->pieceMat[BLACK] < 400
	  &&  (bbPc(p, WHITE, K) & bbRim) == 0
	  &&  (bbPc(p, BLACK, K) & bbRim) == 0 ) {
		  if ( !IllegalPosition(p) ) return 1;
	  }   // Km vs Km; just in case we ensure that neither king is on the rim 
  } // no pawns

  if (p->pieceMat[WHITE] == 0 && p->pieceMat[BLACK] == 0) {

	  if (p->pcCount[WHITE][P] == 1 && p->pcCount[BLACK][P] == 0) 
	     return KPKdraw(p, WHITE); // exactly one white pawn

	  if (p->pcCount[BLACK][P] == 1 && p->pcCount[WHITE][P] == 0)
         return KPKdraw(p, BLACK); // exactly one black pawn
  } // pawns only

  return 0;
}

int KPKdraw(sPosition *p, int stronger)
{
    int weaker = Opp(stronger);
    U64 bbPawn = bbPc(p, stronger, P);
		  
    // opposition through a pawn
	if ( p->side == stronger
	&& (SqBb(p->kingSquare[weaker]) & ShiftFwd(bbPawn, stronger) )
	&& (SqBb(p->kingSquare[stronger]) & ShiftFwd(bbPawn, weaker) )
	) return 1;

    // weaker side can create opposition through a pawn in one move
	if ( p->side == weaker
	&& (bbKingAttacks[p->kingSquare[weaker]] & ShiftFwd(bbPawn, stronger) )
	&& (SqBb(p->kingSquare[stronger]) & ShiftFwd(bbPawn, weaker) )
	) if ( !IllegalPosition(p) ) return 1;

	// TODO: opposition next to a pawn

	return 0;
} 
