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

#include <stdio.h>
#include "bitboard/bitboard.h"
#include "data.h"
#include "rodent.h"

int Swap(sPosition *p, int from, int to)
{
  int side, ply, score[32];
  U64 bbAttackers, bbOcupied, bbPieceType;

  int type  = TpOnSq(p, from);
  int vType = TpOnSq(p, to);

  bbAttackers = AttacksTo(p, to);

  bbOcupied   = OccBb(p);
  score[0]    = Data.matValue[TpOnSq(p, to)];
  bbOcupied ^= SqBb(from);

  // update attacks through removed piece
  if ( type == P || type == B || type == Q)
      bbAttackers |= (BAttacks(bbOcupied, to) & (p->bbTp[B] | p->bbTp[Q]));
  if ( type == R || type == Q )
      bbAttackers |= (RAttacks(bbOcupied, to) & (p->bbTp[R] | p->bbTp[Q]));
  bbAttackers &= bbOcupied;

  side = Opp(p->side);
  ply = 1;

  while (bbAttackers & p->bbCl[side]) {
    
	// safeguard for king captures
	if (type == K) 
	{
      score[ply++] = INF;
      break;
    }

	score[ply] = -score[ply - 1] + Data.matValue[type];
    
	// find the lowest attacker
	for (type = P; type <= K; type++)
      if ((bbPieceType = bbPc(p, side, type) & bbAttackers))
        break;

    bbOcupied ^= bbPieceType & -bbPieceType;

	// update attacks through removed piece
	if ( type == P || type == B || type == Q)
	   bbAttackers |= (BAttacks(bbOcupied, to) & (p->bbTp[B] | p->bbTp[Q]));
    if ( type == R || type == Q )
	   bbAttackers |= (RAttacks(bbOcupied, to) & (p->bbTp[R] | p->bbTp[Q]));
    bbAttackers &= bbOcupied;

    side ^= 1;
    ply++;
  }

  while (--ply)
    score[ply - 1] = -Max(-score[ply - 1], score[ply]);

  return score[0];
}