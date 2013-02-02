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

// Functions responsible for scaling down the evaluation score
// in case of drawish endgames.

int sEvaluator::SetDegradationFactor(sPosition *p, int stronger)
{
	int weaker = Opp(stronger);

	if ( p->pieceMat[stronger] > 1400
	||   p->pcCount[stronger][P] > 2 ) return 64;

	if ( p->pieceMat[weaker] > 1400
	||   p->pcCount[weaker][P] > 2 ) return 64;


	// no pawns for stronger side
	if (p->pcCount[stronger][P] == 0) {
       // no win if stronger side has just one minor piece
       if ( p->pieceMat[stronger] < 400 ) return 0;
	}

    // no pawns of either color on the board
    if (p->pcCount[stronger][P] == 0 
    &&  p->pcCount[weaker  ][P] == 0) 
	{
       if (MaterialNN(p,stronger) ) return 0;

	  // low and almost equal material, except KBB vs KN:     1/8
	  if ( MaterialRook(p, stronger)  && MaterialRook(p, weaker) ) return 8;
	  if ( MaterialQueen(p, stronger) && MaterialQueen(p, weaker) ) return 8;
	  if ( MaterialBN(p, stronger) && ( MaterialMinor (p, weaker) || MaterialRook(p, weaker) ) ) return 8;
	  if ( MaterialBB(p, stronger) && ( MaterialBishop(p, weaker) || MaterialRook(p, weaker) ) ) return 8;
	}

    // it's hard to win with a bare rook against a minor/minor + pawn
    if (p->pcCount[stronger][P] == 0 
    && p->pieceMat[stronger] == Data.matValue[R] 
    && p->pieceMat[weaker] > 0 ) return 32; // 1/2

    // it's hard to win with a rook and a minor against a rook/rook + pawns
    if (p->pcCount[stronger][P] == 0 
    && MaterialRookMinor(p, stronger) 
    && MaterialRook(p, weaker) ) return 32; // 1/2

    // it's hard to win with a queen and a minor against a queen/queen + pawns
    if (p->pcCount[stronger][P] == 0 
    && MaterialQueenMinor(p, stronger) 
    && MaterialQueen(p, weaker) ) return 32; // 1/2

    // KBP vs Km is drawn when defending king stands on pawn's path 
    // and cannot be driven out by a Bishop
    if (MaterialBishop(p, stronger)
    &&  MaterialMinor(p, weaker)
    &&  p->pcCount[stronger][P] == 1
    &&  p->pcCount[weaker][P] == 0
    &&  ( SqBb(p->kingSquare[weaker]) & GetFrontSpan( bbPc(p, stronger, P), stronger ) )
    &&  NotOnBishColor(p, stronger, p->kingSquare[weaker])
    ) return 0;

    // decrease score in pure opposite bishops ending
    if (MaterialBishop(p, stronger) 
    && MaterialBishop(p, weaker)
    && BishopsAreDifferent(p) ) return 32; // 1/2

    // some special rules for rook endgame with one pawn
    if (MaterialRook(p, stronger)
    &&  MaterialRook(p, weaker)
    &&  p->pcCount[stronger][P] == 1
    &&  p->pcCount[weaker][P] == 0
  ) {
      // good defensive position with a king on pawn's path
	  if ( ( SqBb(p->kingSquare[weaker]) & GetFrontSpan( bbPc(p, stronger, P), stronger ) ) ) 
	      return 32; // 1/2

	  // draw code for rook endgame with edge pawn
	  if ( ( SqBb(REL_SQ(A7,stronger) ) & bbPc(p, stronger, P) )
	  &&   ( SqBb(REL_SQ(A8,stronger) ) & bbPc(p, stronger, R) )
	  &&   ( bbFILE_A & bbPc(p, weaker, R) )
	  &&   ( ( SqBb(REL_SQ(H7,stronger)) & bbPc(p, weaker, K) ) || ( SqBb(REL_SQ(G7,stronger) ) & bbPc(p, weaker, K) ) )
		  ) return 0; // dead draw

	  if ( ( SqBb(REL_SQ(H7,stronger)) & bbPc(p, stronger, P) )
	  &&   ( SqBb(REL_SQ(H8,stronger)) & bbPc(p, stronger, R) )
	  &&   ( bbFILE_H & bbPc(p, weaker, R) )
	  &&   ( ( SqBb(REL_SQ(A7,stronger)) & bbPc(p, weaker, K) ) || ( SqBb(REL_SQ(B7,stronger)) & bbPc(p, weaker, K) ) )
		  ) return 0; // dead draw

	  // TODO: back rank defense
	}

    // TODO: KRPPKRP, no passers
    // TODO: KPK draws with edge pawn
    // TODO: KBPK draws with edge pawn and wrong bishop
    // TODO: KBPKP with blockade and wrong bishop

	return 64; // no degradation
}

int BishopsAreDifferent(sPosition * p) {
	if ( ( bbWhiteSq & bbPc(p, WHITE, B) ) && ( bbBlackSq & bbPc(p, BLACK, B) ) ) return 1;
	if ( ( bbBlackSq & bbPc(p, WHITE, B) ) && ( bbWhiteSq & bbPc(p, BLACK, B) ) ) return 1;
    return 0;
}

int NotOnBishColor(sPosition * p, int bishSide, int sq) 
{
    if ( ( ( bbWhiteSq & bbPc(p, bishSide, B) ) == 0 )
    && ( SqBb(sq) & bbWhiteSq) ) return 1;

    if ( ( ( bbBlackSq & bbPc(p, bishSide, B) ) == 0 )
    && ( SqBb(sq) & bbBlackSq) ) return 1;

    return 0;
}

int MaterialKnight(sPosition *p, int side) {
	return ( p->pieceMat[side] == Data.matValue[N] );
}

int MaterialBishop(sPosition *p, int side) {
	return ( p->pieceMat[side] == Data.matValue[B] );
}

int MaterialMinor(sPosition *p, int side) {
   return (    (p->pieceMat[side] ==  Data.matValue[B]) 
	        || (p->pieceMat[side] ==  Data.matValue[N]) );
}

int MaterialRook(sPosition *p, int side) {
	return ( p->pieceMat[side] == Data.matValue[R] );
}

int MaterialRookMinor(sPosition *p, int side) {
    return (    (p->pieceMat[side] == Data.matValue[R] + Data.matValue[B]) 
	         || (p->pieceMat[side] == Data.matValue[R] + Data.matValue[N]) );
}

int MaterialQueen(sPosition *p, int side) {
	return ( p->pieceMat[side] == Data.matValue[Q] );
}

int MaterialQueenMinor(sPosition *p, int side) {
    return (    (p->pieceMat[side] == Data.matValue[Q] + Data.matValue[B]) 
	         || (p->pieceMat[side] == Data.matValue[Q] + Data.matValue[N]) );
}

int MaterialNN(sPosition *p, int side) {
    return ( p->pieceMat[side] == 2*Data.matValue[N] );
}

int MaterialBB(sPosition *p, int side) {
    return ( p->pieceMat[side] == 2*Data.matValue[B] );
}

int MaterialBN(sPosition *p, int side) {
	return ( p->pieceMat[side] == Data.matValue[B] + Data.matValue[N] );
}