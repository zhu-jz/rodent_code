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

/* Compiler switches activating faster instructions for newer 64-bit architectures
   - patch  supplied  by Dann Corbit, 17.02.2012. Asm  version  added  30.01.2013.
   If both USE_FIRST_ONE_ASM  and USE_FIRST_ONE_INTRINSICS are defined, the latter 
   takes precedence.
*/

#pragma once

// #define USE_MM_POPCNT
// #define USE_FIRST_ONE_INTRINSICS
#define USE_FIRST_ONE_ASM

// portable definition of an unsigned 64-bit integer
#if defined(_WIN32) || defined(_WIN64)
    typedef unsigned long long U64;
#else
    #include <stdint.h>
    typedef uint64_t U64;
#endif

#define bbEmpty			(U64)0ULL
#define bbWhiteSq		(U64)0x55AA55AA55AA55AA
#define bbBlackSq		(U64)0xAA55AA55AA55AA55

#define bbRANK_1		(U64)0x00000000000000FF
#define bbRANK_2		(U64)0x000000000000FF00
#define bbRANK_3		(U64)0x0000000000FF0000
#define bbRANK_4		(U64)0x00000000FF000000
#define bbRANK_5		(U64)0x000000FF00000000
#define bbRANK_6		(U64)0x0000FF0000000000
#define bbRANK_7		(U64)0x00FF000000000000
#define bbRANK_8		(U64)0xFF00000000000000

#define bbFILE_A		(U64)0x0101010101010101
#define bbFILE_B		(U64)0x0202020202020202
#define bbFILE_C		(U64)0x0404040404040404
#define bbFILE_D		(U64)0x0808080808080808
#define bbFILE_E		(U64)0x1010101010101010
#define bbFILE_F		(U64)0x2020202020202020
#define bbFILE_G		(U64)0x4040404040404040
#define bbFILE_H		(U64)0x8080808080808080

#define bbNotA          (U64)0xfefefefefefefefe // ~bbFILE_A
#define bbNotH          (U64)0x7f7f7f7f7f7f7f7f // ~bbFILE_H
#define bbRim           (U64)(bbFILE_A | bbFILE_H | bbRANK_1 | bbRANK_8)
#define bbCentralFile   (U64)(bbFILE_C | bbFILE_D | bbFILE_E | bbFILE_F)

#define SqBb(x)         ((U64)1 << (x)) // returns bitboard with a bit of square "x" set
#define ShiftNorth(x)   (x<<8)
#define ShiftSouth(x)   (x>>8)
#define ShiftWest(x)    ((x & bbNotA)>>1)
#define ShiftEast(x)    ((x & bbNotH)<<1)
#define ShiftNW(x)      ((x & bbNotA)<<7)
#define ShiftNE(x)      ((x & bbNotH)<<9)
#define ShiftSW(x)      ((x & bbNotA)>>9)
#define ShiftSE(x)      ((x & bbNotH)>>7)

#define REL_SQ(sq,cl) ( sq ^ (cl * 56) )

// Compiler and architecture dependent versions of FirstOne() function, 
// triggered by defines at the top of this file.
#ifdef USE_FIRST_ONE_INTRINSICS
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)

static int __forceinline FirstOne(U64 x) {
    unsigned long index = -1;
    _BitScanForward64(&index, x);
    return index;
}
#else
	#ifdef USE_FIRST_ONE_ASM
		#define FirstOne(x) FirstOneAsm(x)
	#else
		#define FirstOne(x)     bitTable[(((x) & (~(x) + 1)) * (U64)0x0218A392CD3D5DBF) >> 58] // first "1" in a bitboard
	#endif
#endif

void PrintBb(U64 bbTest);
extern int PopCnt(U64);
extern int PopCnt15(U64 bb);
extern int PopCntSparse(U64);

extern U64 ShiftFwd(U64 bb, int side);

U64 FillKing(U64 bb);
U64 FillNorth(U64 bb);
U64 FillSouth(U64 bb);
U64 FillWest(U64 bb);
U64 FillEast(U64 bb);
U64 FillNW(U64 bb);
U64 FillNE(U64 bb);
U64 FillSW(U64 bb);
U64 FillSE(U64 bb);

U64 FillNorthExcl(U64 bb);
U64 FillSouthExcl(U64 bb);
U64 FillWestExcl(U64 bb);
U64 FillEastExcl(U64 bb);
U64 FillNWExcl(U64 bb);
U64 FillNEExcl(U64 bb);
U64 FillSWExcl(U64 bb);
U64 FillSEExcl(U64 bb);

U64 flipVertical(U64 bb);

U64 GetFrontSpan(U64 bb, int side);
U64 GetRearSpan(U64 bb, int side);
U64 GetWPControl(U64 bb);
U64 GetBPControl(U64 bb);
U64 GetPawnAttacks(int side, U64 bb);

int PopFirstBit(U64 *bb);
int PopLastBit(U64 *bb);
int PopNextBit(int side, U64 *bb);
int FirstOneAsm(U64 bb);
int LastOneAsm(U64 bb);

void InitKindergartenBitboards(void);
void InitPawnAttacks(void);
void InitKnightAttacks(void);
void InitKingAttacks(void);
void InitPassedMask(void);
void InitAdjacentMask(void);
void InitZobrist(void);
void InitPossibleAttacks(void);
void InitPawnSupport(void);