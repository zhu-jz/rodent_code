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

#include "bitboard/bitboard.h"
#include "data.h"
#include "rodent.h"
#include "bitboard/gencache.h"

int *GenerateCaptures(sPosition *p, int *list)
{
  U64 bbPieces, bbMoves;
  int side, from, to;

  side = p->side;
  U64 bbOpp = p->bbCl[Opp(side)];

  if (side == WHITE) {

    bbMoves = ((bbPc(p, WHITE, P) & bbNotA & bbRANK_7) << 7) & p->bbCl[BLACK];
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(Q_PROM, to-7, to);
	  *list++ = SetMove(N_PROM, to-7, to);
      *list++ = SetMove(R_PROM, to-7, to);
      *list++ = SetMove(B_PROM, to-7, to);
    }

    bbMoves = ((bbPc(p, WHITE, P) & bbNotH & bbRANK_7) << 9) & p->bbCl[BLACK];
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(Q_PROM, to-9, to);
	  *list++ = SetMove(N_PROM, to-9, to);
      *list++ = SetMove(R_PROM, to-9, to);
      *list++ = SetMove(B_PROM, to-9, to);
    }

    bbMoves = ((bbPc(p, WHITE, P) & bbRANK_7) << 8) & UnoccBb(p);
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(Q_PROM, to-8, to);
	  *list++ = SetMove(N_PROM, to-8, to);
      *list++ = SetMove(R_PROM, to-8, to);
      *list++ = SetMove(B_PROM, to-8, to);
    }

    bbMoves = ShiftNW(bbPc(p, WHITE, P) & ~bbRANK_7) & p->bbCl[BLACK];
    while (bbMoves) {
      to = PopFirstBit(&bbMoves);
      *list++ = SetMove(NORMAL, to-7, to);
    }

    bbMoves = ShiftNE(bbPc(p, WHITE, P) & ~bbRANK_7) & p->bbCl[BLACK];
    while (bbMoves) {
      to = PopFirstBit(&bbMoves);
      *list++ = SetMove(NORMAL, to-9, to);
    }

	// en passant capture
    if ((to = p->epSquare) != NO_SQ) {
      if (((bbPc(p, WHITE, P) & bbNotA) << 7) & SqBb(to))
        *list++ = SetMove(EP_CAP, to-7, to);
      if (((bbPc(p, WHITE, P) & bbNotH) << 9) & SqBb(to))
        *list++ = SetMove(EP_CAP, to-9, to);
    }

  bbPieces = bbPc(p, side, N);
  while (bbPieces) {
    from = PopFlippedBit(&bbPieces);
    bbMoves = bbKnightAttacks[from] & bbOpp;

    while (bbMoves) {
      to = PopFirstBit(&bbMoves);
      *list++ = SetMove(NORMAL, from, to);
    }
  }

  bbPieces = bbPc(p, side, B);
  while (bbPieces) {
    from = PopFlippedBit(&bbPieces);
	bbMoves = GenCache.GetBishMob(OccBb(p), from) & bbOpp;
    while (bbMoves) {
      to = PopFirstBit(&bbMoves);
      *list++ = SetMove(NORMAL, from, to);
    }
  }

 bbPieces = bbPc(p, side, R);
  while (bbPieces) {
    from = PopFlippedBit(&bbPieces);
	bbMoves = GenCache.GetRookMob(OccBb(p), from) & bbOpp;
    
    while (bbMoves) {
      to = PopFirstBit(&bbMoves);
      *list++ = SetMove(NORMAL, from, to);
    }
  }

  bbPieces = bbPc(p, side, Q);
  while (bbPieces) {
    from = PopFlippedBit(&bbPieces);
    bbMoves = GenCache.GetQueenMob(OccBb(p), from) & bbOpp;
	
    while (bbMoves) {
      to = PopFirstBit(&bbMoves);
      *list++ = SetMove(NORMAL, from, to);
    }
  }

  } else {

    bbMoves = ((bbPc(p, BLACK, P) & bbNotA & bbRANK_2) >> 9) & p->bbCl[WHITE];
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(Q_PROM, to+9, to);
	  *list++ = SetMove(N_PROM, to+9, to);
      *list++ = SetMove(R_PROM, to+9, to);
      *list++ = SetMove(B_PROM, to+9, to);
    }

    bbMoves = ((bbPc(p, BLACK, P) & bbNotH & bbRANK_2) >> 7) & p->bbCl[WHITE];
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(Q_PROM, to+7, to);
	  *list++ = SetMove(N_PROM, to+7, to);
      *list++ = SetMove(R_PROM, to+7, to);
      *list++ = SetMove(B_PROM, to+7, to);
    }

    bbMoves = ((bbPc(p, BLACK, P) & bbRANK_2) >> 8) & UnoccBb(p);
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(Q_PROM, to+8, to);
	  *list++ = SetMove(N_PROM, to+8, to);
      *list++ = SetMove(R_PROM, to+8, to);
      *list++ = SetMove(B_PROM, to+8, to);
    }

    bbMoves = ShiftSW(bbPc(p, BLACK, P) & ~bbRANK_2) & p->bbCl[WHITE];
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(NORMAL, to+9, to);
    }

    bbMoves = ShiftSE(bbPc(p, BLACK, P) & ~bbRANK_2) & p->bbCl[WHITE];
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(NORMAL, to+7, to);
    }

    if ((to = p->epSquare) != NO_SQ) {
      if (((bbPc(p, BLACK, P) & bbNotA) >> 9) & SqBb(to))
        *list++ = SetMove(EP_CAP, to+9, to);
      if (((bbPc(p, BLACK, P) & bbNotH) >> 7) & SqBb(to))
        *list++ = SetMove(EP_CAP, to+7, to);
    }

  bbPieces = bbPc(p, side, N);
  while (bbPieces) {
    from = PopFirstBit(&bbPieces);
    bbMoves = bbKnightAttacks[from] & bbOpp;

    while (bbMoves) {
      to = PopFlippedBit(&bbMoves);
      *list++ = SetMove(NORMAL,from,to);
    }
  }

  bbPieces = bbPc(p, side, B);
  while (bbPieces) {
    from = PopFirstBit(&bbPieces);
	bbMoves = GenCache.GetBishMob(OccBb(p), from) & bbOpp;

    while (bbMoves) {
      to = PopFlippedBit(&bbMoves);
      *list++ = SetMove(NORMAL,from,to);
    }
  }

 bbPieces = bbPc(p, side, R);
  while (bbPieces) {
    from = PopFirstBit(&bbPieces);
	bbMoves = GenCache.GetRookMob(OccBb(p), from) & bbOpp;
    
    while (bbMoves) {
      to = PopFlippedBit(&bbMoves);
      *list++ = SetMove(NORMAL,from,to);
    }
  }

  bbPieces = bbPc(p, side, Q);
  while (bbPieces) {
    from = PopFirstBit(&bbPieces);
    bbMoves = GenCache.GetQueenMob(OccBb(p), from) & bbOpp;
	
    while (bbMoves) {
      to = PopFlippedBit(&bbMoves);
      *list++ = SetMove(NORMAL,from,to);
    }
  }

  }

  bbMoves = bbKingAttacks[KingSq(p, side)] & bbOpp;
  while (bbMoves) {
    to = PopNextBit(Opp(side), &bbMoves);
    *list++ = SetMove(NORMAL,KingSq(p, side),to);
  }
  return list;
}

int *GenerateQuiet(sPosition *p, int *list)
{
  U64 bbPieces, bbMoves;
  U64 bbEmptySq  = UnoccBb(p);
  U64 bbOccupied = ~bbEmptySq;
  int side, from, to;

  side = p->side;
  if (side == WHITE) {
    // white castle
    if ((p->castleFlags & W_KS) && !(bbOccupied & (U64)0x0000000000000060))
      if (!IsAttacked(p, E1, BLACK) && !IsAttacked(p, F1, BLACK))
        *list++ = SetMove(CASTLE, E1, G1);
    if ((p->castleFlags & W_QS) && !(bbOccupied & (U64)0x000000000000000E))
      if (!IsAttacked(p, E1, BLACK) && !IsAttacked(p, D1, BLACK))
        *list++ = SetMove(CASTLE, E1, C1);

	// white pawns
	bbMoves = ((((bbPc(p, WHITE, P) & bbRANK_2) << 8) & bbEmptySq ) << 8) & bbEmptySq;
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(EP_SET, to-16, to);
    }

	bbMoves = ((bbPc(p, WHITE, P) & ~bbRANK_7) << 8) & bbEmptySq;
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(NORMAL, to-8, to);
    }
  } else {
    
	// black castle
	if ((p->castleFlags & B_KS) && !(bbOccupied & (U64)0x6000000000000000))
      if (!IsAttacked(p, E8, WHITE) && !IsAttacked(p, F8, WHITE))
        *list++ = SetMove(CASTLE, E8, G8);
    if ((p->castleFlags & B_QS) && !(bbOccupied & (U64)0x0E00000000000000))
      if (!IsAttacked(p, E8, WHITE) && !IsAttacked(p, D8, WHITE))
        *list++ = SetMove(CASTLE, E8, C8);
		
    
	// black pawns
	bbMoves = ((((bbPc(p, BLACK, P) & bbRANK_7) >> 8) & bbEmptySq) >> 8) & bbEmptySq;
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(EP_SET, to+16, to);
    }
    bbMoves = ((bbPc(p, BLACK, P) & ~bbRANK_2) >> 8) & bbEmptySq;
    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(NORMAL, to+8, to);
    }
  }

  // knight moves
  bbPieces = bbPc(p, side, N);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
    bbMoves = bbKnightAttacks[from] & bbEmptySq;

    while (bbMoves) {
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(NORMAL, from, to);
    }
  }

  // bishop moves
  bbPieces = bbPc(p, side, B);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
	bbMoves = GenCache.GetBishMob(OccBb(p), from) & bbEmptySq;

    while (bbMoves) { // serialize moves
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(NORMAL, from, to);
    }
  }

  // rook moves
  bbPieces = bbPc(p, side, R);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
	bbMoves = GenCache.GetRookMob(OccBb(p), from) & bbEmptySq;

    while (bbMoves) { // serialize moves
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(NORMAL, from, to);
    }
  }

  // queen moves
  bbPieces = bbPc(p, side, Q);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
	bbMoves = GenCache.GetQueenMob(OccBb(p), from) & bbEmptySq;
    
	while (bbMoves) { // serialize moves
      to = PopNextBit(Opp(side), &bbMoves);
      *list++ = SetMove(NORMAL, from, to);
    }
  }

  // king moves
  bbMoves = bbKingAttacks[KingSq(p, side)] & bbEmptySq;

  while (bbMoves) { // serialize moves
    to = PopNextBit(Opp(side), &bbMoves);
    *list++ = SetMove(NORMAL, KingSq(p, side), to);
  }
  return list;
}
