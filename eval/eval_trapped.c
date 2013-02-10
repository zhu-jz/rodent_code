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
#include "../bitboard/bitboard.h"
#include "eval.h"

int sEvaluator::EvalTrappedRook(sPosition *p) 
{
	int score = 0;
	const U64 bbWKTrapKs  = SqBb(F1) | SqBb(G1);
	const U64 bbWRTrapKs  = SqBb(G1) | SqBb(H1) | SqBb(H2);
	const U64 bbWKTrapQs  = SqBb(C1) | SqBb(B1);
	const U64 bbWRTrapQs  = SqBb(B1) | SqBb(A1) | SqBb(A2);
	const U64 bbBKTrapKs  = SqBb(F8) | SqBb(G8);
	const U64 bbBRTrapKs  = SqBb(G8) | SqBb(H8) | SqBb(H7);
	const U64 bbBKTrapQs  = SqBb(C8) | SqBb(B8);
	const U64 bbBRTrapQs  = SqBb(B8) | SqBb(A8) | SqBb(A7);  

  // rook blocked by uncastled king
  // TODO: test lower value
  if ( ( bbPc(p, WHITE, K) & bbWKTrapKs ) && ( bbPc(p, WHITE, R) & bbWRTrapKs ) ) score -= 50;
  if ( ( bbPc(p, WHITE, K) & bbWKTrapQs ) && ( bbPc(p, WHITE, R) & bbWRTrapQs ) ) score -= 50;
  if ( ( bbPc(p, BLACK, K) & bbBKTrapKs ) && ( bbPc(p, BLACK, R) & bbBRTrapKs ) ) score += 50;
  if ( ( bbPc(p, BLACK, K) & bbBKTrapQs ) && ( bbPc(p, BLACK, R) & bbBRTrapQs ) ) score += 50;

  return score;
}

int sEvaluator::EvalTrappedBishop(sPosition *p) 
{
	int score = 0;
	const U64 bbTrapSq[2] = { SqBb(A7) | SqBb(B8) | SqBb(H7) | SqBb(G8) | SqBb(A6) | SqBb(H6) ,
		                      SqBb(A2) | SqBb(B1) | SqBb(H2) | SqBb(G1) | SqBb(A3) | SqBb(H3) };

  if ( bbPc(p, WHITE, B) & bbTrapSq[WHITE] ) {
     if ( ( bbPc(p, WHITE, B) & SqBb(A7) ) && ( bbPc(p, BLACK, P) & SqBb(B6) ) ) score -= 150;
     if ( ( bbPc(p, WHITE, B) & SqBb(B8) ) && ( bbPc(p, BLACK, P) & SqBb(C7) ) ) score -= 150;
     if ( ( bbPc(p, WHITE, B) & SqBb(H7) ) && ( bbPc(p, BLACK, P) & SqBb(G6) ) ) score -= 150;
     if ( ( bbPc(p, WHITE, B) & SqBb(G8) ) && ( bbPc(p, BLACK, P) & SqBb(F7) ) ) score -= 150;
     if ( ( bbPc(p, WHITE, B) & SqBb(A6) ) && ( bbPc(p, BLACK, P) & SqBb(B5) ) ) score -= 50;
     if ( ( bbPc(p, WHITE, B) & SqBb(H6) ) && ( bbPc(p, BLACK, P) & SqBb(G5) ) ) score -= 50;
  }

  if ( bbPc(p, BLACK, B) & bbTrapSq[BLACK] ) {
     if ( ( bbPc(p, BLACK, B) & SqBb(A2) ) && ( bbPc(p, WHITE, P) & SqBb(B3) ) ) score += 150;
     if ( ( bbPc(p, BLACK, B) & SqBb(B1) ) && ( bbPc(p, WHITE, P) & SqBb(C2) ) ) score += 150;
     if ( ( bbPc(p, BLACK, B) & SqBb(H2) ) && ( bbPc(p, WHITE, P) & SqBb(G3) ) ) score += 150;
     if ( ( bbPc(p, BLACK, B) & SqBb(G1) ) && ( bbPc(p, WHITE, P) & SqBb(F2) ) ) score += 150;
     if ( ( bbPc(p, BLACK, B) & SqBb(A3) ) && ( bbPc(p, WHITE, P) & SqBb(B4) ) ) score += 50;
     if ( ( bbPc(p, BLACK, B) & SqBb(H3) ) && ( bbPc(p, WHITE, P) & SqBb(G4) ) ) score += 50;
  }

  return score;
}

int sEvaluator::EvalTrappedKnight(sPosition *p) 
{
  int score = 0;
  const U64 bbTrapSq[2] = { SqBb(A7) | SqBb(A6) | SqBb(H7) | SqBb(H6),
	                        SqBb(A2) | SqBb(A3) | SqBb(H2) | SqBb(H3) };
  
  if (bbPc(p, WHITE, N) & bbTrapSq[WHITE]) {

    if ( bbPc(p, WHITE, N) & SqBb(A7) ) {
	    if ( bbPc(p, BLACK, P) & SqBb(A6) )  {
	  	    score -= 75;
	        if ( bbPc(p, BLACK, P) & SqBb(B7) )  score -= 75;
	    }
    }

    if ( bbPc(p, WHITE, N) & SqBb(H7) ) {
	    if ( bbPc(p, BLACK, P) & SqBb(G6) )  {
	  	    score -= 75;
	        if ( bbPc(p, BLACK, P) & SqBb(G7) )  score -= 75;
	    }
    }

    if ( bbPc(p, WHITE, N) & SqBb(H6) ) {
	    if ( ( bbPc(p, BLACK, P) & SqBb(H5) ) && ( bbPc(p, BLACK, P) & SqBb(G6) ) ) score -= 75;	  
    }

    if ( bbPc(p, WHITE, N) & SqBb(A6) ) {
	    if ( ( bbPc(p, BLACK, P) & SqBb(A5) ) && ( bbPc(p, BLACK, P) & SqBb(B6) ) ) score -= 75;	  
    }
  }

  if ( bbPc(p,BLACK,N) & bbTrapSq[BLACK] ) {
    if ( bbPc(p, BLACK, N) & SqBb(A2) ) {
	    if ( bbPc(p, WHITE, P) & SqBb(A3) )  {
		    score += 75;
	        if ( bbPc(p, WHITE, P) & SqBb(B2) )  score += 75;
	    }
    }

    if ( bbPc(p, BLACK, N) & SqBb(H2) ) {
	    if ( bbPc(p, WHITE, P) & SqBb(H3) )  {
		    score += 75;
	        if ( bbPc(p, WHITE, P) & SqBb(G2) )  score += 75;
	    }
    }

    if ( bbPc(p, BLACK, N) & SqBb(H3) ) {
	    if ( ( bbPc(p, WHITE, P) & SqBb(H4) ) && ( bbPc(p, WHITE, P) & SqBb(G3) ) ) score += 75;	  
    }

    if ( bbPc(p, BLACK, N) & SqBb(A3) ) {
	    if ( ( bbPc(p, WHITE, P) & SqBb(A4) ) && ( bbPc(p, WHITE, P) & SqBb(B3) ) ) score += 75;	  
    }
  }

  return score;
}
