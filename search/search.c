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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../rodent.h"
#include "../data.h"
#include "../bitboard/bitboard.h"
#include "../timer.h"
#include "../trans.h"
#include "../hist.h"
#include "../parser.h"
#include "../learn.h"
#include "../book.h"
#include "search.h"
#include "../eval/eval.h"

static const int moveCountLimit[24] = {0, 0, 0, 0, 4, 4, 4, 4, 7, 7, 7, 7, 12, 12, 12, 12, 19, 19, 19, 19, 28, 28, 28, 28};

void sSearcher::Init(void)
{
   aspiration       = 30;
   futilityDepth    = 4;   // 5 is worse
   minimalLmrDepth  = 2 * ONE_PLY; // 3 is worse
   minimalNullDepth = 2 * ONE_PLY; // 3 is worse

   // set null move reduction depth
   for(int depth = 0; depth < MAX_PLY * ONE_PLY; depth ++) 
      nullDepth[depth] = depth - 3*ONE_PLY - (depth - 3*ONE_PLY) / 4;

   // set futility margin
   for(int depth = 0; depth < 10 * ONE_PLY; depth ++) 
      futilityMargin[depth] = 100 + depth * 20;

   // set late move reduction depth using modified Stockfish formula
   for(int depth = 0; depth < MAX_PLY * ONE_PLY; depth ++)
      for(int moves = 0; moves < MAX_PLY * ONE_PLY; moves ++) {
         lmrSize[0][depth][moves] = 4 * (0.33 + log((double) (depth/ONE_PLY)) * log((double) (moves)) / 2.25); // all node
         lmrSize[1][depth][moves] = 4 * (log((double) (depth/ONE_PLY)) * log((double) (moves)) / 3.5 );        // pv node
         lmrSize[2][depth][moves] = 4 * (0.33 + log((double) (depth/ONE_PLY)) * log((double) (moves)) / 2.25); // cut node

         for (int node = 0; node <= 2; node++) {
            if (lmrSize[node][depth][moves] > 2 * ONE_PLY)
               lmrSize[node][depth][moves] += ONE_PLY / 2;
            else if (lmrSize[node][depth][moves] > 1 * ONE_PLY)
               lmrSize[node][depth][moves] += ONE_PLY / 4;

			if ( lmrSize[node][depth][moves] > depth - ONE_PLY)
				lmrSize[node][depth][moves] = depth - ONE_PLY;
         }
      }
}

void sSearcher::Think(sPosition *p, int *pv)
{
   nodesPerBranch = 0;
   bestMove = 0;
   int flagBookProblem = 0;
   isReporting         = 1;
   nodes               = 0;
   flagAbortSearch     = 0;
   History.OnNewSearch();
   TransTable.ChangeDate();
   Timer.SetStartTime();
   ClearStats();
   pv[0] = 0; // for tests where book move is disabled
   if (Data.useBook) 
      pv[0] = Book.GetBookMove(p, 1, &flagBookProblem); 

   if (!pv[0] || !IsLegal(p, pv[0]) || Data.isAnalyzing) {
      if (flagProtocol == PROTO_TXT) PrintTxtHeader();
      Iterate(p, pv);
      if (Data.panelStyle == PANEL_NORMAL) DisplaySettings();
   }

   DisplayStats();
}

void sSearcher::Iterate(sPosition *p, int *pv) 
{
   int val = 0;
   int curVal, alpha, beta, delta;
   rootSide = p->side;
   Data.InitAsymmetric(p->side);          // set asymmetric eval parameters, dependent on the side to move
   rootList.Init(p);                      // create sorted root move list (using quiescence search scores)
   int localDepth = Timer.GetData(MAX_DEPTH) * ONE_PLY;
   if (rootList.nOfMoves == 1) localDepth = 4 * ONE_PLY; // single reply
   Timer.SetIterationTiming();            // define additional rules for starting next iteration
   Timer.SetData(FLAG_ROOT_FAIL_LOW, 0);  // we haven't failed low yet

   // check whether of the moves is potentially easy
   Timer.SetData(FLAG_EASY_MOVE,     1);
   for(int i = 0; i < rootList.nOfMoves; i ++) {
      if (rootList.moves[i] != rootList.bestMove
      && rootList.value[i] > rootList.bestVal - 220) {
         Timer.SetData(FLAG_EASY_MOVE,     0);
         break;
      }
   }

   for (rootDepth = ONE_PLY; rootDepth <= localDepth; rootDepth+=ONE_PLY) {

      DisplayRootInfo();
      delta = aspiration;

      if (rootDepth <= 6 * ONE_PLY) { alpha = -INF;      beta = INF;       }
      else                          { alpha = val-delta; beta = val+delta; }
    
      // first use aspiration window around the value from the last completed depth
      curVal = SearchRoot(p, alpha, beta, rootDepth, pv);
      bestMove = pv[0];
      if (flagAbortSearch) break;

      // if score is outside the window, re-search
      if (curVal >= beta || curVal <= alpha) {

         // fail-low, it might be prudent to assign some more time
         if (curVal < val) Timer.OnRootFailLow();
         if (curVal >= beta)  beta  = val +3*delta;
         if (curVal <= alpha) alpha = val -3*delta;

         curVal = SearchRoot(p, alpha, beta, rootDepth, pv);
         bestMove = pv[0];
         if (flagAbortSearch) break;

         // the second window
         if (curVal >= beta || curVal <= alpha) 
            curVal = SearchRoot(p, -INF, INF, rootDepth, pv);
            bestMove = pv[0];
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
         if (Data.verbose) {
            DisplaySavedIterationTime();
            if (Timer.GetData(FLAG_EASY_MOVE) ) printf("info string This iteration was easy!\n");
         }
         break; 
      }

      val = curVal;
   }
}

int sSearcher::SearchRoot(sPosition *p, int alpha, int beta, int depth, int *pv)
{
  int best,                     // best value found at this node
	  score,                    // score returned by a search started in this node
	  scoreChange = 0,          // has root move changed?
	  move,                     // a move we are searching right now
	  depthChange,              // extension/reduction value
	  newDepth,                 // depth of a new search started in this node
	  newPv[MAX_PLY];           // new main line
    UNDO  undoData[1];          // data required to undo a move

  // NODE INITIALIZATION
  int movesTried     = 0;       // count of moves that initiated new searches
  int flagInCheck    = InCheck(p); // are we in check at the beginning of the search?

  // at root we keep track whether a move has been found; if not, we let the engine
  // search for a bit longer, as this might indicate a fail-high/fail-low
  Timer.SetData(FLAG_NO_FIRST_MOVE, 1);

  // CHECK EXTENSION (no QS entry later as SearchRoot is called with depth >= 1 ply)
  if (flagInCheck) depth += ONE_PLY;

  nodes++;
  CheckInput();
  
  // EARLY EXIT / DRAW CONDITIONS
  if ( flagAbortSearch )                         return 0;
  if ( IsRepetition(p) && 0 )                    return 0;
  if ( DrawBy50Moves(p) )                        return 0;
  if ( !flagInCheck && 0   && RecognizeDraw(p) ) return 0;
  
  if (0) *pv = 0;
  move = 0;

  // MATE DISTANCE PRUNING
   alpha = Max(-MATE, alpha);
   beta  = Min( MATE, beta);
   if (alpha >= beta) return alpha;

  // TRANSPOSITION TABLE READ (at root we only get a move for sorting purposes)
  TransTable.Retrieve(p->hashKey, &move, &score, alpha, beta, depth, 0);

  // INTERNAL ITERATIVE DEEPENING - we try to get a hash move to improve move ordering
  if (!move && depth >= 4*ONE_PLY && !flagInCheck ) {
	  Search(p, 0, alpha, beta, depth-2*ONE_PLY, PV_NODE, NO_NULL, 0, 0, newPv);
	  TransTable.RetrieveMove(p->hashKey, &move);
  }

  // CREATE MOVE LIST AND START SEARCHING
  best = -INF;
  rootList.ClearUsed(bestMove);

  // LOOP THROUGH ROOT MOVE LIST (which supplies legal moves only)
  while( move = rootList.GetNextMove() ) {
  
	 // MAKE A MOVE
	 Manipulator.DoMove(p, move, undoData);   
	 nodesPerBranch = 0;
	
	 movesTried++;    // increase legal move count
	 if (Data.verbose && !pondering && depth > 6 * ONE_PLY) DisplayCurrmove(move, movesTried);
	 depthChange = 0; // no depth modification so far
	 History.OnMoveTried(move);

	 // EXTENSIONS might be placed here

	 newDepth = depth - ONE_PLY + depthChange; // determine new depth

	 // PRINCIPAL VARIATION SEARCH
	 if (best == -INF )
       score =   -Search(p, 1, -beta,    -alpha, newDepth, NEW_NODE(PV_NODE), NO_NULL, move, 0, newPv);
     else {
       score =   -Search(p, 1, -alpha-1, -alpha, newDepth, CUT_NODE, NO_NULL, move, 0, newPv);
       if (!flagAbortSearch && score > alpha && score < beta)
         score = -Search(p, 1, -beta,    -alpha, newDepth, PV_NODE, NO_NULL, move, 0, newPv);
     }

     Manipulator.UndoMove(p, move, undoData);
	 rootList.ScoreLastMove(move, nodesPerBranch);

	 Timer.SetData(FLAG_NO_FIRST_MOVE, 0);

	 if (flagAbortSearch) return 0; // timeout, "stop" command or mispredicted ponder move

     // BETA CUTOFF
	 if (score >= beta) {
		 if (movesTried > 1 && depth > 2*ONE_PLY) Timer.SetData(FLAG_EASY_MOVE, 0);
		 IncStat(FAIL_HIGH);
		 if (movesTried == 1) IncStat(FAIL_FIRST);
         History.OnGoodMove(p, 0, move, depth / ONE_PLY, 0);
         TransTable.Store(p->hashKey, move, score, LOWER, depth, 0);
         return score;
     }

	 // SCORE CHANGE
     if (score > best) {
         best = score;
		 if (best != -INF) scoreChange++;
		 if (movesTried > 1 && depth > 2*ONE_PLY) Timer.SetData(FLAG_EASY_MOVE, 0);
         if (score > alpha) {
            alpha = score;
            BuildPv(pv, newPv, move);
		    DisplayPv(score, pv);
         }
      }
   }
  
   // RETURN CORRECT CHECKMATE/STALEMATE SCORE
   if (best == -INF) return flagInCheck ? -MATE : 0;

   // SAVE SEARCH RESULT IN TRANSPOSITION TABLE
   if (*pv) {
 	 History.OnGoodMove(p, 0, *pv, depth / ONE_PLY, 0);
     TransTable.Store(p->hashKey, *pv, best, EXACT, depth, 0);
   } else
     TransTable.Store(p->hashKey, 0, best, UPPER, depth, 0);

   if (!scoreChange) Timer.OnOldRootMove();
   else              Timer.OnNewRootMove();
   return best;
}

int sSearcher::Search(sPosition *p, int ply, int alpha, int beta, int depth, int nodeType, int wasNull, int lastMove, int contMove, int *pv)
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
  int fullNodeEval   = INVALID;
  int flagIsReduced  = 0;       // are we in a reduced search? (guides re-searches)
  int flagFutility   = 0;       // can we apply futility pruning in this node
  int flagInCheck    = InCheck(p); // are we in check at the beginning of the search?
  int normalMoveCnt  = 0;       // counter used to delay futility pruning initialization

  // EARLY EXIT / DRAW CONDITIONS
  if ( flagAbortSearch )                  return 0;
  if ( IsRepetition(p) )                  return DrawScore(p); 
  if ( DrawBy50Moves(p) )                 return DrawScore(p); 
  if ( !flagInCheck && RecognizeDraw(p) ) return 0;

  // CHECK EXTENSION
  if (flagInCheck) depth += ONE_PLY;

  // QUIESCENCE SEARCH ENTRY POINT
  if ( depth < ONE_PLY ) return Quiesce(p, ply, 0, alpha, beta, 0, pv);

  nodes++;
  nodesPerBranch++;
  CheckInput();

  // REUSING LEARNED DATA ABOUT SCORE OF SPECIFIC POSITIONS
  if ( Data.useLearning 
  &&  !Data.useWeakening
  &&   ply == 2 ) // reading at bigger depths may hinder exploring alternatives to mainline
  {
	  int learnVal = Learner.ReadLearnData(p->hashKey, depth);
	  if (learnVal != INVALID) return learnVal;
  }
  
  if (ply) *pv = 0;
  move = 0;

  // MATE DISTANCE PRUNING
  alpha = Max(-MATE + ply, alpha);
  beta  = Min( MATE - ply, beta);
  if (alpha >= beta) return alpha;

  // TRANSPOSITION TABLE READ
  if (TransTable.Retrieve(p->hashKey, &move, &score, alpha, beta, depth, ply)) 
  {
	  if (score >= beta)
		  History.UpdateSortOnly(p, move, depth / ONE_PLY, ply);
     return score;
  }
  
  // SAFEGUARD AGAINST HITTING MAX PLY LIMIT
  if (ply >= MAX_PLY - 1) return Eval.ReturnFull(p, alpha, beta);

  // DETERMINE IF WE CAN APPLY PRUNING
  int flagCanPrune 
  =  (nodeType != PV_NODE) 
  && (beta < MAX_EVAL) 
  && !flagInCheck;

  // EVAL PRUNING (aka "post futility pruning", inspired by DiscoCheck by Lucas Braesch) 
  if ( depth <= 3*ONE_PLY
  &&   flagCanPrune
  &&  !wasNull
  &&   p->pieceMat[p->side] > Data.matValue[N] ) {
	 if (nodeEval == INVALID) nodeEval = Eval.ReturnFast(p);
	 int evalMargin = 40 * depth;
	 if (nodeEval - evalMargin >= beta)
		return nodeEval - evalMargin;
  } // end of eval pruning code

  // QUIESCENCE NULL MOVE (idea from CCC post of Vincent Diepeveen)
  if ( Data.useNull
  &&   flagCanPrune
  &&   depth <= minimalNullDepth
  &&  !wasNull
  &&   p->pieceMat[p->side] > Data.matValue[N]) {
     Manipulator.DoNull(p, undoData);
     score = -Quiesce(p, ply, 0, -beta, -beta+1, 0, pv);
	 Manipulator.UndoNull(p, undoData);

	 if (score >= beta) return score;
  }  // end of quiescence null move code

  // NULL MOVE - we allow opponent to move twice in a row; if he cannot beat
  // beta this way, then we assume that there is no need to search any further.

  if ( Data.useNull
  &&   flagCanPrune
  &&   depth > minimalNullDepth
  &&  !wasNull
  &&   p->pieceMat[p->side] > Data.matValue[N]) 
  {
    fullNodeEval = Eval.ReturnFull(p, alpha, beta);
    if ( beta <= fullNodeEval ) {

      newDepth = nullDepth[depth];

	  // normal search would fail low, so null move search shouldn't fail high
      if (TransTable.Retrieve(p->hashKey, &nullRefutation, &nullScore, alpha, beta, newDepth, ply) ) {
		  if (nullScore <= alpha) goto avoidNull;
	  }
		  
      Manipulator.DoNull(p, undoData);                            // NODE_ALL
      nullScore = -Search(p, ply + 1, -beta, -beta + 1, newDepth, NEW_NODE(nodeType), WAS_NULL, 0, 0, newPv);

	  // extract refutation of a null move from transposition table; usually 
	  // it will be a capture and we will sort safe evasions above other quiet moves.
	  TransTable.Retrieve(p->hashKey, &nullRefutation, &score, alpha, beta, depth, ply);
	  
	  // get target square of null move refutation to sort escape moves a bit higher
	  if (nullRefutation != 0 ) refutationSq = Tsq(nullRefutation);
      Manipulator.UndoNull(p, undoData);

      if (flagAbortSearch) return 0; // timeout, "stop" command or mispredicted ponder move

	  // verify null move in the endgame or at sufficient depth
	  if (nullScore >= beta && p->pieceMat[p->side] < 1600 ) 
          nullScore = Search(p, ply, alpha, beta, newDepth-ONE_PLY, CUT_NODE, NO_NULL, 0, 0, newPv); // BUG, should verify with lastMove
	                                             
      if (nullScore >= beta) {
         // we don't want to overwrite real entries, as they are more useful for move ordering 
         if (!move) TransTable.Store(p->hashKey, 0, nullScore, LOWER, depth, ply);
		 return Eval.Normalize(nullScore, MAX_EVAL); // checkmate from null move search isn't reliable
	  }
    }
  } // end of null move code 

  avoidNull:

   // RAZORING based on Toga II 4.0
   if ( nodeType != PV_NODE 
   &&  !flagInCheck 
   &&  !move
   &&  !wasNull
   &&  !(bbPc(p,p->side,P) & bbRelRank[p->side][RANK_7] ) // no pawns to promote in one move
   &&   depth <= 3*ONE_PLY) {
      int threshold = beta - 300 - (depth-ONE_PLY) * 15;
	  if (fullNodeEval == INVALID) fullNodeEval = Eval.ReturnFull(p, alpha, beta);

      if (fullNodeEval < threshold) {
		 score = Quiesce(p, ply, 0, alpha, beta, 0, pv); 
         if (score < threshold) return score;
      }
   } // end of razoring code

  // INTERNAL ITERATIVE DEEPENING - we try to get a hash move to improve move ordering
  if (nodeType == PV_NODE && !move && depth >= 4*ONE_PLY && !flagInCheck ) {
	  Search(p, ply, alpha, beta, depth-2*ONE_PLY, PV_NODE, NO_NULL, lastMove, contMove, newPv); 
	  TransTable.RetrieveMove(p->hashKey, &move);
  }

  if (nodeType == CUT_NODE && !move && depth >= 6*ONE_PLY && !flagInCheck ) {
	  Search(p, ply, alpha, beta, depth-4*ONE_PLY, PV_NODE, NO_NULL, lastMove, contMove, newPv);
	  TransTable.RetrieveMove(p->hashKey, &move);
  } // end of internal iterative deepening code

  // CREATE MOVE LIST AND START SEARCHING
  best = -INF;
  Selector.InitMoveList(p, History.GetRefutation(lastMove), History.GetContinuation(contMove), move, ply);

  // LOOP THROUGH THE MOVE LIST
  while ( move = Selector.NextMove(refutationSq, &flagMoveType) ) {

     // SET FUTILITY PRUNING FLAG BEFORE SEARCHING FIRST APPLICABLE MOVE
	 if (IsMoveOrdinary(flagMoveType) ) {

	     if (++normalMoveCnt == 1) {   
            if ( depth < futilityDepth * ONE_PLY  // we are sufficiently close to the leaf
            && flagCanPrune
            && alpha > -MAX_EVAL ) {
               if (nodeEval == INVALID) nodeEval = Eval.ReturnFast(p);
               nodeEval = TransTable.RefineScore( p->hashKey, nodeEval );

               // this node looks bad enough, so we may apply futility pruning
               if ( (nodeEval + futilityMargin[depth] ) < beta ) flagFutility = 1;
			   // TODO: smaller margin for later moves (requires restructuring)
            }
	     }
	 }

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
	 flagIsReduced = 0;                // this move has not been reduced (yet)
	 depthChange   = 0;                // no depth modification so far
	 History.OnMoveTried(move);

	 // EXTENSIONS might be placed here

	 // DETERMINE IF A MOVE CAN BE REDUCED
	 int flagCanReduce
	 = ( nodeType != PV_NODE) 
	 && !flagInCheck 
	 && !InCheck(p) 
	 && !depthChange 
	 && AvoidReduction(move, flagMoveType);

	 // FUTILITY PRUNING 
	 if ( flagCanReduce
	 &&   flagFutility
	 &&   IsMoveOrdinary(flagMoveType)  // not a tt move, not a capture, not a killer move
	 &&   movesTried > 1                // we have found at least one legal move
	 ) {
        Manipulator.UndoMove(p, move, undoData);
        continue;
	 }

     // LATE MOVE PRUNING (2014-01-24: modelled after Toga II 3.0)
     if ( flagCanReduce
     &&   depth <= 5*ONE_PLY 
     &&  !History.Refutes(lastMove, move) )
     {
         if (IsMoveOrdinary(flagMoveType) 
         &&  normalMoveCnt > moveCountLimit[depth] ) { 
		 if (normalMoveCnt > moveCountLimit[depth] / 2 )  
			 { Manipulator.UndoMove(p, move, undoData); continue; }
	 		 if (History.MoveIsBad(move) ) 
			 { Manipulator.UndoMove(p, move, undoData); continue; }

 		 }
	 }

	 // LATE MOVE REDUCTION
	 if  ( !flagInCheck 
	 &&    !depthChange 
	 &&     AvoidReduction(move, flagMoveType)
     &&     depth >= minimalLmrDepth    // we have some depth left
	 &&     movesTried > 3              // we're sufficiently down the move list
	 &&     History.MoveIsBad(move)     // current move has bad history score
	 &&    !History.Refutes(lastMove, move)
	 &&    !History.Continues(contMove, move)
	 ) {
		 if ( IsMoveOrdinary(flagMoveType) ) {
		    depthChange -= lmrSize[nodeType+1][depth][movesTried];
			if (depth + depthChange < ONE_PLY) {
				depthChange = 0; // don't reduce into qs
			}
			else {
		        History.OnMoveReduced(move);
 	            flagIsReduced = 1;
			}
		 }
	 } // end of late move reduction code

	 newDepth = depth - ONE_PLY + depthChange; // determine new depth

     re_search: // we come back here if a reduced search has not failed low

	 // PRINCIPAL VARIATION SEARCH
	 if (best == -INF )
       score =   -Search(p, ply+1, -beta,    -alpha, newDepth, NEW_NODE(nodeType), NO_NULL, move, lastMove, newPv);
     else { 
       score =   -Search(p, ply+1, -alpha-1, -alpha, newDepth, CUT_NODE, NO_NULL, move, lastMove, newPv);
       if (!flagAbortSearch && score > alpha && score < beta)
         score = -Search(p, ply+1, -beta,    -alpha, newDepth, PV_NODE, NO_NULL, move, lastMove, newPv);
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
         History.OnGoodMove(p, lastMove, move, depth / ONE_PLY, ply);
		 if (!History.MoveChangesMaterialBalance(p, move) ) {
		    History.UpdateRefutation(lastMove, move);
			History.UpdateContinuation(contMove, move);
		 }		    
         TransTable.Store(p->hashKey, move, score, LOWER, depth, ply);
         return score;
     }

	 // SCORE CHANGE
     if (score > best) {
         best = score;
         if (score > alpha) {
            alpha = score;
            if (nodeType == PV_NODE) BuildPv(pv, newPv, move);
         }
      }
   }
  
   // RETURN CORRECT CHECKMATE/STALEMATE SCORE
   if (best == -INF) return flagInCheck ? -MATE + ply : 0;

   // SAVE SEARCH RESULT IN TRANSPOSITION TABLE
   if (*pv) {
 	 History.OnGoodMove(p, lastMove, *pv, depth / ONE_PLY, ply);
     TransTable.Store(p->hashKey, *pv, best, EXACT, depth, ply);
   } else
     TransTable.Store(p->hashKey, 0, best, UPPER, depth, ply);

   return best;
}

int sSearcher::IsRepetition(sPosition *p)
{
   for (int i = 4; i <= p->reversibleMoves; i += 2)
      if (p->hashKey == p->repetitionList[p->head - i])
         return 1;
   return 0;
}

int sSearcher::DrawBy50Moves(sPosition *p) { 
   return( p->reversibleMoves > 100);
}

int sSearcher::IsMoveOrdinary(int flagMoveType) {
   return (!flagMoveType || flagMoveType == FLAG_NULL_EVASION);
}

int sSearcher::AvoidReduction(int move, int flagMoveType) {
   return (MoveType(move) != CASTLE) && (flagMoveType != FLAG_HASH_MOVE) && (flagMoveType != FLAG_KILLER_MOVE);
}

void sSearcher::CheckInput(void)
{
   char command[80];

   if (Data.verbose) {
      if (!( nodes % 500000) ) DisplaySpeed(); // report search speed
   }

   if (nodes & 4095 || rootDepth == ONE_PLY) return;

   if (InputAvailable()) {
      Parser.ReadLine(command, sizeof(command));
      if (strcmp(command, "stop") == 0)
         flagAbortSearch = 1;
      else if (strcmp(command, "ponderhit") == 0)
         pondering = 0;
   }

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

int sSearcher::DrawScore(sPosition *p)
{
   int scale = Min(24, p->phase);
   int score = (-Data.contempt * scale) / 24;

   if ( p->side == rootSide ) return score;
   else                       return -score;
}
