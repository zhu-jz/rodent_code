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
#include "move.h"

void sManipulator::DoMove(sPosition *p, int move, UNDO *u)
{
  int side, fsq, tsq, ftp, ttp;

  side = p->side;         // moving side 
  fsq  = Fsq(move);       // start square
  tsq  = Tsq(move);       // target square
  ftp  = TpOnSq(p, fsq);  // moving piece
  ttp  = TpOnSq(p, tsq);  // captured piece

  U64 bbMove = SqBb(fsq) | SqBb(tsq); // optimization from Stockfish

  // save data for undoing a move
  u->ttp = ttp;
  u->castleFlags = p->castleFlags;
  u->epSquare = p->epSquare;
  u->reversibleMoves = p->reversibleMoves;
  u->hashKey = p->hashKey;
  u->pawnKey = p->pawnKey;

  p->repetitionList[p->head++] = p->hashKey;

  // update reversible move counter (zeroing is done on captures and pawn moves)
  p->reversibleMoves++;

  p->hashKey     ^= zobCastle[p->castleFlags];
  p->castleFlags &= castleMask[fsq] & castleMask[tsq];
  p->hashKey     ^= zobCastle[p->castleFlags];

  // clear en passant square
  if (p->epSquare != NO_SQ) {
    p->hashKey ^= zobEp[File(p->epSquare)];
    p->epSquare = NO_SQ;
  }

  // move a piece from start square
  p->pc[fsq]      = NO_PC;
  p->pc[tsq]      = Pc(side, ftp);
  p->hashKey     ^= zobPiece[Pc(side, ftp)][fsq] ^ zobPiece[Pc(side, ftp)][tsq];
  if (ftp == P) {
	  p->reversibleMoves = 0;
	  p->pawnKey ^= zobPiece[Pc(side, ftp)][fsq] ^ zobPiece[Pc(side, ftp)][tsq];
  } 
  p->bbCl[side]  ^= bbMove;
  p->bbTp[ftp]   ^= bbMove;
  p->pstMg[side] += Data.pstMg[side][ftp][tsq] - Data.pstMg[side][ftp][fsq];
  p->pstEg[side] += Data.pstEg[side][ftp][tsq] - Data.pstEg[side][ftp][fsq];

  // on a king move update king location data
  if (ftp == K) p->kingSquare[side] = tsq;
  
  // capture
  if (ttp != NO_TP) {
	p->reversibleMoves = 0;
    p->hashKey ^= zobPiece[Pc(Opp(side), ttp)][tsq];
	if (ttp == P) 
		p->pawnKey     ^= zobPiece[Pc(Opp(side), ttp)][tsq];
    p->bbCl[Opp(side)] ^= SqBb(tsq);
    p->bbTp[ttp]       ^= SqBb(tsq);
	p->pcCount[Opp(side)][ttp]--;
    p->pieceMat[Opp(side)] -= Data.matValue[ttp];
	p->phase               -= Data.phaseValue[ttp]; 
    p->pstMg[Opp(side)]    -= Data.pstMg[Opp(side)][ttp][tsq];
	p->pstEg[Opp(side)]    -= Data.pstEg[Opp(side)][ttp][tsq];
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
    p->pc[fsq]      = NO_PC;
    p->pc[tsq]      = Pc(side, R);
    p->hashKey     ^= zobPiece[Pc(side, R)][fsq] ^ zobPiece[Pc(side, R)][tsq];
    p->bbCl[side]  ^= SqBb(fsq) | SqBb(tsq);
    p->bbTp[R]     ^= SqBb(fsq) | SqBb(tsq);
    p->pstMg[side] += Data.pstMg[side][R][tsq] - Data.pstMg[side][R][fsq];
	p->pstEg[side] += Data.pstEg[side][R][tsq] - Data.pstEg[side][R][fsq];
    break;

  case EP_CAP:
    tsq ^= 8;
    p->pc[tsq]  = NO_PC;
    p->hashKey ^= zobPiece[Pc(Opp(side), P)][tsq];
	p->pawnKey ^= zobPiece[Pc(Opp(side), P)][tsq];
    p->bbCl[Opp(side)] ^= SqBb(tsq);
    p->bbTp[P] ^= SqBb(tsq);
	p->pcCount[Opp(side)][P]--;
	p->phase             -= Data.phaseValue[P];
    p->pstMg[Opp(side)] -= Data.pstMg[Opp(side)][P][tsq];
	p->pstEg[Opp(side)] -= Data.pstEg[Opp(side)][P][tsq];
    break;

  case EP_SET:
    tsq ^= 8;
    if (bbPawnAttacks[side][tsq] & bbPc(p, Opp(side), P)) {
      p->epSquare = tsq;
      p->hashKey ^= zobEp[File(tsq)];
    }
    break;

  // promotion:    (1) add promoted piece and add values associated with it
  case N_PROM:  // (2) remove promoted pawn and substract values associated with it 
  case B_PROM: 
  case R_PROM: 
  case Q_PROM:
    ftp = PromType(move);
    p->pc[tsq]   = Pc(side, ftp);
    p->hashKey  ^= zobPiece[Pc(side, P)][tsq] ^ zobPiece[Pc(side, ftp)][tsq];
	p->pawnKey  ^= zobPiece[Pc(side, P)][tsq];
    p->bbTp[P]  ^= SqBb(tsq);
    p->bbTp[ftp]^= SqBb(tsq);
	p->pcCount[side][ftp]++;
	p->pcCount[side][P]--;
	p->pieceMat[side] += Data.matValue[ftp];
	p->phase          += Data.phaseValue[ftp]       - Data.phaseValue[P];
    p->pstMg[side]    += Data.pstMg[side][ftp][tsq] - Data.pstMg[side][P][tsq];
	p->pstEg[side]    += Data.pstEg[side][ftp][tsq] - Data.pstEg[side][P][tsq];
    break;

  }
  p->side ^= 1;
  p->hashKey ^= SIDE_RANDOM;
}

void sManipulator::DoNull(sPosition *p, UNDO *u)
{
  u->epSquare = p->epSquare;
  u->hashKey  = p->hashKey;
  u->pawnKey  = p->pawnKey;
  p->repetitionList[p->head++] = p->hashKey;
  p->reversibleMoves++;

  if (p->epSquare != NO_SQ) {
    p->hashKey ^= zobEp[File(p->epSquare)];
    p->epSquare = NO_SQ;
  }

  p->side ^= 1;
  p->hashKey ^= SIDE_RANDOM;
}
