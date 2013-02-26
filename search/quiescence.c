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
#include "../selector.h"
#include "../timer.h"
#include "../trans.h"
#include "search.h"
#include "../eval/eval.h"
#include "../bitboard/bitboard.h"

int sSearcher::Quiesce(sPosition *p, int ply, int qDepth, int alpha, int beta, int *pv)
{
  int best, score, move = 0, newPv[MAX_PLY];
  sSelector Selector;
  UNDO undoData[1];

  nodes++;
  IncStat(Q_NODES);
  CheckInput();
  //int flagInCheck    = InCheck(p); // are we in check at the beginning of the search?

  // TRANSPOSITION TABLE READ
  if (TransTable.Retrieve(p->hashKey, &move, &score, alpha, beta, 1, ply))
  return score;
  
  // EARLY EXIT CONDITIONS
  if (flagAbortSearch) return 0;
  if (!qDepth) {
     if (IsRepetition(p)) return 0;
	// if ( !flagInCheck && RecognizeDraw(p) ) return 0;
  }
  
  *pv = 0;

  // safeguard against hitting max ply limit
  if (ply >= MAX_PLY - 1) return Eval.Return(p, alpha, beta);

  best = Eval.Return(p, alpha, beta);
  if (best >= beta) { 
  // CAUSES DIFFERENT NODE COUNTS BETWEEN DEBUG AND RELEASE COMPILE 
  // - probably there's an uninitialized variable in sEvaluator or setboard
  //  TransTable.Store(p->hashKey, 0, score, LOWER, 1, ply);
	  return best;
  }

  if (best > alpha) alpha = best;

  Selector.InitCaptureList(p, move);

  while ( move = Selector.NextCapture() ) {      // on finding next capture...

    // DELTA PRUNING 

	// 1) (cheap) gain promised by this move is unlikely to raise score
	if ( best + Data.deltaValue[ TpOnSq(p, Tsq(move) ) ] < alpha) continue;

	// 2) (expensive) this capture appears to lose material
	if (Selector.BadCapture(p, move)) continue;  
	  
    Manipulator.DoMove(p, move, undoData);
    
	// don't process illegal moves
	if (IllegalPosition(p)) { 
		Manipulator.UndoMove(p, move, undoData); 
		continue; 
	}
    
	score = -Quiesce(p, ply+1, qDepth+1, -beta, -alpha, newPv);
    Manipulator.UndoMove(p, move, undoData);

    if (flagAbortSearch) return 0; // timeout, "stop" command or mispredicted ponder move

	// BETA CUTOFF
	if (score >= beta) {
	   TransTable.Store(p->hashKey, move, score, LOWER, 1, ply);
	   return score;
	}

	// SET NEW SCORE
    if (score > best) {
      best = score;
      if (score > alpha) {
		alpha = score;
        BuildPv(pv, newPv, move);
      }
    }
  }

  // SAVE SEARCH RESULT IN TRANSPOSITION TABLE
  if (*pv) TransTable.Store(p->hashKey, *pv, best, EXACT, 1, ply);
  else     TransTable.Store(p->hashKey,   0, best, UPPER, 1, ply);

  return best;
}