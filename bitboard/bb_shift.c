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

U64 ShiftNorth(U64 bb) {
	return bb << 8;
}

U64 ShiftSouth(U64 bb) {
	return bb >> 8;
}

U64 ShiftWest(U64 bb) {
    return (bb & bbNotA) >> 1;
}

U64 ShiftEast(U64 bb) {
    return ( bb & bbNotH) << 1;
}

U64 ShiftNW(U64 bb) {
    return (bb & bbNotA) << 7;
}

U64 ShiftNE(U64 bb) {
    return (bb & bbNotH) << 9;
}

U64 ShiftSW(U64 bb) {
    return (bb & bbNotA) >> 9;
}

U64 ShiftSE(U64 bb) {
    return (bb & bbNotH) >> 7;
}

U64 ShiftFwd(U64 bb, int side) {
	if (side == WHITE) return ShiftNorth(bb);
	                   return ShiftSouth(bb);
}