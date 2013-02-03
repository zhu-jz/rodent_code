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

#pragma once

enum eTimeData {
	 W_TIME,
	 B_TIME,
	 W_INC,
	 B_INC,
	 TIME,
	 INC,
	 MOVES_TO_GO,
	 MOVE_TIME,
	 MAX_NODES,
	 MAX_DEPTH,
	 FLAG_EASY_MOVE,
	 FLAG_ROOT_FAIL_LOW,
	 FLAG_NO_FIRST_MOVE,
     SIZE_OF_DATA
};

struct sTimer {
private:
	int data[SIZE_OF_DATA]; // various data used to set actual time per move (see eTimeData)
	int startTime;          // when we have begun searching
	int iterationTime;      // when we are allowed to start new iteration
	int moveTime;           // basic time allocated for a move
	int maxMoveTime;        // time allocated for a move in more difficult circumstances
	int minMoveTime;        // time allocated for an easy move
public:
	void Clear(void);
	void SetStartTime();
	void SetMoveTiming(void);
	void SetIterationTiming(void);
	int FinishIteration(void);
	int GetMS(void);
	int GetElapsedTime(void);
	int GetSavedIterationTime(void);
	int IsInfiniteMode(void);
	int TimeHasElapsed(void);
	int GetData(int slot);
	void SetData(int slot, int val);
	void SetSideData(int side);
	void WasteTime(int miliseconds);
};

extern sTimer Timer;