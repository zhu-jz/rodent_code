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

#include "bitboard.h"
#include "../rodent.h"

void InitPassedMask() 
{
  for (int sq = 0; sq < 64; sq++) {
	bbPassedMask[WHITE][sq] = FillNorthExcl(SqBb(sq));
	bbPassedMask[WHITE][sq] |= ShiftWest(bbPassedMask[WHITE][sq]);
	bbPassedMask[WHITE][sq] |= ShiftEast(bbPassedMask[WHITE][sq]);
	bbPassedMask[BLACK][sq] = FillSouthExcl(SqBb(sq));
	bbPassedMask[BLACK][sq] |= ShiftWest(bbPassedMask[BLACK][sq]);
	bbPassedMask[BLACK][sq] |= ShiftEast(bbPassedMask[BLACK][sq]);
  }
}

void InitAdjacentMask() 
{
  for (int col = 0; col < 8; col++) {
    bbAdjacentMask[col] = 0ULL;
    if (col > 0) bbAdjacentMask[col] |= bbFILE_A << (col - 1);
    if (col < 7) bbAdjacentMask[col] |= bbFILE_A << (col + 1);
  }
}

void InitPossibleAttacks() 
{
  for (int i = 0; i < 64; i++) {
	for (int j = 0; j < 64; j++) {
	  // which squares can be attacked from a given square?
	  bbBAttacksOnEmpty[i] = BAttacks(0ULL, i); 
	  bbRAttacksOnEmpty[i] = RAttacks(0ULL, i); 

	  // can a piece on a given square attack zone around enemy king?
	  bbRCanAttack[i][j] = bbBCanAttack[i][j] = bbQCanAttack[i][j] = 0ULL;
	  if ( RAttacks(0ULL, i) & bbKingAttacks[j] ) { bbRCanAttack[i][j] = 1; bbQCanAttack[i][j] = 1; };
	  if ( BAttacks(0ULL, i) & bbKingAttacks[j] ) { bbBCanAttack[i][j] = 1; bbQCanAttack[i][j] = 1; };
	}
  }
}

void InitPawnSupport() // pawn is supported if we find friendly pawns on masked squares
{
  for (int sq = 0; sq < 64; sq++) {
      bbPawnSupport[WHITE][sq] = ShiftWest(SqBb(sq)) | ShiftEast(SqBb(sq));
	  bbPawnSupport[WHITE][sq] |= FillSouth(bbPawnSupport [WHITE][sq] );

	  bbPawnSupport[BLACK][sq] = ShiftWest(SqBb(sq)) | ShiftEast(SqBb(sq));
	  bbPawnSupport[BLACK][sq] |= FillNorth(bbPawnSupport [BLACK][sq] );
  }
}
