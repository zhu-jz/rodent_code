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

#include "rodent.h"
#include "hist.h"

void sHistory::OnNewSearch(void)
{
  int i, j;

  for (i = 0; i < 64; i++)
    for (j = 0; j < 64; j++)
      cutoff[i][j] = 100;

  for (i = 0; i < 12; i++)
    for (j = 0; j < 64; j++)
      history[i][j] /= 16;

  for (i = 0; i < MAX_PLY-2; i++) {
    killer[i][0] = killer[i+2][0];
    killer[i][1] = killer[i+2][1];
  }
}

void sHistory::OnNewGame(void)
{
  int i, j;

  for (i = 0; i < 64; i++)
    for (j = 0; j < 64; j++)
      cutoff[i][j] = 100;

  for (i = 0; i < 12; i++)
    for (j = 0; j < 64; j++)
      history[i][j] = 0;

  for (i = 0; i < MAX_PLY-2; i++) {
    killer[i][0] = 0;
    killer[i][1] = 0;
  }
}

void sHistory::Reduce(void)
{
  for (int i = 0; i < 12; i++)
    for (int j = 0; j < 64; j++)
      history[i][j] /= 2;
}

void sHistory::OnGoodMove(sPosition *p, int move, int depth, int ply)
{
  // no history update if a move changes material balance
  if (p->pc[Tsq(move)] != NO_PC 
  || IsProm(move) 
  || MoveType(move) == EP_CAP)
    return;

  // update cutoff table used for reduction decisions
  cutoff [Fsq(move)] [Tsq(move)] += 8; // 8 and 9 work equally well in self-play, 7 and 10 untested

  // update history table used for move sorting
  int hist_change = depth * depth;
  history[p->pc[Fsq(move)]][Tsq(move)] += hist_change;

  // update killer moves
  if (move != killer[ply][0]) {
    killer[ply][1] = killer[ply][0];
    killer[ply][0] = move;
  }
}

int sHistory::GetMoveHistoryValue(int pc, int sq_to) 
{
    int val = history[pc][sq_to];
	if ( val > (1 << 15) ) History.Reduce();
	return val;
}

int sHistory::GetKiller(int ply, int slot) 
{
    return killer[ply][slot];
}

void sHistory::OnMoveReduced(int move) 
{
    cutoff [Fsq(move)] [Tsq(move)] = 51;
}

void sHistory::OnMoveTried(int move) 
{
    cutoff [Fsq(move)] [Tsq(move)] -= 1;
}

int sHistory::MoveIsBad(int move) 
{
    return (cutoff [Fsq(move)] [Tsq(move)] < 50);
}