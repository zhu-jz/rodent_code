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
#include "../data.h"

void sManipulator::UndoMove(sPosition *p, int move, UNDO *u)
{
  int side, fsq, tsq, ftp, ttp;

  side = Opp(p->side);   // moving side
  fsq  = Fsq(move);      // start square
  tsq  = Tsq(move);      // target square
  ftp  = TpOnSq(p, tsq); // moving piece
  ttp  = u->ttp;         // captured piece

  U64 bbMove = SqBb(fsq) | SqBb(tsq); // optimization from Stockfish

  p->castleFlags = u->castleFlags;
  p->epSquare = u->epSquare;
  p->reversibleMoves = u->reversibleMoves;
  p->hashKey = u->hashKey;
  p->pawnKey = u->pawnKey;
  p->head--;

  p->pc[fsq] = Pc(side, ftp);
  p->pc[tsq] = NO_PC;
  p->bbCl[side] ^= bbMove;
  p->bbTp[ftp]  ^= bbMove;
  p->pstMg[side] += Data.pstMg[side][ftp][fsq] - Data.pstMg[side][ftp][tsq]; 
  p->pstEg[side] += Data.pstEg[side][ftp][fsq] - Data.pstEg[side][ftp][tsq]; 
  
  // on king move update king location data
  if (ftp == K) p->kingSquare[side] = fsq;

  // capture
  if (ttp != NO_TP) {
    p->pc[tsq] = Pc(Opp(side), ttp);
    p->bbCl[Opp(side)] ^= SqBb(tsq);
    p->bbTp[ttp]       ^= SqBb(tsq);
	p->pcCount[Opp(side)] [ttp]++;
    p->pieceMat[Opp(side)] += Data.matValue[ttp];
	p->phase               += Data.phaseValue[ttp];
    p->pstMg[Opp(side)]    += Data.pstMg[Opp(side)][ttp][tsq];
	p->pstEg[Opp(side)]    += Data.pstEg[Opp(side)][ttp][tsq];
  }

  switch (MoveType(move)) {

  case NORMAL:
    break;

  case CASTLE:
    if (tsq > fsq) {
      fsq += 3;
      tsq -= 1;
    } else {
      fsq -= 4;
      tsq += 1;
    }
    p->pc[tsq] = NO_PC;
    p->pc[fsq] = Pc(side, R);
    p->bbCl[side] ^= SqBb(fsq) | SqBb(tsq);
    p->bbTp[R] ^= SqBb(fsq) | SqBb(tsq);
    p->pstMg[side] += Data.pstMg[side][R][fsq] - Data.pstMg[side][R][tsq];
	p->pstEg[side] += Data.pstEg[side][R][fsq] - Data.pstEg[side][R][tsq];
    break;

  case EP_CAP:
    tsq ^= 8;
    p->pc[tsq] = Pc(Opp(side), P);
    p->bbCl[Opp(side)] ^= SqBb(tsq);
    p->bbTp[P] ^= SqBb(tsq);
	p->pcCount[Opp(side)] [P]++;
	p->phase            += Data.phaseValue[P];
    p->pstMg[Opp(side)] += Data.pstMg[Opp(side)][P][tsq];
	p->pstEg[Opp(side)] += Data.pstEg[Opp(side)][P][tsq];
    break;

  case EP_SET:
    break;

  case N_PROM: case B_PROM: case R_PROM: case Q_PROM:
    p->pc[fsq] = Pc(side, P);
    p->bbTp[P]   ^= SqBb(fsq);
    p->bbTp[ftp] ^= SqBb(fsq);
	p->pcCount[side][P]++;
	p->pcCount[side][ftp]--;
    p->pieceMat[side] -= Data.matValue[ftp];
	p->phase          += Data.phaseValue[P]         - Data.phaseValue[ftp];
    p->pstMg[side]    += Data.pstMg[side][P][fsq] - Data.pstMg[side][ftp][fsq];
	p->pstEg[side]    += Data.pstEg[side][P][fsq] - Data.pstEg[side][ftp][fsq];
    break;
  }
  p->side ^= 1;
}

void sManipulator::UndoNull(sPosition *p, UNDO *u)
{
  p->epSquare = u->epSquare;
  p->hashKey = u->hashKey;
  p->pawnKey = u->pawnKey;
  p->head--;
  p->reversibleMoves--;
  p->side ^= 1;
}
