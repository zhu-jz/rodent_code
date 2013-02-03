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

// transposition table entry
typedef struct {
  U64 key;
  short date;
  short move;
  short score;
  unsigned char flags;
  unsigned char depth;
} ENTRY;

// transposition table with access functions
struct sTransTable {
private:
  int tt_size;
  int tt_mask;
  int tt_date;
  ENTRY *tt;

public:
  U64 InitHashKey(sPosition *p);
  U64 InitPawnKey(sPosition *p);
  void Alloc(int);
  void Clear(void);
  int Retrieve(U64 key, int *move, int *score, int alpha, int beta, int depth, int ply);
  void RetrieveMove(U64 key, int *move );
  int RefineScore(U64 key, int score);
  void Store(U64 key, int move, int score, int flags, int depth, int ply);
  void ChangeDate();
};

extern sTransTable TransTable;