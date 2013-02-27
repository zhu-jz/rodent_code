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

int IsLegal(sPosition *p, int move)
{
  int side = p->side;        // moving side 
  int fsq  = Fsq(move);      // start square
  int tsq  = Tsq(move);      // target square
  int ftp  = TpOnSq(p, fsq); // moving piece
  int ttp  = TpOnSq(p, tsq); // captured piece

  // we must move own piece
  if (ftp == NO_TP || Cl(p->pc[fsq]) != side)
    return 0;
  
  // we cannot capture own piece
  if (ttp != NO_TP && Cl(p->pc[tsq]) == side)
    return 0;

  switch (MoveType(move)) {
  case NORMAL:
    break;

  case CASTLE:
    if (side == WHITE) {
      if (fsq != E1)
        return 0;
      if (tsq > fsq) {
        if ((p->castleFlags & W_KS) && !(OccBb(p) & (U64)0x0000000000000060))
          if (!IsAttacked(p, E1, BLACK) && !IsAttacked(p, F1, BLACK))
            return 1;
      } else {
        if ((p->castleFlags & W_QS) && !(OccBb(p) & (U64)0x000000000000000E))
          if (!IsAttacked(p, E1, BLACK) && !IsAttacked(p, D1, BLACK))
            return 1;
      }
    } else {
      if (fsq != E8)
        return 0;
      if (tsq > fsq) {
        if ((p->castleFlags & B_KS) && !(OccBb(p) & (U64)0x6000000000000000))
          if (!IsAttacked(p, E8, WHITE) && !IsAttacked(p, F8, WHITE))
            return 1;
      } else {
        if ((p->castleFlags & B_QS) && !(OccBb(p) & (U64)0x0E00000000000000))
          if (!IsAttacked(p, E8, WHITE) && !IsAttacked(p, D8, WHITE))
            return 1;
      }
    }
    return 0;

  case EP_CAP:
    if (ftp == P && tsq == p->epSquare)
      return 1;
    return 0;

  case EP_SET:
    if (ftp == P && ttp == NO_TP && p->pc[tsq ^ 8] == NO_PC)
      if ((tsq > fsq && side == WHITE) ||
          (tsq < fsq && side == BLACK))
        return 1;
    return 0;
  }
  if (ftp == P) {
    if (side == WHITE) {
      if (Rank(fsq) == RANK_7 && !IsProm(move))
        return 0;
      if (tsq - fsq == 8)
        if (ttp == NO_TP)
          return 1;
      if ((tsq - fsq == 7 && File(fsq) != FILE_A) ||
          (tsq - fsq == 9 && File(fsq) != FILE_H))
        if (ttp != NO_TP)
          return 1;
    } else {
      if (Rank(fsq) == RANK_2 && !IsProm(move))
        return 0;
      if (tsq - fsq == -8)
        if (ttp == NO_TP)
          return 1;
      if ((tsq - fsq == -9 && File(fsq) != FILE_A) ||
          (tsq - fsq == -7 && File(fsq) != FILE_H))
        if (ttp != NO_TP)
          return 1;
    }
    return 0;
  }
  if (IsProm(move))
    return 0;
  return (AttacksFrom(p, fsq) & SqBb(tsq)) != 0;
}
