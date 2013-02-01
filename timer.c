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
#include "timer.h"
#include "rodent.h"
#include "data.h"

#if defined(_WIN32) || defined(_WIN64)
#  include <windows.h>
#else
#  include <unistd.h>
#  include <sys/time.h>
#endif

// we need macros here, because they can be used 
// to modify both move time and iteration time
#define EASY_TIME(time) ((time*2)/3)
#define HARD_TIME(time) ((time*3)/2)

void sTimer::Clear(void) 
{
  iterationTime = 999999000; // TODO: change it to real max_int
  SetData(MAX_DEPTH, MAX_PLY);
  moveTime = -1;
  maxMoveTime = -1;
  SetData(W_TIME,-1);
  SetData(B_TIME,-1);
  SetData(W_INC, 0);
  SetData(B_INC, 0);
  SetData(MOVE_TIME, 0);
  SetData(MAX_NODES, 0);
  SetData(MOVES_TO_GO, 40);
}

void sTimer::SetStartTime(void) 
{
  startTime = GetMS();
}

void sTimer::SetMoveTiming(void)
{ 
  // user-defined time per move, no tricks available
  if ( data[MOVE_TIME] ) {
	 moveTime    = data[MOVE_TIME];
	 maxMoveTime = moveTime;
	 return;
  }

  if (data[TIME] >= 0) {
     if (data[MOVES_TO_GO] == 1) data[TIME] -= Min(1000, data[TIME] / 10);
     moveTime = ( data[TIME] + data[INC] * ( data[MOVES_TO_GO] - 1)) / data[MOVES_TO_GO];
	 
	 // assign less time per move on extremely short time controls
	 if		 (moveTime <  200) moveTime = (moveTime * 23) / 32;
	 else if (moveTime <  400) moveTime = (moveTime * 26) / 32;
	 else if (moveTime < 1200) moveTime = (moveTime * 29) / 32;	 

	 // while in time trouble, try to save a bit on increment
	 if (moveTime < data[INC] ) moveTime -= ( (data[INC] * 4) / 5);
	 
	 if (moveTime > 2000 ) maxMoveTime = HARD_TIME(moveTime);
	 else                  maxMoveTime = moveTime;

	 minMoveTime = EASY_TIME(moveTime);

     if (moveTime    > data[TIME]) moveTime    = data[TIME];
	 if (maxMoveTime > data[TIME]) maxMoveTime = data[TIME];
	 if (minMoveTime > data[TIME]) minMoveTime = data[TIME];

     // safeguard against a lag
	 moveTime    -= 10;
	 maxMoveTime -= 10;
	 minMoveTime -=  5;

     if (moveTime < 0) moveTime = 0;
	 if (maxMoveTime < moveTime) maxMoveTime = moveTime;
  }
}

void sTimer::SetIterationTiming(void) 
{
	if (moveTime > 0) iterationTime = ( (moveTime * 3) / 4 );
	else              iterationTime = 999999000;

	 // assign less time per itwration on extremely short time controls
	 if		 (iterationTime <  200) iterationTime = (iterationTime * 23) / 32;
	 else if (iterationTime <  400) iterationTime = (iterationTime * 26) / 32;
	 else if (iterationTime < 1200) iterationTime = (iterationTime * 29) / 32;	 

	 // while in time trouble, try to save a bit on increment
	 if (iterationTime < data[INC] ) iterationTime -= ( (data[INC] * 4) / 5);
}

int sTimer::FinishIteration(void) 
{
    if ( data[FLAG_EASY_MOVE] ) 
		return (GetElapsedTime() >= EASY_TIME(iterationTime) && !pondering);
	return (GetElapsedTime() >= iterationTime && !pondering);
}

int sTimer::GetMS(void)
{
#if defined(_WIN32) || defined(_WIN64)
  return GetTickCount();
#else
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

int sTimer::GetElapsedTime(void) 
{
    return (GetMS() - startTime);
}

int sTimer::GetSavedIterationTime(void) 
{
    return ( moveTime - (GetMS() - startTime) );
}

int sTimer::IsInfiniteMode(void) 
{
    return( !(moveTime >= 0) );
}

int sTimer::TimeHasElapsed(void)
{
    if (Data.isAnalyzing) return 0;

	if ( (data[FLAG_ROOT_FAIL_LOW] || data[FLAG_NO_FIRST_MOVE]) 
    &&   moveTime > 2000 )
		return (GetElapsedTime() >= maxMoveTime );

	if (data[FLAG_EASY_MOVE]) 
		return (EASY_TIME(GetElapsedTime() ) >= minMoveTime);

	return (GetElapsedTime() >= moveTime);
}

int sTimer::GetData(int slot) 
{
    return data[slot];
}

void sTimer::SetData(int slot, int val) 
{
    data[slot] = val;
}

void sTimer::SetSideData(int side) 
{
     data[TIME] = side == WHITE ? GetData(W_TIME) : GetData(B_TIME);
     data[INC]  = side == WHITE ? GetData(W_INC)  : GetData(B_INC);
}

void sTimer::WasteTime(int miliseconds) 
{
#if defined(_WIN32) || defined(_WIN64)
    Sleep(miliseconds);
#endif
// TODO: Linux version
}