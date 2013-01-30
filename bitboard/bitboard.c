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
#include "bitboard.h"

void PrintBb( U64 bbTest) 
{
  for (int i = 0; i < 64; i++) {
      if (bbTest & SqBb(relativeSq[BLACK][i]) ) printf("+ ");
	  else                   printf(". ");
	  if ( (i+1) % 8 == 0) printf(" %d\n", 9 - ((i+1) / 8) );
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

U64 GetWPControl(U64 bb) 
{
    return ( ShiftNE(bb) | ShiftNW(bb) );
}

U64 GetBPControl(U64 bb) 
{
    return ( ShiftSE(bb) | ShiftSW(bb) );
}

U64 GetPawnAttacks(int side, U64 bb) 
{
    if (side == WHITE) return GetWPControl(bb);
	                   return GetBPControl(bb);    
}

int PopFirstBit(U64 * bb) 
{
	U64 bbLocal = *bb;
    *bb &= (*bb - 1);
    return FirstOne(bbLocal);
}

int FirstOneAsm(U64 bb)
{ _asm { mov  eax, dword ptr bb[0]
         test eax, eax
         jz   f_hi
         bsf  eax, eax
         jmp  f_ret
f_hi:    bsf  eax, dword ptr bb[4]
         add  eax, 20h
f_ret:
  }
}