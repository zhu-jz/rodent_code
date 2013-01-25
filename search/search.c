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
#include <string.h>
#include "../rodent.h"
#include "../data.h"
#include "../bitboard/bitboard.h"
#include "../move/move.h"
#include "../selector.h"
#include "../timer.h"
#include "../trans.h"
#include "../hist.h"
#include "../parser.h"
#include "../learn.h"
#include "../book.h"
#include "search.h"
#include "../eval/eval.h"

void sSearcher::Think(sPosition *p, int *pv)
{
  int flagBookProblem = 0;
  isReporting         = 1;
  nodes               = 0;
  flagAbortSearch     = 0;
  History.OnNewSearch();
  TransTable.ChangeDate();
  Timer.SetStartTime();
  ClearStats();
  pv[0] = Book.GetBookMove(p, 1, &flagBookProblem); 

  if (!pv[0] || !IsLegal(p, pv[0]) || Data.isAnalyzing) {
	   if (flagProtocol == PROTO_TXT) PrintTxtHeader();
      Iterate(p, pv); // TODO: switch search methodologies
      if (Data.panelStyle == PANEL_NORMAL) DisplaySettings();
  }

  DisplayStats();
}

void sSearcher::Iterate(sPosition *p, int *pv) 
{
  int val = 0;
  int curVal;
  int alpha, beta, delta;

  rootList.Init(p);
  int localDepth = Timer.GetData(MAX_DEPTH) * ONE_PLY;
  if (rootList.nOfMoves == 1) localDepth = 4 * ONE_PLY;

  Timer.SetIterationTiming();          // define additional rules for starting next iteration
  Data.InitAsymmetric(p->side);        // set asymmetric eval parameters, dependent on the side to move
  Timer.SetData(FLAG_ROOT_FAIL_LOW, 0);

  for (rootDepth = ONE_PLY; rootDepth <= localDepth; rootDepth+=ONE_PLY) {

	 DisplayRootInfo();
	 delta = Data.aspiration; // TODO: progressive widening loop 
	 alpha = val-delta;
	 beta  = val+delta;

	 //if (rootDepth <= 6 * ONE_PLY) { alpha = -INF; beta = INF; }
    
	// first use aspiration window around the value from the last completed depth
	curVal = SearchRoot(p, 0, alpha, beta, rootDepth, PV_NODE, NO_NULL, 0, pv);
	if (flagAbortSearch) break;

	// if score is outside the window, re-search
	if (curVal >= beta || curVal <= alpha) 
	{
        // fail-low, it might be prudent to assign some more time
        if (curVal < val) Timer.SetData(FLAG_ROOT_FAIL_LOW, 1);

		if (curVal >= beta)  beta  = val +3*delta;
		if (curVal <= alpha) alpha = val -3*delta;

		curVal = SearchRoot(p, 0, alpha, beta, rootDepth, PV_NODE, NO_NULL, 0, pv);
        if (flagAbortSearch) break;

		// the second window
		if (curVal >= beta || curVal <= alpha) 
            curVal = SearchRoot(p, 0, -INF, INF, rootDepth, PV_NODE, NO_NULL, 0, pv);
        if (flagAbortSearch) break;
	}

	// SAVE POSITION LEARNING DATA
	if (Data.useLearning 
	&& !flagAbortSearch
	&& !Data.useWeakening
	&&  p->pieceMat[WHITE] > 2000 
	&&  p->pieceMat[BLACK] > 2000) 
		Learner.WriteLearnData(p->hashKey, rootDepth, curVal);

	// abort root search if we don't expect to finish the next iteration
	if (Timer.FinishIteration() ) {
		if (Data.verbose) DisplaySavedIterationTime();
		break; 
	}

	val = curVal;
  }
}

int sSearcher::VerifyValue(sPosition *p, int depth, int move) 
{
  int val = 0;
  UNDO  undoData[1];  // data required to undo a move
  int pv[MAX_PLY];

  rootList.Init(p);

  Data.isAnalyzing = 1;
  isReporting = 0;
  if (move != 0) Manipulator.DoMove(p, move, undoData);    

  for (rootDepth = ONE_PLY; rootDepth <= depth * ONE_PLY; rootDepth+=ONE_PLY)
      val = SearchRoot(p, 0, -INF, INF, rootDepth, PV_NODE, NO_NULL, 0, pv);

  Data.isAnalyzing = 0;

  if (move != 0) Manipulator.UndoMove(p, move, undoData);

  if (move != 0) return -val;
  return val;
}

int sSearcher::SearchRoot(sPosition *p, int ply, int alpha, int beta, int depth, int nodeType, int wasNull, int lastMove, int *pv)
{
  int best,                     // best value found at this node
	  score,                    // score returned by a search started in this node
	  move,                     // a move we are searching right now
	  depthChange,              // extension/reduction value
	  newDepth,                 // depth of a new search started in this node
	  newPv[MAX_PLY],           // new main line
      flagMoveType;             // move type flag, supplied by NextMove()
  sSelector Selector;           // an object responsible for maintaining move list and picking moves 
    UNDO  undoData[1];          // data required to undo a move

  // NODE INITIALIZATION
  int nullScore      = 0;       // result of a null move search
  int nullRefutation = 0;       // a capture that refuted a null move
  int refutationSq   = NO_SQ;   // target square of that capture
  int movesTried     = 0;       // count of moves that initiated new searches
  int blunderCount   = 0;       // forces the engine to try one of the top moves in weakening mode
  int nodeEval       = INVALID; // we have not called evaluation function at this node yet 
  int flagIsReduced  = 0;       // are we in a reduced search? (guides re-searches)
  int flagCanPrune   = 0;       // can we statically prune in this node
  int flagInCheck    = InCheck(p); // are we in check at the beginning of the search?

  // at root we keep track whether a move has been found; if not, we let the engine
  // search for a bit longer, as this might indicate a fail-high/fail-low
  if (!ply && nodeType == PV_NODE) Timer.SetData(FLAG_NO_FIRST_MOVE, 1);

  // CHECK EXTENSION
  if (flagInCheck) depth += ONE_PLY;
	  // TODO: extend at low no. of replies (requires out of check / legal move generator)
  
  // QUIESCENCE SEARCH ENTRY POINT
  if ( depth < ONE_PLY ) return Quiesce(p, ply, 0, alpha, beta, pv);

  nodes++;
  CheckInput();
  
  // EARLY EXIT / DRAW CONDITIONS
  if ( flagAbortSearch )                         return 0;
  if ( IsRepetition(p) && ply )                  return 0;
  if ( DrawBy50Moves(p) )                        return 0;
  if ( !flagInCheck && ply && RecognizeDraw(p) ) return 0;
  
  if (ply) *pv = 0;
  move = 0;

  // MATE DISTANCE PRUNING
   alpha = Max(-MATE+ply, alpha);
   beta  = Min( MATE-ply, beta);
   if (alpha >= beta)
       return alpha;

  // TRANSPOSITION TABLE READ
  // get transposition table score or at least get a move for sorting purposes

  if (TransTable.Retrieve(p->hashKey, &move, &score, alpha, beta, depth, ply))
     return score;
  
  // safeguard against hitting max ply limit
  if (ply >= MAX_PLY - 1) return Eval.Return(p, alpha, beta);

  // INTERNAL ITERATIVE DEEPENING - we try to get a hash move to improve move ordering
  if (nodeType == PV_NODE && !move && depth >= 4*ONE_PLY && !flagInCheck ) {
	  Search(p, ply, alpha, beta, depth-2*ONE_PLY, PV_NODE, NO_NULL, 0, newPv);
	  TransTable.RetrieveMove(p->hashKey, &move);
  }

  // CREATE MOVE LIST AND START SEARCHING
  best = -INF;
  Selector.InitMoves(p, move, ply);

  // LOOP THROUGH THE MOVE LIST
  while ( move = Selector.NextMove(refutationSq, &flagMoveType) ) 
  {
	 // MAKE A MOVE
	 Manipulator.DoMove(p, move, undoData);    
	
	 // UNDO ILLEGAL MOVES
	 if (IllegalPosition(p)) { 
	 	 Manipulator.UndoMove(p, move, undoData); 
		 continue; 
	 }

	 movesTried++;                     // increase legal move count
	 flagIsReduced  = 0;               // this move has not been reduced (yet)
	 depthChange    = 0;               // no depth modification so far
	 History.OnMoveTried(move);

	 // EXTENSIONS might be placed here

	 newDepth = depth - ONE_PLY + depthChange; // determine new depth

	 // PRINCIPAL VARIATION SEARCH
	 if (best == -INF )
       score =   -Search(p, ply+1, -beta,    -alpha, newDepth, NEW_NODE(nodeType), NO_NULL, move, newPv);
     else {
       score =   -Search(p, ply+1, -alpha-1, -alpha, newDepth, CUT_NODE, NO_NULL, move, newPv);
       if (!flagAbortSearch && score > alpha && score < beta)
         score = -Search(p, ply+1, -beta,    -alpha, newDepth, PV_NODE, NO_NULL, move, newPv);
     }

     Manipulator.UndoMove(p, move, undoData);

	 if (!ply && nodeType == PV_NODE) Timer.SetData(FLAG_NO_FIRST_MOVE, 0);

	 if (flagAbortSearch) return 0; // timeout, "stop" command or mispredicted ponder move

     // BETA CUTOFF
	 if (score >= beta) {
		 IncStat(FAIL_HIGH);
		 if (movesTried == 1) IncStat(FAIL_FIRST);
         History.OnGoodMove(p, move, depth / ONE_PLY, ply);
         TransTable.Store(p->hashKey, move, score, LOWER, depth, ply);
         return score;
     }

	 // SCORE CHANGE
     if (score > best) {
         best = score;
         if (score > alpha) {
            alpha = score;
            if (nodeType == PV_NODE) BuildPv(pv, newPv, move);
		    if (!ply) DisplayPv(score, pv);
         }
      }
   }
  
   // RETURN CORRECT CHECKMATE/STALEMATE SCORE
   if (best == -INF) return flagInCheck ? -MATE + ply : 0;

   // SAVE SEARCH RESULT IN TRANSPOSITION TABLE
   if (*pv) {
 	 History.OnGoodMove(p, *pv, depth / ONE_PLY, ply);
     TransTable.Store(p->hashKey, *pv, best, EXACT, depth, ply);
   } else
     TransTable.Store(p->hashKey, 0, best, UPPER, depth, ply);

   return best;
}

int sSearcher::Search(sPosition *p, int ply, int alpha, int beta, int depth, int nodeType, int wasNull, int lastMove, int *pv)
{
  int best,                     // best value found at this node
	  score,                    // score returned by a search started in this node
	  move,                     // a move we are searching right now
	  depthChange,              // extension/reduction value
	  newDepth,                 // depth of a new search started in this node
	  newPv[MAX_PLY],           // new main line
      flagMoveType;             // move type flag, supplied by NextMove()
  sSelector Selector;           // an object responsible for maintaining move list and picking moves 
    UNDO  undoData[1];          // data required to undo a move

  // NODE INITIALIZATION
  int nullScore      = 0;       // result of a null move search
  int nullRefutation = 0;       // a capture that refuted a null move
  int refutationSq   = NO_SQ;   // target square of that capture
  int movesTried     = 0;       // count of moves that initiated new searches
  int blunderCount   = 0;       // forces the engine to try one of the top moves in weakening mode
  int nodeEval       = INVALID; // we have not called evaluation function at this node yet 
  int flagIsReduced  = 0;       // are we in a reduced search? (guides re-searches)
  int flagCanPrune   = 0;       // can we statically prune in this node
  int flagInCheck    = InCheck(p); // are we in check at the beginning of the search?

  // CHECK EXTENSION
  if (flagInCheck) depth += ONE_PLY;
	  // TODO: extend at low no. of replies (requires out of check / legal move generator)

  // EARLY EXIT / DRAW CONDITIONS
  if ( flagAbortSearch )                  return 0;
  if ( IsRepetition(p) )                  return 0;
  if ( DrawBy50Moves(p) )                 return 0;
  if ( !flagInCheck && RecognizeDraw(p) ) return 0;
  
  // QUIESCENCE SEARCH ENTRY POINT
  if ( depth < ONE_PLY ) return Quiesce(p, ply, 0, alpha, beta, pv);

  nodes++;
  CheckInput();

  // REUSING LEARNED DATA ABOUT SCORE OF SPECIFIC POSITIONS
  if ( Data.useLearning 
  && !Data.useWeakening
  && ply == 2 ) // reading at bigger depths may hinder exploring alternatives to mainline
  {
	  int learnVal = Learner.ReadLearnData(p->hashKey, depth);
	  if (learnVal != INVALID) return learnVal;
  }
  
  if (ply) *pv = 0;
  move = 0;

  // MATE DISTANCE PRUNING
   alpha = Max(-MATE+ply, alpha);
   beta  = Min( MATE-ply, beta);
   if (alpha >= beta)
       return alpha;

  // TRANSPOSITION TABLE READ
  // get transposition table score or at least get a move for sorting purposes

  if (TransTable.Retrieve(p->hashKey, &move, &score, alpha, beta, depth, ply))
     return score;
  
  // safeguard against hitting max ply limit
  if (ply >= MAX_PLY - 1) return Eval.Return(p, alpha, beta);

  // EVAL PRUNING (inspired by DiscoCheck by Lucas Braesch)
  if (depth <= 3*ONE_PLY
  && nodeType != PV_NODE
  &&  beta < MAX_EVAL
  && !flagInCheck) 
  {
     if (nodeEval == INVALID) nodeEval = Eval.ReturnFast(p);
	 int evalMargin = 40 * depth;
	 if ( nodeEval - evalMargin >= beta )
		return nodeEval - evalMargin;
  }

  // QUIESCENCE NULL MOVE (idea from CCC post of Vincent Diepeveen)
  if (Data.useNull
  && nodeType != PV_NODE
  && depth <= Data.minimalNullDepth
  && !wasNull
  && beta < MAX_EVAL
  && !flagInCheck
  && p->pieceMat[p->side] > Data.matValue[N])
  {
    Manipulator.DoNull(p, undoData);
    score = -Quiesce(p, ply, 0, -beta, -beta+1, pv);
	Manipulator.UndoNull(p, undoData);

	if (score >= beta) return score;
  }

  // NULL MOVE - we allow opponent to move twice in a row; if he cannot beat
  // beta this way, then we assume that there is no need to search any further.

  if (Data.useNull
  && nodeType != PV_NODE
  && depth > Data.minimalNullDepth
  && !wasNull
  && beta < MAX_EVAL
  && !flagInCheck
  && p->pieceMat[p->side] > Data.matValue[N]) 
  {
    if ( beta <= Eval.Return(p, alpha, beta) )
	{
      Manipulator.DoNull(p, undoData);
	  newDepth = SetNullDepth(depth);                             // ALL_NODE
      nullScore = -Search(p, ply + 1, -beta, -beta + 1, newDepth, NEW_NODE(nodeType), WAS_NULL, 0, newPv);

	  // Extract refutation of a null move from transposition table; 
	  // most of the time it will be capture and we will use this information
	  // to sort an evasion much higher than other quiet moves.
	  TransTable.Retrieve(p->hashKey, &nullRefutation, &score, alpha, beta, depth, ply);
	  
	  // get target square of null move refutation to sort escape moves a bit higher
	  if (nullRefutation != 0 ) refutationSq = Tsq(nullRefutation);
      Manipulator.UndoNull(p, undoData);

      if (flagAbortSearch) return 0; // timeout, "stop" command or mispredicted ponder move

	  // verify null move in the endgame
	  if (nullScore >= beta && p->pieceMat[p->side] < 1600 ) 
          nullScore = Search(p, ply, alpha, beta, newDepth-ONE_PLY, CUT_NODE, NO_NULL, 0, newPv);

      if (nullScore >= beta) {
         if (!move) // we don't want to overwrite real entries, as they are more useful for move ordering 
			TransTable.Store(p->hashKey, 0, nullScore, LOWER, depth, ply);
		
		 return Eval.Normalize(nullScore, MAX_EVAL); // checkmate from null move search isn't reliable
	  }
    }
  } // end of null move code 

  // INTERNAL ITERATIVE DEEPENING - we try to get a hash move to improve move ordering
  if (nodeType == PV_NODE && !move && depth >= 4*ONE_PLY && !flagInCheck ) {
	  Search(p, ply, alpha, beta, depth-2*ONE_PLY, PV_NODE, NO_NULL, 0, newPv);
	  TransTable.RetrieveMove(p->hashKey, &move);
  }

  // TODO: if passes gauntlet test, merge with the iid code, using ? operator
  /*
  if (nodeType == CUT_NODE && !move && depth >= 8*ONE_PLY && !flagInCheck ) {
	  if (Eval.ReturnFast(p)+50 >= beta) { 
	     Search(p, ply, alpha, beta, depth/2, CUT_NODE, NO_NULL, 0, newPv);
	     TransTable.RetrieveMove(p->hashKey, &move);
	  }
  }
  /**/

  // SET FUTILITY PRUNING FLAG
  if ( depth < Data.futilityDepth * ONE_PLY  // we are sufficiently close to the leaf
  && beta < MAX_EVAL                         // we're not checkmating
  && alpha > -MAX_EVAL                       // we're not being checkmated
  && nodeType != PV_NODE                     // we're not in a pv node
  && !flagInCheck )                          // we're not in check
  {
     if (nodeEval == INVALID) nodeEval = Eval.ReturnFast(p);
     nodeEval = TransTable.RefineScore( p->hashKey, nodeEval );

     // this node looks bad enough, so we may apply futility pruning
     if ( (nodeEval + SetFutilityMargin(depth) ) < beta ) flagCanPrune = 1;
  }

  // CREATE MOVE LIST AND START SEARCHING
  best = -INF;
  Selector.InitMoves(p, move, ply);

  // LOOP THROUGH THE MOVE LIST
  while ( move = Selector.NextMove(refutationSq, &flagMoveType) ) 
  {
	 // MAKE A MOVE
	 Manipulator.DoMove(p, move, undoData);    
	
	 // UNDO ILLEGAL MOVES
	 if (IllegalPosition(p)) { 
	 	 Manipulator.UndoMove(p, move, undoData); 
		 continue; 
	 }

     // MAKE RANDOM BLUNDERS IN WEAKENING MODE
     if (Blunder(p, ply, depth, flagMoveType, move, lastMove, flagInCheck)
     && (movesTried > 1  || blunderCount < 2 ) ) {
	     blunderCount++;
	     Manipulator.UndoMove(p, move, undoData); 
	     continue; 
     }

	 movesTried++;                     // increase legal move count
	 flagIsReduced  = 0;               // this move has not been reduced (yet)
	 depthChange    = 0;               // no depth modification so far
	 History.OnMoveTried(move);

	 // EXTENSIONS might be placed here

	 // FUTILITY PRUNING 
	 if ( flagCanPrune                 // flag is set
	 && !depthChange                   // we have not extended this move
	 && IsMoveOrdinary(flagMoveType)   // we are considering ordinary move
	 && movesTried > 1                 // we have found at least one legal move
	 ) {
        Manipulator.UndoMove(p, move, undoData);
        continue;
	 }

	 // LATE MOVE PRUNING near the leaves (2012-04-02: two-tier approach)
	 if (nodeType != PV_NODE            // we're not in a pv node
     &&  depth <= Data.minimalLmrDepth  // we are near the leaf
	 &&  depthChange == 0               // we have not extended
	 &&  !flagInCheck                   // we're not escaping from check
     &&  !InCheck(p)                    // we're not giving check	
	 &&  movesTried > 12                // move is sufficiently down the list
	 // adding !flagMoveType (= not pruning bad captures) or history restiction is worse
	 ) {
 		 if ( movesTried > 20 ) 
		    { Manipulator.UndoMove(p, move, undoData); continue; }
		 if ( History.MoveIsBad(move) ) 
		    { Manipulator.UndoMove(p, move, undoData); continue; }
	 }

	 // LATE MOVE REDUCTION (lmr) - we reduce a move if:
	 if  ( nodeType != PV_NODE          // we're not in a pv node
     &&  depth > Data.minimalLmrDepth   // we have some depth left
	 &&  depthChange == 0               // it has not been extended
	 &&  movesTried > Data.moveIsLate   // it is sufficiently down the move list
	 &&  !flagInCheck                   // we're not escaping a check
     &&  !InCheck(p)                    // we're not giving check
	 &&  History.MoveIsBad(move)        // current move has bad history score
	 ) {
		 // big reduction of a quiet move (hash and killer moves excluded)
		 if ( IsMoveOrdinary(flagMoveType) ) {                 
		    depthChange -= ( SetLmrDepth(move,movesTried) );
		    History.OnMoveReduced(move);
 	        flagIsReduced = 1;
		 }
		
		 // marginal reduction of bad captures
		 if (flagMoveType == FLAG_BAD_CAPTURE) {
            depthChange -= HALF_PLY;
 	        flagIsReduced = 1;
	     }
		
	   // strangely, history restriction from main lmr loop is beneficial for bad captures)
	 } // end of late move reduction code

	 newDepth = depth - ONE_PLY + depthChange; // determine new depth

     re_search: // we come back here if a reduced search has not failed low

	 // PRINCIPAL VARIATION SEARCH
	 if (best == -INF )
       score =   -Search(p, ply+1, -beta,    -alpha, newDepth, NEW_NODE(nodeType), NO_NULL, move, newPv);
     else { 
       score =   -Search(p, ply+1, -alpha-1, -alpha, newDepth, CUT_NODE, NO_NULL, move, newPv);
       if (!flagAbortSearch && score > alpha && score < beta)
         score = -Search(p, ply+1, -beta,    -alpha, newDepth, PV_NODE, NO_NULL, move, newPv);
     }

     // FALLBACK TO NORMAL SEARCH IF REDUCED MOVE SEEMS GOOD
	 if (flagIsReduced && score > alpha) {
        flagIsReduced = 0;          // flag this search as unreduced         
        newDepth -= depthChange;    // undo reduction
	    nodeType = CUT_NODE;        // the chances are that unreduced search will fail high as well
	    goto re_search;             // repeat search
	 }

     Manipulator.UndoMove(p, move, undoData);

	 if (flagAbortSearch) return 0; // timeout, "stop" command or mispredicted ponder move

     // BETA CUTOFF
	 if (score >= beta) {
		 IncStat(FAIL_HIGH);
		 if (movesTried == 1) IncStat(FAIL_FIRST);
         History.OnGoodMove(p, move, depth / ONE_PLY, ply);
         TransTable.Store(p->hashKey, move, score, LOWER, depth, ply);
         return score;
     }

	 // SCORE CHANGE
     if (score > best) {
         best = score;
         if (score > alpha) {
            alpha = score;
            if (nodeType == PV_NODE) BuildPv(pv, newPv, move);
		    if (!ply) DisplayPv(score, pv);
         }
      }
   }
  
   // RETURN CORRECT CHECKMATE/STALEMATE SCORE
   if (best == -INF) return flagInCheck ? -MATE + ply : 0;

   // SAVE SEARCH RESULT IN TRANSPOSITION TABLE
   if (*pv) {
 	 History.OnGoodMove(p, *pv, depth / ONE_PLY, ply);
     TransTable.Store(p->hashKey, *pv, best, EXACT, depth, ply);
   } else
     TransTable.Store(p->hashKey, 0, best, UPPER, depth, ply);

   return best;
}

// determine the amount a null move is reduced

int sSearcher::SetNullDepth(int depth) 
{
    int newDepth = depth - 3*ONE_PLY;
    newDepth -= (newDepth / 4);  // newDepth -= newDepth / 3 is comparable
    return newDepth;
}

int sSearcher::SetFutilityMargin(int depth) 
{
	return Data.futilityBase + depth * Data.futilityStep;
}

int sSearcher::SetLmrDepth(int move, int movesTried) 
{
    int depthChange = ONE_PLY + QUARTER_PLY;

	// reductions for even later moves get fractionally bigger
	depthChange += QUARTER_PLY * (movesTried > Data.moveIsLate + Data.lmrStep);
	depthChange += QUARTER_PLY * (movesTried > Data.moveIsLate + 2*Data.lmrStep);
	depthChange += QUARTER_PLY * (movesTried > Data.moveIsLate + 3*Data.lmrStep);
	depthChange += QUARTER_PLY * (movesTried > Data.moveIsLate + 4*Data.lmrStep);

    return depthChange;
}

int sSearcher::IsRepetition(sPosition *p)
{
    for (int i = 4; i <= p->reversibleMoves; i += 2)
        if (p->hashKey == p->repetitionList[p->head - i])
           return 1;
    return 0;
}

int sSearcher::DrawBy50Moves(sPosition *p) 
{
    return( p->reversibleMoves > 100);
}

int sSearcher::IsMoveOrdinary(int flagMoveType)
{
    return (!flagMoveType || flagMoveType == FLAG_NULL_EVASION);
}

void sSearcher::CheckInput(void)
{
    char command[80];

    if (Data.verbose) {
        // report search speed every now and then
        if (!( nodes % 500000) ) DisplaySpeed();
    }

    if (nodes & 4095 || rootDepth == ONE_PLY) return;

    if (InputAvailable()) {
       Parser.ReadLine(command, sizeof(command));
       if (strcmp(command, "stop") == 0)
          flagAbortSearch = 1;
       else if (strcmp(command, "ponderhit") == 0)
          pondering = 0;
    }
  
#ifdef FAST_TUNING
	if (nodes > FAST_TUNING) 
	   flagAbortSearch = 1;
#endif

	// node limit exceeded
	if ( Timer.GetData(MAX_NODES) 
	&& nodes > Timer.GetData(MAX_NODES)) 
	   flagAbortSearch = 1;

    // timeout
    if ( !pondering 
    && !Timer.IsInfiniteMode()
    && Timer.TimeHasElapsed() )
       flagAbortSearch = 1;
}
