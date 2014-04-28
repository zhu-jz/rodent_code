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

#include "../rodent.h"
#include "bitboard.h"

#if defined(__GNUC__)

int PopCnt(U64 bb) {
    return __builtin_popcountll(bb);
}

int PopCnt15(U64 bb) {
    return __builtin_popcountll(bb);
}

int PopCntSparse(U64 bb) {
   return __builtin_popcountll(bb);
}

#elif defined(USE_MM_POPCNT) && defined(_M_AMD64)  // 64 bit windows
#include <nmmintrin.h>
FORCEINLINE int PopCnt(U64 bb) {
    return (int) _mm_popcnt_u64(bb);
}
FORCEINLINE int PopCnt15(U64 bb) {
    return (int) _mm_popcnt_u64(bb);
}
FORCEINLINE int PopCntSparse(U64 bb) {
    return (int) _mm_popcnt_u64(bb);
}

#else

int PopCnt(U64 bb) // general purpose population count
{
    U64 k1 = (U64)0x5555555555555555;
    U64 k2 = (U64)0x3333333333333333;
    U64 k3 = (U64)0x0F0F0F0F0F0F0F0F;
    U64 k4 = (U64)0x0101010101010101;

    bb -= (bb >> 1) & k1;
    bb = (bb & k2) + ((bb >> 2) & k2);
    bb = (bb + (bb >> 4)) & k3;
    return (bb * k4) >> 56;
}

// faster version assuming not more than 15 bits set, used in mobility
// eval, posted on CCC forum by Marco Costalba of Stockfish team

int PopCnt15(U64 bb) {
    unsigned w = unsigned(bb >> 32), v = unsigned(bb);
    v -= (v >> 1) & 0x55555555; // 0-2 in 2 bits
    w -= (w >> 1) & 0x55555555;
    v = ((v >> 2) & 0x33333333) + (v & 0x33333333); // 0-4 in 4 bits
    w = ((w >> 2) & 0x33333333) + (w & 0x33333333);
    v += w; // 0-8 in 4 bits
    v *= 0x11111111;
    return int(v >> 28);
}

int PopCntSparse(U64 bb) // version faster on sparsely populated bitboards
{
    int count = 0;
    while (bb) {
        count++;
        bb &= bb - 1;
    }
    return count;
}
#endif
