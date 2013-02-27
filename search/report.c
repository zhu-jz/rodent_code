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
#include "../rodent.h"
#include "../data.h"
#include "../timer.h"
#include "search.h"

void sSearcher::ClearStats(void)
{
	for (int i=0; i < END_OF_STATS; i++) stat[i] = 0;
}

void sSearcher::IncStat(int slot) 
{
	stat[slot]++;
}

void sSearcher::DisplayStats(void) 
{
	 printf("Fail high ratio : %d percent \n", (stat[FAIL_FIRST] * 100) / Max( 1, stat[FAIL_HIGH]) );
	 printf("Quiescence ratio: %d percent \n", (stat[Q_NODES   ] * 100) / Max(1, nodes) );
}

void sSearcher::DisplayRootInfo(void) 
{
   if (Data.verbose) {
       DisplayDepth();
	   DisplaySpeed();
	}
}

void sSearcher::DisplayCurrmove(int move, int movesTried) 
{
	printf("info currmove ");
    PrintMove(move);
	printf(" currmovenumber %d ", movesTried);
    printf("\n");
	DisplaySpeed();
}

void sSearcher::DisplayDepth(void) 
{
     printf("info depth %d \n", rootDepth / ONE_PLY);
}

void sSearcher::DisplaySettings(void)
{
     printf("info string Searched by Rodent, level %s", Data.currLevel);
	 printf(", style %s\n", Data.currStyle);
}

void sSearcher::DisplayPv(int score, int *pv)
{
  // slowdown on weak levels
  if (Data.elo < MAX_ELO && Data.useWeakening) 
     Timer.WasteTime( (MAX_ELO - Data.elo) / 10 );

  char *type, pv_str[512];
  int time = Timer.GetElapsedTime();
  U32 nps  = GetNps(nodes, time);

  type = "mate";
  if (score < -MAX_EVAL)
    score = (-MATE - score) / 2;
  else if (score > MAX_EVAL)
    score = (MATE - score + 1) / 2;
  else
    type = "cp";
  
  PvToStr(pv, pv_str);

  if (flagProtocol == PROTO_UCI)
  printf("info depth %d time %d nodes %u nps %d score %s %d pv %s\n",
          rootDepth/ONE_PLY, time,   nodes,   nps,   type, score, pv_str);

  if (flagProtocol == PROTO_TXT)
  printf("%2d. %3d.%1d %10u %4d %4d %s\n",
          rootDepth/ONE_PLY, time/1000, (time/100)%10, nodes, nodes / (time+1),  score, pv_str);
}

void sSearcher::DisplaySavedIterationTime(void) 
{
     printf(" info string saving %d ms \n", Timer.GetSavedIterationTime() );
}

void sSearcher::DisplaySpeed(void) 
{
    int time = Timer.GetElapsedTime();
    U32 nps  = GetNps(nodes, time);

    printf("info time %d nodes %u nps %d \n",
                 time,   nodes,   nps );
}

U32 sSearcher::GetNps(int nodes, int time) 
{
    U64 uTime  = (U64)time;
	U64 uNodes = (U64)nodes;

    if (uTime == 0) return 0;

	if (nodes > 100000000) {
	   uNodes *= 10;
	   uNodes /= (uTime / 100);
	}
	else {
      uNodes *= 1000;
	  uNodes /= uTime;
	}

    return (U32)uNodes;
}

void sSearcher::PrintTxtHeader(void)
{
	 printf("---------------------------------------------------------------\n");
	 printf("ply  time      nodes knps  score pv\n");
	 printf("---------------------------------------------------------------\n");
}