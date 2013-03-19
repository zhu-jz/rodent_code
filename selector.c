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


This  file contains implementation of sSelector  class,  reponsible 
for maintaining a move list and returning moves in predefined order. 
Its core function is NextMove(), implementing staged move generation 
and sorting of moves.
*/

#include "rodent.h"
#include "data.h"
#include "bitboard/bitboard.h"
#include "search/search.h"
#include "hist.h"

// initializes data needed for move ordering
void sSelector::InitMoveList(sPosition *p, int transMove, int ply)
{
  m->p = p;
  m->phase = 0;
  m->transMove = transMove;
  m->killer1 = History.GetKiller(ply, 0);
  m->killer2 = History.GetKiller(ply, 1);
}

int sSelector::NextMove(int refutationSq, int *flag)
{
  int move;

  switch (m->phase) {

  case 0: // first return transposition table move, if legal
    move = m->transMove;
    if (move && IsLegal(m->p, move)) {
      m->phase = 1;
	  *flag = FLAG_HASH_MOVE;
      return move;
    }

  case 1: // helper phase: capture generation
    m->last = GenerateCaptures(m->p, m->move);
    ScoreCaptures(0);
    m->next = m->move;
    m->badp = m->bad;
    m->phase = 2;

  case 2: // return good or equal captures
    while (m->next < m->last) {
      move = PickBestMove();
      if (move == m->transMove) continue; // hash move already tried

      // save bad captures for later
	  if (CaptureIsBad(m->p, move) ) {
        *m->badp++ = move;
        continue;
      }
	  *flag = FLAG_GOOD_CAPTURE;
      return move;
    }

  case 3: // return first killer move
    move = m->killer1;
    if (move && move != m->transMove 
    &&  m->p->pc[Tsq(move)] == NO_PC 
	&& IsLegal(m->p, move)) 
	{
      m->phase = 4;
	  *flag = FLAG_KILLER_MOVE;
      return move;
    }

  case 4: // return second killer move
    move = m->killer2;
    if (move && move != m->transMove 
	&&  m->p->pc[Tsq(move)] == NO_PC 
	&& IsLegal(m->p, move)) {
      m->phase = 5;
	  *flag = FLAG_KILLER_MOVE;
      return move;
    }

  case 5: // helper phase: generate quiet moves
    m->last = GenerateQuiet(m->p, m->move);
    ScoreQuiet(refutationSq);
    m->next = m->move;
    m->phase = 6;

  case 6: // return next quiet move
    while (m->next < m->last) {
      move = PickBestMove();

      if (move == m->transMove 
      ||  move == m->killer1 
	  ||  move == m->killer2)
        continue;

	  if ( Fsq(move) == refutationSq)   *flag = FLAG_NULL_EVASION;
	  else                              *flag = FLAG_NORMAL_MOVE;

      return move;
    }
    m->next = m->bad;
    m->phase = 7;

  case 7: // return next bad capture
	  if (m->next < m->badp) {
      *flag = FLAG_BAD_CAPTURE; 
      return *m->next++;
	  }
  }
  return 0;
}

void sSelector::InitCaptureList(sPosition *p, int hashMove)
{
  m->p = p;
  m->last = GenerateCaptures(m->p, m->move);
  ScoreCaptures(hashMove);
  m->next = m->move;
}

int sSelector::NextCapture(void)  // used in Quiesce()
{
  int move;

  while (m->next < m->last) {
    move = PickBestMove();
    return move;
  }
  return 0;
}

// order captures using MvvLva() function and putting hash move first

void sSelector::ScoreCaptures(int hashMove)
{
  int *movep, *valuep;

  valuep = m->value;
  for (movep = m->move; movep < m->last; movep++)
    *valuep++ = (MvvLva(m->p, *movep) + (1000 * (*movep == hashMove) ) );
}

// ScoreQuiet() sorts quiet moves by history heuristic, increasing 
// the value of a move that evades a capture which  had  refuted  
// null move in the same node and doesn't cause large material losses.

void sSelector::ScoreQuiet(int refutationSq)
{
  int *movep, *valuep;
  int sortVal;

  valuep = m->value;
  for (movep = m->move; movep < m->last; movep++) {
    
	// assign base sort value
	sortVal =  History.GetMoveHistoryValue(m->p->pc[Fsq(*movep)], Tsq(*movep) );
	//sortVal = Data.pstMg[WHITE][B][Tsq(*movep)] - Data.pstMg[WHITE][B][Fsq(*movep)];

    // null move refutations are sorted much higher	
	if ( Fsq(*movep) == refutationSq ) { 
		if ( Swap(m->p, Fsq(*movep), Tsq(*movep) ) >= -100 ) { // TODO: try -50
		   sortVal *= 2;
		   sortVal += 10000;
		}
    }

    *valuep++ = sortVal;
  }
}

int sSelector::PickBestMove(void)
{
  int *movep, *valuep, aux;

  valuep = m->value + (m->last - m->move) - 1;
  for (movep = m->last - 1; movep > m->next; movep--) {
    if (*valuep > *(valuep - 1)) {
      aux = *valuep;
      *valuep = *(valuep - 1);
      *(valuep - 1) = aux;
      aux = *movep;
      *movep = *(movep - 1);
      *(movep - 1) = aux;
    }
    valuep--;
  }
  return *m->next++;
}

int sSelector::CaptureIsBad(sPosition *p, int move)
{
  int fsq = Fsq(move);
  int tsq = Tsq(move);

  // Marginal saving on value comparisons and Swap() calls. Here we accept
  // "pawn takes any" and "minor takes minor", including BxN (added 2012-03-06)
  if ( TpOnSq(p, fsq) == P ) return 0;
  if ( TpOnSq(p, fsq) == N && TpOnSq(p, tsq) != P) return 0;
  if ( TpOnSq(p, fsq) == B && TpOnSq(p, tsq) != P) return 0;

  // Remaining good or equal captures identified by piece value
  if (Data.matValue[TpOnSq(p, tsq)] >= Data.matValue[TpOnSq(p, fsq)] )
    return 0;

  // En passant capture is equal by definition
  if (MoveType(move) == EP_CAP) return 0;

  // If we are still here, but opponent can recapture by a pawn, capture is bad.
  if (bbPc(p, Opp(p->side), P) & bbPawnAttacks[p->side][tsq] ) return 1;

  // No shortcut worked - we must do expensive static exchange evaluation
  return Swap(p, fsq, tsq) < -Data.goodCaptMargin;
}

// MvvLva() (stands for most valuable victim - least valuable attacker)
// sorts captures according to the value of a captured piece (victim), 
// with the capturer's value used as a tie-break (the lower the better)

int sSelector::MvvLva(sPosition *p, int move)
{  
  if (p->pc[Tsq(move)] != NO_PC)
    return TpOnSq(p, Tsq(move)) * 6 + 5 - TpOnSq(p, Fsq(move));
  
  if (IsProm(move)) 
	return PromType(move) - 5;

  return 5;
}

void sFlatMoveList::AddMove(int move)
{
	 moves[nOfMoves] = move;
	 nOfMoves++;
}

void sFlatMoveList::ClearUsed(int bestMove)
{
     for (int i = 0; i < nOfMoves; i++)
		used[i] = 0;
}

void sFlatMoveList::ScoreLastMove( int move, int val)
{
	 for (int i = 0; i < nOfMoves; i++)
		 if (moves[i] == move) value[i] += val;
}

int sFlatMoveList::GetNextMove() 
{
	int tempVal;
	int bestVal  = -INF;
	int bestMove = 0;
	for (int i = 0; i < nOfMoves; i++) {
		if (moves[i] == Searcher.bestMove ) tempVal = MAX_INT;
		else                                tempVal = value[i];

		if ( tempVal > bestVal && !used[i]) {
			bestVal = tempVal;
			bestMove = moves[i];
			currMoveIndex = i;
		}
	}
	used[currMoveIndex] = 1;
	return bestMove;
}

void sFlatMoveList::Init(sPosition * p)
{
	sSelector Selector;      // an object responsible for maintaining move list and picking moves 
	int move;
	int unusedFlag;
	UNDO  undoData[1];       // data required to undo a move
	int pv[MAX_PLY];

	Selector.InitMoveList(p, 0, 0); // prepare move selector
	nOfMoves = 0;
	bestMove = 0;
	bestVal = -INF;
	
	while ( move = Selector.NextMove(0, &unusedFlag) ) {

      Manipulator.DoMove(p, move, undoData);
      if (IllegalPosition(p)) { 
		  Manipulator.UndoMove(p, move, undoData); 
		  continue; 
	  }
 
	  value[nOfMoves] = -Searcher.Quiesce(p, 0, 0, -INF, INF, pv);
	  if (value[nOfMoves] > bestVal) {
		  bestVal = value[nOfMoves];
		  bestMove = move;
	  }

	  AddMove(move);
	  Manipulator.UndoMove(p, move, undoData);
	}
}