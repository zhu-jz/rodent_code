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
#include "../data.h"
#include "search.h"

/*
1850 internal -> 2000 CCRL 
???? internal -> 2200 CCRL
*/

int sSearcher::Blunder(sPosition *p, int ply, int depth, int flag, int move, int lastMove, int flagInCheck) 
{
	// we're playing the best game we can, so don't bother with weakening
	// (speed optimization)
	if (Data.elo == MAX_ELO || !Data.useWeakening ) return 0;
	
    // Weaker levels progressively use more and more heuristics 
	// that disallow forgetting about certain moves:

	// try to capture the last piece that moved
	if (Tsq(move) == Tsq(lastMove) && Data.elo > 999) return 0;

	// don't miss captures by a pawn
	if ( p->pc[Tsq(move)] == P 
	&& File(Tsq(move)) != File(Fsq(move)) 
	&& Data.elo > 1199) return 0; 

	// don't miss short captures
	if ( Data.distance[Fsq(move)][Tsq(move)] > 9
	&& flag > FLAG_KILLER_MOVE 
	&& flag != FLAG_HASH_MOVE 
	&& Data.elo > 1299 ) return 0;

    // don't miss queen promotions
	if ( MoveType(move) == Q_PROM && Data.elo > 1399) return 0;

	// don't miss check evasions
	if (flagInCheck && Data.elo > 1499) return 0;

	// Stronger levels progressively push blunders away from the root
	// ( extra randomness is used to scale this property as smoothly as possible)

	if ( (Data.elo > 1599 && ply < 3)
	||   (Data.elo > 1899 && ply < 4) 
	||   (Data.elo > 2299 && ply < 5) ) return 0;

    if (Data.elo > 1300 && Data.elo <= 1600 && ply < 3)  {
		int saveRate = p->hashKey % 300;
		if (saveRate > 1600 - Data.elo) return 0;
	}

    if (Data.elo > 1600 && Data.elo <= 1900 && ply < 4 ) {
        int saveRate = p->hashKey % 300;
		if (saveRate > 1900 - Data.elo) return 0;
	}

    if (Data.elo > 1900 && Data.elo <= 2300 && ply < 5 ) {
        int saveRate = p->hashKey % 300;
		if (saveRate > 2300 - Data.elo) return 0;
	}

	int blunderRate = p->hashKey % Data.elo;

	if (flag == FLAG_KILLER_MOVE 
	|| flag == FLAG_HASH_MOVE) {
       blunderRate *= 2;
	   blunderRate *= Data.elo;
	   blunderRate /= MAX_ELO;
	}
	
	// calculate blunder probability (less blunders near the root in deeper search)
	if ( blunderRate + depth < MAX_ELO - Data.elo) return 1;

	return 0;
}