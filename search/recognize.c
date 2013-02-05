#include "../rodent.h"
#include "../bitboard/bitboard.h"
#include "search.h"

int sSearcher::RecognizeDraw(sPosition *p) 
{
  U64 bbPawn;

  // bare kings or Km vs K are classified as a draw and not searched any further
  if ( p->pcCount[WHITE][P] == 0 && p->pcCount[BLACK][P] == 0 ) {		  
	  // K(m) vs K
	  if ( p->pieceMat[WHITE] + p->pieceMat[BLACK] < 400 ) {
	     if ( !IllegalPosition(p) ) return 1;
	  }
	  
      // Km vs Km, jut in case we take care that no king is on the rim 
	  if (p->pieceMat[WHITE] < 400 
	  &&  p->pieceMat[BLACK] < 400
	  &&  (bbPc(p, WHITE, K) & bbRim) == 0
	  &&  (bbPc(p, BLACK, K) & bbRim) == 0 ) {
		  if ( !IllegalPosition(p) ) return 1;
	  }
  } // no pawns

  // pawns only
  if (p->pieceMat[WHITE] == 0 && p->pieceMat[BLACK] == 0) {

	  // exactly one white pawn
	  if (p->pcCount[WHITE][P] == 1 && p->pcCount[BLACK][P] == 0) {
		  bbPawn = bbPc(p, WHITE, P);
		  
		  // opposition through a pawn
		  if ( p->side == WHITE
		  && (SqBb(p->kingSquare[BLACK]) & ShiftFwd(bbPawn, WHITE) )
		  && (SqBb(p->kingSquare[WHITE]) & ShiftFwd(bbPawn, BLACK) )
		  ) return 1;

		  // black can create opposition through a pawn in one move
		  if ( p->side == BLACK
		  && (bbKingAttacks[p->kingSquare[BLACK]] & ShiftFwd(bbPawn, WHITE) )
		  && (SqBb(p->kingSquare[WHITE]) & ShiftFwd(bbPawn, BLACK) )
		  ) if ( !IllegalPosition(p) ) return 1;

	  } // exactly one white pawn

	  // exactly one black pawn
	  if (p->pcCount[BLACK][P] == 1 && p->pcCount[WHITE][P] == 0) {
		  bbPawn = bbPc(p, BLACK, P);

		  // opposition through a pawn
		  if ( p->side == BLACK
		  && (SqBb(p->kingSquare[WHITE]) & ShiftFwd(bbPawn, BLACK) )
		  && (SqBb(p->kingSquare[BLACK]) & ShiftFwd(bbPawn, WHITE) )
		  ) return 1;

		  // white can create opposition through a pawn in one move
		  if ( p->side == WHITE
		  && ( bbKingAttacks[p->kingSquare[WHITE] ] & ShiftFwd(bbPawn, BLACK) )
		  && (SqBb(p->kingSquare[BLACK]) & ShiftFwd(bbPawn, WHITE) )
		  ) if ( !IllegalPosition(p) ) return 1;

	  } // exactly one black pawn

  } // pawns only

  return 0;
}