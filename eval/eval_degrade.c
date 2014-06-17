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
#include "../data.h"
#include "../bitboard/bitboard.h"
#include "eval.h"

int sEvaluator::PullToDraw(sPosition *p, int score)
{
  int degradation = 64; 
  if (score > 0) degradation = SetDegradationFactor(p, WHITE);
  if (score < 0) degradation = SetDegradationFactor(p, BLACK);

  score *= degradation;
  return score / 64;
}

// Functions responsible for scaling down the evaluation score
// in case of drawish endgames.

int sEvaluator::SetDegradationFactor(sPosition *p, int stronger)
{
	int weaker = Opp(stronger);

	if ( p->pieceMat[stronger] > 1400
	||   p->pieceMat[weaker] > 1400 ) return 64;

    // decrease score in pure opposite bishops ending
    if (PcMatBishop(p, stronger) 
    && PcMatBishop(p, weaker)
    && BishopsAreDifferent(p) ) return 32; // 1/2
	
	if (p->pcCount[stronger][P] > 2 
	||  p->pcCount[weaker][P] > 2 ) return 64;

	const U64 bbKingBlockH[2] = {SqBb(H8) | SqBb(H7) | SqBb(G8) | SqBb(G7) ,
	                             SqBb(H1) | SqBb(H2) | SqBb(G1) | SqBb(G2)};
	const U64 bbKingBlockA[2] = {SqBb(A8) | SqBb(A7) | SqBb(B8) | SqBb(B7) ,
	                             SqBb(A1) | SqBb(A2) | SqBb(B1) | SqBb(B2)};

	if ( p->pieceMat[stronger] < Data.matValue[R] ) {

	   // KPK with edge pawn (else KBPK recognizer would break)
	   if (p->pieceMat[stronger] == 0
	   &&  p->pieceMat[weaker]   == 0
	   &&  p->pcCount[stronger][P]  == 1  // TODO: all pawns of a stronger side on a rim
	   &&  p->pcCount[weaker][P]  == 0) { // TODO: accept pawns for a weaker side

	      if (bbPc(p, stronger, P) & bbFILE_H
          && bbPc(p, weaker, K)  & bbKingBlockH[stronger]) return 0;

	      if (bbPc(p, stronger, P) & bbFILE_A
          && bbPc(p, weaker, K)  & bbKingBlockA[stronger]) return 0;
	}

	   // KBPK(P) draws with edge pawn and wrong bishop
	   if (p->pieceMat[stronger] == Data.matValue[B]
	   &&  p->pieceMat[weaker]   == 0        // TODO: accept pawns for a weaker side
	   &&  p->pcCount[stronger][P]  == 1 ) { // TODO: all pawns of a stronger side on a rim

	      if (bbPc(p, stronger, P) & bbFILE_H
          && NotOnBishColor(p, stronger, REL_SQ(H8,stronger))
          && bbPc(p, weaker, K)  & bbKingBlockH[stronger]) return 0;

	      if (bbPc(p, stronger, P) & bbFILE_A
          && NotOnBishColor(p, stronger, REL_SQ(A8,stronger))
          && bbPc(p, weaker, K)  & bbKingBlockA[stronger]) return 0;
	   }

	   // KBP vs Km is drawn when defending king stands on pawn's path 
       // and cannot be driven out by a Bishop
       if (PcMatBishop(p, stronger)
       &&  PcMatMinor(p, weaker)
       &&  p->pcCount[stronger][P] == 1
       &&  p->pcCount[weaker][P] == 0
       &&  ( SqBb(p->kingSquare[weaker]) & GetFrontSpan( bbPc(p, stronger, P), stronger ) )
       &&  NotOnBishColor(p, stronger, p->kingSquare[weaker])
       ) return 0;

	} // stronger side has no more than a minor piece

	// no win if stronger side has just one minor piece and no pawns
	if (p->pcCount[stronger][P] == 0) {   
       if ( p->pieceMat[stronger] < 400 ) return 0;
	}

    // no pawns of either color on the board
    if (p->pcCount[stronger][P] == 0 
    &&  p->pcCount[weaker  ][P] == 0) 
	{
      if (PcMatNN(p,stronger) ) return 0;

	  // low and almost equal material, except KBB vs KN:     1/8
	  if ( PcMatRook(p, stronger)  && PcMatRook(p, weaker) ) return 8;
	  if ( PcMatQueen(p, stronger) && PcMatQueen(p, weaker) ) return 8;
	  if ( PcMatRookMinor(p, stronger) && PcMatRookMinor(p, weaker) ) return 8;
	  if ( PcMatBN(p, stronger) && ( PcMatMinor (p, weaker) || PcMatRook(p, weaker) ) ) return 8;
	  if ( PcMatBB(p, stronger) && ( PcMatBishop(p, weaker) || PcMatRook(p, weaker) ) ) return 8;
	}

	if (p->pcCount[stronger][P] == 0 ) {

       // it's hard to win with a bare rook against a minor/minor + pawn
       if (p->pieceMat[stronger] == Data.matValue[R] 
       && p->pieceMat[weaker] > 0 ) return 16; // 1/4

       // it's hard to win with a rook and a minor against a rook/rook + pawns
       if (PcMatRookMinor(p, stronger) 
       && PcMatRook(p, weaker) ) return 16; // 1/4

       // it's hard to win with a queen and a minor against a queen/queen + pawns
       if ( PcMatQueenMinor(p, stronger) 
       && PcMatQueen(p, weaker) ) return 32; // 1/2
	}

	// rare KNPK draw rule
	if (PcMatKnight(p, stronger) 
    &&  PcMatNone(p, weaker)
    &&  p->pcCount[stronger][P] == 1
    &&  p->pcCount[weaker][P] == 0 ) {
        if ( ( RelSqBb(A7,stronger) & bbPc(p, stronger, P) )
		&&   ( RelSqBb(A8,stronger) & bbPc(p, weaker, K) ) ) return 0; // dead draw
        if ( ( RelSqBb(H7,stronger) & bbPc(p, stronger, P) )
		&&   ( RelSqBb(H8,stronger) & bbPc(p, weaker, K) ) ) return 0; // dead draw
	}

    // some special rules for rook endgame with one pawn
    if (PcMatRook(p, stronger)
    &&  PcMatRook(p, weaker)
    &&  p->pcCount[stronger][P] == 1
    &&  p->pcCount[weaker][P] == 0
  ) {
      // good defensive position with a king on pawn's path increases drawing chances
	  if ( ( SqBb(p->kingSquare[weaker]) & GetFrontSpan( bbPc(p, stronger, P), stronger ) ) ) 
	      return 32; // 1/2

	  // draw code for rook endgame with edge pawn
	  if ( ( RelSqBb(A7,stronger) & bbPc(p, stronger, P) )
	  &&   ( RelSqBb(A8,stronger) & bbPc(p, stronger, R) )
	  &&   ( bbFILE_A & bbPc(p, weaker, R) )
	  &&   ( ( RelSqBb(H7,stronger) & bbPc(p, weaker, K) ) || ( RelSqBb(G7,stronger) & bbPc(p, weaker, K) ) )
		  ) return 0; // dead draw

	  if ( ( RelSqBb(H7,stronger) & bbPc(p, stronger, P) )
	  &&   ( RelSqBb(H8,stronger) & bbPc(p, stronger, R) )
	  &&   ( bbFILE_H & bbPc(p, weaker, R) )
	  &&   ( ( RelSqBb(A7,stronger) & bbPc(p, weaker, K) ) || ( RelSqBb(B7,stronger) & bbPc(p, weaker, K) ) )
		  ) return 0; // dead draw

	  // TODO: back rank defense
	}

	// TODO: KRPPKRP with no passers
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

int PcMatNone(sPosition *p, int side) {
	return ( p->pieceMat[side] == 0 );
}

int PcMatKnight(sPosition *p, int side) {
   return ( p->pieceMat[side] == Data.matValue[N] );
}

int PcMatBishop(sPosition *p, int side) {
   return ( p->pieceMat[side] == Data.matValue[B] );
}

int PcMatMinor(sPosition *p, int side) {
   return ( (p->pieceMat[side] ==  Data.matValue[B]) 
	 || (p->pieceMat[side] ==  Data.matValue[N]) );
}

int PcMatRook(sPosition *p, int side) {
   return ( p->pieceMat[side] == Data.matValue[R] );
}

int PcMatRookMinor(sPosition *p, int side) {
   return (    (p->pieceMat[side] == Data.matValue[R] + Data.matValue[B]) 
   || (p->pieceMat[side] == Data.matValue[R] + Data.matValue[N]) );
}

int PcMatQueen(sPosition *p, int side) {
   return ( p->pieceMat[side] == Data.matValue[Q] );
}

int PcMatQueenMinor(sPosition *p, int side) {
    return (    (p->pieceMat[side] == Data.matValue[Q] + Data.matValue[B]) 
	     || (p->pieceMat[side] == Data.matValue[Q] + Data.matValue[N]) );
}

int PcMatNN(sPosition *p, int side) {
    return ( p->pieceMat[side] == 2*Data.matValue[N] );
}

int PcMatBB(sPosition *p, int side) {
    return ( p->pieceMat[side] == 2*Data.matValue[B] );
}

int PcMatBN(sPosition *p, int side) {
	return ( p->pieceMat[side] == Data.matValue[B] + Data.matValue[N] );
}
