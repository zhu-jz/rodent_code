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

#include "bitboard.h"
#include "../rodent.h"

/* bitboard fill routines adapted from:
/ http://chessprogramming.wikispaces.com/Kogge-Stone+Algorithm#Fillonanemptyboard */

U64 FillKing(U64 bb) {
    bb |= ShiftWest(bb);
	bb |= ShiftEast(bb);
	bb |= ( ShiftNorth(bb) | ShiftSouth(bb) );
	return bb;
}

U64 FillNorth(U64 bb) {
    bb |= bb << 8;
	bb |= bb << 16;
	bb |= bb << 32;
	return bb;
}

U64 FillSouth(U64 bb) {
    bb |= bb >> 8;
	bb |= bb >> 16;
	bb |= bb >> 32;
	return bb;
}

U64 FillWest(U64 bb) {
   const U64 pr0 = bbNotH;
   const U64 pr1 = pr0 & (pr0 >> 1);
   const U64 pr2 = pr1 & (pr1 >> 2);
   bb |= pr0 & (bb >> 1);
   bb |= pr1 & (bb >> 2);
   bb |= pr2 & (bb >> 4);
   return bb;
}

U64 FillEast(U64 bb) {
   const U64 pr0 = bbNotA;
   const U64 pr1 = pr0 & (pr0 << 1);
   const U64 pr2 = pr1 & (pr1 << 2);
   bb |= pr0 & (bb  << 1);
   bb |= pr1 & (bb  << 2);
   bb |= pr2 & (bb  << 4);
   return bb;
}

U64 FillNW(U64 bb) {
   const U64 pr0 = bbNotH;
   const U64 pr1 = pr0 & (pr0 <<  7);
   const U64 pr2 = pr1 & (pr1 << 14);
   bb |= pr0 & (bb <<  7);
   bb |= pr1 & (bb << 14);
   bb |= pr2 & (bb << 28);
   return bb;
}

U64 FillNE(U64 bb) {
   const U64 pr0 = bbNotA;
   const U64 pr1 = pr0 & (pr0 <<  9);
   const U64 pr2 = pr1 & (pr1 << 18);
   bb |= pr0 & (bb <<  9);
   bb |= pr1 & (bb << 18);
   bb |= pr2 & (bb << 36);
   return bb;
}

U64 FillSW(U64 bb) {
   const U64 pr0 = bbNotH;
   const U64 pr1 = pr0 & (pr0 >>  9);
   const U64 pr2 = pr1 & (pr1 >> 18);
   bb |= pr0 & (bb >>  9);
   bb |= pr1 & (bb >> 18);
   bb |= pr2 & (bb >> 36);
   return bb;
}

U64 FillSE(U64 bb) {
   const U64 pr0 = bbNotA;
   const U64 pr1 = pr0 & (pr0 >>  7);
   const U64 pr2 = pr1 & (pr1 >> 14);
   bb |= pr0 & (bb >>  7);
   bb |= pr1 & (bb >> 14);
   bb |= pr2 & (bb >> 28);
   return bb;
}

// fill routines excluding initial square

U64 FillNorthExcl(U64 bb) {
	return FillNorth( ShiftNorth(bb) );
}

U64 FillSouthExcl(U64 bb) {
  return FillSouth( ShiftSouth(bb) );
}

U64 FillWestExcl(U64 bb) {
	return FillWest( ShiftWest(bb) );
}

U64 FillEastExcl(U64 bb) {
	return FillEast( ShiftEast(bb) );
}

U64 FillNEExcl(U64 bb) {
	return FillNE( ShiftNE(bb) );
}

U64 FillNWExcl(U64 bb) {
	return FillNW( ShiftNW(bb) );
}

U64 FillSEExcl(U64 bb) {
	return FillSE( ShiftSE(bb) );
}

U64 FillSWExcl(U64 bb) {
	return FillSW( ShiftSW(bb) );
}