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
#include "rodent.h"

const int swapVal[7] = {80, 325, 335, 500, 975, 0, 0};

int Swap(sPosition *p, int from, int to)
{
  int score[32];
  U64 bbPieceType;

  int side        = side = Opp(p->side); 
  int type        = TpOnSq(p, from);
  U64 bbAttackers = AttacksTo(p, to);
  U64 bbOcupied   = OccBb(p) ^ SqBb(from);   // clear moving piece
  score[0]        = swapVal[TpOnSq(p, to)];  // set initial gain
  int ply = 1;

  // update attacks through removed piece
  if ( type == P || type == B || type == Q)
      bbAttackers |= (BAttacks(bbOcupied, to) & (p->bbTp[B] | p->bbTp[Q]));
  if ( type == R || type == Q )
      bbAttackers |= (RAttacks(bbOcupied, to) & (p->bbTp[R] | p->bbTp[Q]));
  bbAttackers &= bbOcupied;

  while (bbAttackers & p->bbCl[side]) {
    
	// safeguard against king captures
	if (type == K) {
      score[ply++] = INF;
      break;
    }

	// update score by the material gain of the next capture
	score[ply] = -score[ply - 1] + swapVal[type];
    
	// find the lowest attacker
	for (type = P; type <= K; type++)
      if ((bbPieceType = bbPc(p, side, type) & bbAttackers))
        break;

    // remove attacker we have just found
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

  // now we go down the score table and backpropagate the score
  while (--ply)
    score[ply - 1] = -Max(-score[ply - 1], score[ply]);

  return score[0];
}