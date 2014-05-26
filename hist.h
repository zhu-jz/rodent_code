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

#pragma once

class sHistory {
private:
     int history[12][64];
	 int cutoff[64][64];
	 int refutation[64][64];
	 int killer[MAX_PLY][2];
	 void ReducePeaks(void);
	 void UpdateCutoff(int move);
	 void UpdateKillers(int move, int ply);
	 void UpdateHistory(sPosition *p, int move, int depth);
public:
	 int MoveChangesMaterialBalance(sPosition *p, int move);
	 void OnGoodMove(sPosition *p, int lastMove, int move, int depth, int ply);
	 void UpdateSortOnly(sPosition *p, int move, int depth, int ply);
	 void OnNewSearch(void);
	 void OnNewGame(void);
	 void OnMoveTried(int move);
	 void OnMoveReduced(int move);
	 int MoveIsBad(int move);
	 void UpdateRefutation(int lastMove, int move);
	 int GetMoveHistoryValue(int pc, int sq_to);
	 int GetKiller(int ply, int slot);
	 int Refutes(int lastMove, int move);
	 int GetRefutation(int lastMove);
};

extern sHistory History;
