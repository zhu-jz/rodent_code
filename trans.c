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

#include <stdlib.h>
#include "data.h"
#include "rodent.h"
#include "bitboard/bitboard.h"
#include "trans.h"

//#define EVAL_SYMMETRY_TEST

// calculates full hash key from scratch
U64 sTransTable::InitHashKey(sPosition *p)
{
  U64 key = 0;
  for (int i = 0; i < 64; i++) {
    if (p->pc[i] != NO_PC)
      key ^= zobPiece[p->pc[i]][i];
  }

  key ^= zobCastle[p->castleFlags];
  
  if (p->epSquare != NO_SQ)
    key ^= zobEp[File(p->epSquare)];

  if (p->side == BLACK)
    key ^= SIDE_RANDOM;

  return key;
}

// calculates pawn hash key from scratch
U64 sTransTable::InitPawnKey(sPosition *p)
{
  U64 pawnKey = 0;

  for (int i = 0; i < 64; i++)
    if ( p->bbTp[P] & SqBb(i) )
      pawnKey ^= zobPiece[p->pc[i]][i];

  return pawnKey;
}


void sTransTable::Alloc(int mbsize)
{
  for (tt_size = 2; tt_size <= mbsize; tt_size *= 2)
    ;
  tt_size = ((tt_size / 2) << 20) / sizeof(ENTRY);
  tt_mask = tt_size - 4;
  free(tt);
  tt = (ENTRY *) malloc(tt_size * sizeof(ENTRY));
  Clear();
}

void sTransTable::Clear(void)
{
  ENTRY *entry;

  tt_date = 0;
  for (entry = tt; entry < tt + tt_size; entry++) {
    entry->key = 0;
    entry->date = 0;
    entry->move = 0;
    entry->score = 0;
    entry->flags = 0;
    entry->depth = 0;
  }
}

int sTransTable::Retrieve(U64 key, int *move, int *score, int alpha, int beta, int depth, int ply)
{
#ifdef EVAL_SYMMETRY_TEST
  return 0;
#endif
  ENTRY *entry;
  int i;
  // TODO: node type as input and return only exact scores in pv nodes

  entry = tt + (key & tt_mask);
  for (i = 0; i < 4; i++) {
    if (entry->key == key) {
      *move = entry->move;
      if (entry->depth >= depth) {
        *score = entry->score;
        if (*score < -MAX_EVAL)
          *score += ply;
        else if (*score > MAX_EVAL)
          *score -= ply;
        if ((entry->flags & UPPER && *score <= alpha) ||
            (entry->flags & LOWER && *score >= beta) )
		    {
            entry->date = tt_date; // refreshing entry
            return 1;
            }
      }
      break;
    }
    entry++;
  }
  return 0;
}

 void sTransTable::RetrieveMove(U64 key, int *move )
 {
#ifdef EVAL_SYMMETRY_TEST
  return;
#endif
  ENTRY *entry;
  int i;
  *move = 0;
  entry = tt + (key & tt_mask);
  for (i = 0; i < 4; i++) {
    if (entry->key == key) {
    *move = entry->move; // found
     break;
              }
    entry++;
  }
 }

// This is an idea from Stockfish: eval score used in search for pruning
// or  reduction decisions may be "refined" by comparing it with  actual
// search result scored in transposition table.

int sTransTable::RefineScore(U64 key, int score)
{
#ifdef EVAL_SYMMETRY_TEST
  return score;
#endif
  ENTRY *entry;
  int i;
  int val;

  entry = tt + (key & tt_mask);
  for (i = 0; i < 4; i++) {
    if (entry->key == key) 
	{
      val = entry->score;		
      if  (entry->flags & UPPER ) return Max(score, val);
      if  (entry->flags & LOWER ) return Min(score, val);                
      break;
    }
    entry++;
  }
  return score;
}

void sTransTable::Store(U64 key, int move, int score, int flags, int depth, int ply)
{
  ENTRY *entry, *replace;
  int i, oldest, age;

  if (score < -MAX_EVAL)
    score -= ply;
  else if (score > MAX_EVAL)
    score += ply;
  replace = NULL;
  oldest = -1;
  entry = tt + (key & tt_mask);
  for (i = 0; i < 4; i++) {
    if (entry->key == key) {
      if (!move) move = entry->move; // preserve hash move
      replace = entry;
      break;
    }
    age = ((tt_date - entry->date) & 255) * 256 + 255 - (entry->depth / ONE_PLY);
    if (age > oldest) {
      oldest = age;
      replace = entry;
    }
    entry++;
  }
  replace->key = key; 
  replace->date = tt_date; 
  replace->move = move;
  replace->score = score; 
  replace->flags = flags; 
  replace->depth = depth;
}

void sTransTable::ChangeDate() 
{
  tt_date = (tt_date + 1) & 255;
}