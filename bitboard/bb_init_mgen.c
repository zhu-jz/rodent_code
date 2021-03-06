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

static const int dirs[4][2] = {{1, -1}, {16, -16}, {17, -17}, {15, -15}};
static const U64 bbDIAG_A1H8 = 0x8040201008040201; // constants shifted to create diagonal bitmasks
static const U64 bbDIAG_A8H1 = 0x0102040810204080;

// Stuff initialized here is used by the macros RAttacks, BAtacks and QAttacks.
// To see the use of these tables, study rodent.h, just before aforementioned macros.

void InitKindergartenBitboards()
{
   int i, j, k, l, x, y;

   // init masks
   for (int sq = 0; sq < 64; sq++) {

	  // line masks
      bbLineMask[HOR][sq] = bbRANK_1 << (sq & 070);
      bbLineMask[VER][sq] = bbFILE_A << (sq & 007);

	  // 1st set of diagonal masks
      j = File(sq) - Rank(sq);
      if (j > 0) bbLineMask[DIAG_AH][sq] = bbDIAG_A1H8 >> (j * 8);
      else       bbLineMask[DIAG_AH][sq] = bbDIAG_A1H8 << (-j * 8);

	  // 2nd set of diagonal masks
      j = File(sq) - (RANK_8 - Rank(sq));
      if (j > 0) bbLineMask[DIAG_HA][sq] = bbDIAG_A8H1 << (j * 8);
      else       bbLineMask[DIAG_HA][sq] = bbDIAG_A8H1 >> (-j * 8);
   }

   // init sliding piece attacks
   for (i = 0; i < 4; i++)
      for (j = 0; j < 64; j++)
         for (k = 0; k < 64; k++) {
            attacks[i][j][k] = 0;

            for (l = 0; l < 2; l++) {
               x = Map0x88(j) + dirs[i][l];
               while (!Sq0x88Off(x)) {
                  y = Unmap0x88(x);
                  attacks[i][j][k] |= SqBb(y);
                  if ((k << 1) & (1 << (i != 1 ? File(y) : Rank(y))))
                     break;
                  x += dirs[i][l];
               }
			}
         }
}

void InitPawnAttacks()
{
   for (int sq = 0; sq < 64; sq++) {
      bbPawnAttacks[WHITE][sq] = GetWPControl(SqBb(sq) );
      bbPawnAttacks[BLACK][sq] = GetBPControl(SqBb(sq) );
   }
}

void InitKnightAttacks()
{
   for (int sq = 0; sq < 64; sq++)
      bbKnightAttacks[sq] = ShiftNE( ShiftNorth(SqBb(sq) ) )
                          | ShiftNW( ShiftNorth(SqBb(sq) ) )
                          | ShiftNE( ShiftEast (SqBb(sq) ) )
                          | ShiftSE( ShiftEast (SqBb(sq) ) )
                          | ShiftSW( ShiftSouth(SqBb(sq) ) )
                          | ShiftSE( ShiftSouth(SqBb(sq) ) )
                          | ShiftNW( ShiftWest (SqBb(sq) ) )
                          | ShiftSW( ShiftWest (SqBb(sq) ) );
}

void InitKingAttacks()
{
   for (int sq = 0; sq < 64; sq++) {
      bbKingAttacks[sq] = FillKing( SqBb(sq) ) ^ SqBb(sq);
  
   // set bitboard of squares constituting king zone (used in king safety eval)
   bbKingZone[WHITE][sq] = bbKingAttacks[sq] | bbKingAttacks[sq] << 8;
   bbKingZone[BLACK][sq] = bbKingAttacks[sq] | bbKingAttacks[sq] >> 8;
  }
}
