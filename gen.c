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

#include "bitboard/bitboard.h"
#include "data.h"
#include "rodent.h"
#include "bitboard/gencache.h"

int *GenerateCaptures(sPosition *p, int *list)
{
  U64 bbPieces, bbMoves;
  int side, from, to;

  side = p->side;
  if (side == WHITE) {

    bbMoves = ((bbPc(p, WHITE, P) & bbNotA & bbRANK_7) << 7) & p->bbCl[BLACK];
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to - 7);
	  *list++ = (N_PROM << 12) | (to << 6) | (to - 7);
      *list++ = (R_PROM << 12) | (to << 6) | (to - 7);
      *list++ = (B_PROM << 12) | (to << 6) | (to - 7);
    }

    bbMoves = ((bbPc(p, WHITE, P) & bbNotH & bbRANK_7) << 9) & p->bbCl[BLACK];
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to - 9);
	  *list++ = (N_PROM << 12) | (to << 6) | (to - 9);
      *list++ = (R_PROM << 12) | (to << 6) | (to - 9);
      *list++ = (B_PROM << 12) | (to << 6) | (to - 9);
    }

    bbMoves = ((bbPc(p, WHITE, P) & bbRANK_7) << 8) & UnoccBb(p);
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to - 8);
	  *list++ = (N_PROM << 12) | (to << 6) | (to - 8);
      *list++ = (R_PROM << 12) | (to << 6) | (to - 8);
      *list++ = (B_PROM << 12) | (to << 6) | (to - 8);
    }

    bbMoves = ShiftNW(bbPc(p, WHITE, P) & ~bbRANK_7) & p->bbCl[BLACK];
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | (to - 7);
    }

    bbMoves = ShiftNE(bbPc(p, WHITE, P) & ~bbRANK_7) & p->bbCl[BLACK];
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | (to - 9);
    }

	// en passant capture
    if ((to = p->epSquare) != NO_SQ) {
      if (((bbPc(p, WHITE, P) & bbNotA) << 7) & SqBb(to))
        *list++ = (EP_CAP << 12) | (to << 6) | (to - 7);
      if (((bbPc(p, WHITE, P) & bbNotH) << 9) & SqBb(to))
        *list++ = (EP_CAP << 12) | (to << 6) | (to - 9);
    }

  } else {

    bbMoves = ((bbPc(p, BLACK, P) & bbNotA & bbRANK_2) >> 9) & p->bbCl[WHITE];
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to + 9);
	  *list++ = (N_PROM << 12) | (to << 6) | (to + 9);
      *list++ = (R_PROM << 12) | (to << 6) | (to + 9);
      *list++ = (B_PROM << 12) | (to << 6) | (to + 9);
    }

    bbMoves = ((bbPc(p, BLACK, P) & bbNotH & bbRANK_2) >> 7) & p->bbCl[WHITE];
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to + 7);
	  *list++ = (N_PROM << 12) | (to << 6) | (to + 7);
      *list++ = (R_PROM << 12) | (to << 6) | (to + 7);
      *list++ = (B_PROM << 12) | (to << 6) | (to + 7);
    }

    bbMoves = ((bbPc(p, BLACK, P) & bbRANK_2) >> 8) & UnoccBb(p);
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (Q_PROM << 12) | (to << 6) | (to + 8);
	  *list++ = (N_PROM << 12) | (to << 6) | (to + 8);
      *list++ = (R_PROM << 12) | (to << 6) | (to + 8);
      *list++ = (B_PROM << 12) | (to << 6) | (to + 8);
    }

    bbMoves = ShiftSW(bbPc(p, BLACK, P) & ~bbRANK_2) & p->bbCl[WHITE];
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | (to + 9);
    }

    bbMoves = ShiftSE(bbPc(p, BLACK, P) & ~bbRANK_2) & p->bbCl[WHITE];
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | (to + 7);
    }

    if ((to = p->epSquare) != NO_SQ) {
      if (((bbPc(p, BLACK, P) & bbNotA) >> 9) & SqBb(to))
        *list++ = (EP_CAP << 12) | (to << 6) | (to + 9);
      if (((bbPc(p, BLACK, P) & bbNotH) >> 7) & SqBb(to))
        *list++ = (EP_CAP << 12) | (to << 6) | (to + 7);
    }
  }
  
  bbPieces = bbPc(p, side, N);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
    bbMoves = bbKnightAttacks[from] & p->bbCl[Opp(side)];

    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | from;
    }
  }

  bbPieces = bbPc(p, side, B);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
	bbMoves = GenCache.GetBishMob(OccBb(p), from) & p->bbCl[Opp(side)];

    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | from;
    }
  }
  
  bbPieces = bbPc(p, side, R);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
	bbMoves = GenCache.GetRookMob(OccBb(p), from) & p->bbCl[Opp(side)];
    
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | from;
    }
  }

  bbPieces = bbPc(p, side, Q);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
    bbMoves = GenCache.GetQueenMob(OccBb(p), from) & p->bbCl[Opp(side)];
	
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | from;
    }
  }

  bbMoves = bbKingAttacks[KingSq(p, side)] & p->bbCl[Opp(side)];
  while (bbMoves) {
    to = PopNextBit(side, &bbMoves);
    *list++ = (to << 6) | KingSq(p, side);
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
    if ((p->castleFlags & 1) && !(bbOccupied & (U64)0x0000000000000060))
      if (!IsAttacked(p, E1, BLACK) && !IsAttacked(p, F1, BLACK))
        *list++ = (CASTLE << 12) | (G1 << 6) | E1;
    if ((p->castleFlags & 2) && !(bbOccupied & (U64)0x000000000000000E))
      if (!IsAttacked(p, E1, BLACK) && !IsAttacked(p, D1, BLACK))
        *list++ = (CASTLE << 12) | (C1 << 6) | E1;

	// white pawns
	bbMoves = ((((bbPc(p, WHITE, P) & bbRANK_2) << 8) & bbEmptySq ) << 8) & bbEmptySq;
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (EP_SET << 12) | (to << 6) | (to - 16);
    }

	bbMoves = ((bbPc(p, WHITE, P) & ~bbRANK_7) << 8) & bbEmptySq;
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | (to - 8);
    }
  } else {
    
	// black castle
	if ((p->castleFlags & 4) && !(bbOccupied & (U64)0x6000000000000000))
      if (!IsAttacked(p, E8, WHITE) && !IsAttacked(p, F8, WHITE))
        *list++ = (CASTLE << 12) | (G8 << 6) | E8;
    if ((p->castleFlags & 8) && !(bbOccupied & (U64)0x0E00000000000000))
      if (!IsAttacked(p, E8, WHITE) && !IsAttacked(p, D8, WHITE))
        *list++ = (CASTLE << 12) | (C8 << 6) | E8;
    
	// black pawns
	bbMoves = ((((bbPc(p, BLACK, P) & bbRANK_7) >> 8) & bbEmptySq) >> 8) & bbEmptySq;
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (EP_SET << 12) | (to << 6) | (to + 16);
    }
    bbMoves = ((bbPc(p, BLACK, P) & ~bbRANK_2) >> 8) & bbEmptySq;
    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | (to + 8);
    }
  }

  // knight moves
  bbPieces = bbPc(p, side, N);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
    bbMoves = bbKnightAttacks[from] & bbEmptySq;

    while (bbMoves) {
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | from;
    }
  }

  // bishop moves
  bbPieces = bbPc(p, side, B);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
	bbMoves = GenCache.GetBishMob(OccBb(p), from) & bbEmptySq;

    while (bbMoves) { // serialize moves
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | from;
    }
  }

  // rook moves
  bbPieces = bbPc(p, side, R);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
	bbMoves = GenCache.GetRookMob(OccBb(p), from) & bbEmptySq;

    while (bbMoves) { // serialize moves
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | from;
    }

  }

  // queen moves
  bbPieces = bbPc(p, side, Q);
  while (bbPieces) {
    from = PopNextBit(side, &bbPieces);
	bbMoves = GenCache.GetQueenMob(OccBb(p), from) & bbEmptySq;

    while (bbMoves) { // serialize moves
      to = PopNextBit(side, &bbMoves);
      *list++ = (to << 6) | from;
    }
  }

  // king moves
  bbMoves = bbKingAttacks[KingSq(p, side)] & bbEmptySq;

  while (bbMoves) { // serialize moves
    to = PopNextBit(side, &bbMoves);
    *list++ = (to << 6) | KingSq(p, side);
  }
  return list;
}
