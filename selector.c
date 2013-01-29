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
#include "move/move.h"
#include "search/search.h"
#include "hist.h"
#include "selector.h"

// TODO: try bad captures as killers
// TODO: try introducing "bad quiet moves"

// initializes data needed for move ordering
void sSelector::InitMoves(sPosition *p, int transMove, int ply)
{
  m->p = p;
  m->phase = 0;
  m->transMove = transMove;
  m->killer1 = History.GetFirstKiller(ply);
  m->killer2 = History.GetSecondKiller(ply);
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
      move = SelectBest();
      if (move == m->transMove) continue; // hash move already tried

      // save bad captures for later
	  if (BadCapture(m->p, move) ) {
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
      move = SelectBest();

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


// initializes capture list, assigning sorting values to the moves

void sSelector::InitCaptures(sPosition *p, int hashMove)
{
  m->p = p;
  m->last = GenerateCaptures(m->p, m->move);
  ScoreCaptures(hashMove);
  m->next = m->move;
}

// used in Quiesce()
int sSelector::NextCapture(void)
{
  int move;

  while (m->next < m->last) {
    move = SelectBest();                  // find next best move
    return move;
  }
  return 0;
}

// order captures using MvvLva() function

void sSelector::ScoreCaptures(int hashMove)
{
  int *movep, *valuep;

  valuep = m->value;
  for (movep = m->move; movep < m->last; movep++)
    *valuep++ = (MvvLva(m->p, *movep) + (1000 * (*movep == hashMove) ) );
}

// ScoreQuiet() sorts quiet moves by history heuristic, 
// increasing the value of a move that evades a capture 
// which  had  refuted  null  move  in  the  same  node 
// and doesn't cause large material losses.

void sSelector::ScoreQuiet(int refutationSq)
{
  int *movep, *valuep;
  int sort_val;

  valuep = m->value;
  for (movep = m->move; movep < m->last; movep++) {
    
	// assign history value
	sort_val =  History.GetMoveHistoryValue(m->p->pc[Fsq(*movep)], Tsq(*movep) );

    // null move refutation is sorted much higher	
	if ( Fsq(*movep) == refutationSq ) { 
		if ( Swap(m->p, Fsq(*movep), Tsq(*movep) ) >= -100 ) { // TODO: try -50
		sort_val *= 2;
		sort_val += 10000;
		}
	}
	// TODO: if capture refuting null move is HxL, promote moves that defend a piece

    *valuep++ = sort_val;
  }
}

// pick the best remaining move

int sSelector::SelectBest(void)
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

// BadCapture() identifies captures that are likely to lose material

// TODO: split BadCapture() into function for quiescence search 
// and for main search; the latter might accept sacrifices 
// in the vicinity of enemy king, captures of passed pawns etc.
// in order to sort them higher.

int sSelector::BadCapture(sPosition *p, int move) // last change 2012-03-06
{
  int fsq = Fsq(move);
  int tsq = Tsq(move);

  // Marginal speedup: function saves on some value comparisons and Swap() calls
  // automatically accepts P x X and minor x minor, including BxN (added 2012-03-06)
  if ( TpOnSq(p, fsq) == P ) return 0;
  if ( TpOnSq(p, fsq) == N && TpOnSq(p, tsq) != P) return 0;
  if ( TpOnSq(p, fsq) == B && TpOnSq(p, tsq) != P) return 0;

  // Good or equal captures not identified by the above conditions
  if (Data.matValue[TpOnSq(p, tsq)] >= Data.matValue[TpOnSq(p, fsq)] )
    return 0;

  // En passant capture is equal by definition
  if (MoveType(move) == EP_CAP) return 0;

  // No shortcut worked - we must do expensive static exchange evaluation
  return Swap(p, fsq, tsq) < -Data.goodCaptMargin;
}

// MvvLva()  (most valuable victim - least valuable  attacker)
// function  is  used to sort captures; main consideration  is 
// the value of a captured piece (victim), with the capturer's
// value used as a tie-break (the lower the better)

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

void sFlatMoveList::Init(sPosition * p)
{
	sSelector Selector;      // an object responsible for maintaining move list and picking moves 
	int move;
	int unusedFlag;
	UNDO  undoData[1];       // data required to undo a move
	  int pv[MAX_PLY];

	Selector.InitMoves(p, 0, 0); // prepare move selector
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