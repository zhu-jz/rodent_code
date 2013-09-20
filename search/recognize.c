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
	  &&  (bbPc(p, WHITE, K)& bbRim) == 0
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