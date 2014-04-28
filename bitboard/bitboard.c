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
#include "../rodent.h"
#include "bitboard.h"

extern const U64 bbFile        [8] = { bbFILE_A, bbFILE_B, bbFILE_C, bbFILE_D, bbFILE_E, bbFILE_F, bbFILE_G, bbFILE_H };
extern const U64 bbRelRank [2] [8] = { {bbRANK_1, bbRANK_2, bbRANK_3, bbRANK_4, bbRANK_5, bbRANK_6, bbRANK_7, bbRANK_8 }, 
                                       {bbRANK_8, bbRANK_7, bbRANK_6, bbRANK_5, bbRANK_4, bbRANK_3, bbRANK_2, bbRANK_1 } };

void PrintBb( U64 bbTest) 
{
  for (int sq = 0; sq < 64; sq++) {
	  if (bbTest & RelSqBb(sq,BLACK) ) printf("+ ");
	  else                             printf(". ");
	  if ( (sq+1) % 8 == 0) printf(" %d\n", 9 - ((sq+1) / 8) );
  }
  printf("\na b c d e f g h\n");
}

U64 flipVertical(U64 bb) 
{
   const U64 k1 = 0x00FF00FF00FF00FF;
   const U64 k2 = 0x0000FFFF0000FFFF;
   bb = ((bb >>  8) & k1) | ((bb & k1) <<  8);
   bb = ((bb >> 16) & k2) | ((bb & k2) << 16);
   bb = ( bb >> 32)       | ( bb       << 32);
   return bb;
}

U64 GetFrontSpan(U64 bb, int side) 
{
    if (side == WHITE) return FillNorthExcl(bb);
	                   return FillSouthExcl(bb);
}

U64 GetRearSpan(U64 bb, int side) 
{
    if (side == WHITE) return FillSouthExcl(bb);
	                   return FillNorthExcl(bb);
}

U64 GetWPControl(U64 bb) {
    return ( ShiftNE(bb) | ShiftNW(bb) );
}

U64 GetBPControl(U64 bb) {
    return ( ShiftSE(bb) | ShiftSW(bb) );
}

U64 GetPawnAttacks(int side, U64 bb) 
{
    if (side == WHITE) return GetWPControl(bb);
	                   return GetBPControl(bb);    
}

U64 ShiftFwd(U64 bb, int side) 
{
	if (side == WHITE) return ShiftNorth(bb);
	                   return ShiftSouth(bb);
}

int PopFirstBit(U64 * bb) 
{
	U64 bbLocal = *bb;
    *bb &= (*bb - 1);
    return FirstOne(bbLocal);
}

int PopFlippedBit(U64 * bb)
{
   U64 x = *bb;
   x = x ^ ((x ^ flipVertical(x)) & -1);
   int sq = FirstOne(x) ^ 56;
   *bb ^= SqBb(sq);
   return sq;
}

int PopNextBit(int side, U64 * bb)
{
   U64 x = *bb;
   U64 m = (U64)side - 1; // e.g. -1 if white, 0 for black
   int o = (int)m & 56;
   x = x ^ ((x ^ flipVertical(x)) & m); // conditional flip
   int sq = FirstOne(x) ^ o;
   *bb ^= SqBb(sq);
   return sq;
}

#ifdef USE_FIRST_ONE_ASM

int FirstOneAsm(U64 bb)
{
#if defined(__GNUC__)
return __builtin_ffsll(bb) - 1;
#else
_asm { mov  eax, dword ptr bb[0]
         test eax, eax
         jz   f_hi
         bsf  eax, eax
         jmp  f_ret
f_hi:    bsf  eax, dword ptr bb[4]
         add  eax, 20h
f_ret:
}
#endif
}

#endif
