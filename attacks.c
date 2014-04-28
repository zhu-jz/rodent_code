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

U64 AttacksFrom(sPosition *p, int sq)
{
  switch (TpOnSq(p, sq)) {
  case P:
    return bbPawnAttacks[Cl(p->pc[sq])][sq];
  case N:
    return bbKnightAttacks[sq];
  case B:
    return BAttacks(OccBb(p), sq);
  case R:
    return RAttacks(OccBb(p), sq);
  case Q:
    return QAttacks(OccBb(p), sq);
  case K:
    return bbKingAttacks[sq];
  }
  return 0;
}

U64 AttacksTo(sPosition *p, int sq)
{
  return (bbPc(p, WHITE, P) & bbPawnAttacks[BLACK][sq]) |
         (bbPc(p, BLACK, P) & bbPawnAttacks[WHITE][sq]) |
         (p->bbTp[N] & bbKnightAttacks[sq]) |
         ((p->bbTp[B] | p->bbTp[Q]) & BAttacks(OccBb(p), sq)) |
         ((p->bbTp[R] | p->bbTp[Q]) & RAttacks(OccBb(p), sq)) |
         (p->bbTp[K] & bbKingAttacks[sq]);
}

int IsAttacked(sPosition *p, int sq, int side)
{
  return (bbPc(p, side, P) & bbPawnAttacks[Opp(side)][sq]) ||
         (bbPc(p, side, N) & bbKnightAttacks[sq]) ||
         ((bbPc(p, side, B) | bbPc(p, side, Q)) & BAttacks(OccBb(p), sq)) ||
         ((bbPc(p, side, R) | bbPc(p, side, Q)) & RAttacks(OccBb(p), sq)) ||
         (bbPc(p, side, K) & bbKingAttacks[sq]);
}
