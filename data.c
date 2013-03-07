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
#include <math.h>
#include "data.h"
#include "bitboard/bitboard.h"
#include "rodent.h"

U64 bbLineMask[4][64];
U64 attacks[4][64][64];
U64 bbPawnAttacks[2][64];
U64 bbKnightAttacks[64];
U64 bbKingAttacks[64];
U64 bbKingZone[2][64];
U64 bbPassedMask[2][64];
U64 bbAdjacentMask[8];
U64 bbBadBishopMasks[2][64];
int castleMask[64];

// used for a mathematical trick in FirstOne
const int bitTable[64] = {
   0,  1,  2,  7,  3, 13,  8, 19,
   4, 25, 14, 28,  9, 34, 20, 40,
   5, 17, 26, 38, 15, 46, 29, 48,
  10, 31, 35, 54, 21, 50, 41, 57,
  63,  6, 12, 18, 24, 27, 33, 39,
  16, 37, 45, 47, 30, 53, 49, 56,
  62, 11, 23, 32, 36, 44, 52, 55,
  61, 22, 43, 51, 60, 42, 59, 58
};

U64 zobPiece[12][64];
U64 zobCastle[16];
U64 zobEp[8];
int pondering;
char ponder_str[6];

// mobility values
static const int n_mob_mg[28] = { -16-4, -12-2, -8,  0,  4,  8,  12, 16, 16};
static const int n_mob_eg[28] = { -16-4, -12-2, -8,  0,  4,  8,  12, 16, 16};
static const int b_mob_mg[28] = { -16-4, -12-2, -8, -4, -2,  0,  2,  4,  6,  8, 10, 12, 14, 14, 16                                 };
static const int b_mob_eg[28] = { -16-4, -12-2, -8, -4, -2,  0,  2,  4,  6,  8, 10, 12, 14, 14, 16                                 };
static const int r_mob_mg[28] = {  -6-4,  -4-2, -2,  0,  2,  4,  6,  8,  9, 10, 11, 12, 13, 14, 15                                 };
static const int r_mob_eg[28] = { -12-4,  -8-2, -4,  0,  4,  8, 12, 16, 18, 20, 22, 24, 26, 28, 30                                 };
static const int q_mob_mg[28] = {  -6-4,  -4-2, -2,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  5,  5 };
static const int q_mob_eg[28] = { -12-4,  -8-2, -4, -2,  0,  1,  2,  3,  4,  5,  6,  6,  7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10 };


void sData::InitBadBishop(void)
{
	 // first clear bad bishop masks
	 for (int side = 0; side < 2; side++) {
	    for (int i = 0; i < 64; i++) {  
			 bbBadBishopMasks [side][i] = 0ULL;
		 }
	 }

	 SetBadBishopMask(F1, E2);
	 SetBadBishopMask(C1, D2);
	 SetBadBishopMask(A2, B3); SetBadBishopMask(A2, C4);
	 SetBadBishopMask(B2, C3);
	 SetBadBishopMask(C2, D3); SetBadBishopMask(C2, E4);
	 SetBadBishopMask(D2, E3);
	 SetBadBishopMask(E2, D3);
	 SetBadBishopMask(F2, E3); SetBadBishopMask(F2, D4);
	 SetBadBishopMask(G2, F3);
	 SetBadBishopMask(H2, G3); SetBadBishopMask(H2, F4);
	 SetBadBishopMask(A3, B4); SetBadBishopMask(A3, C5);
	 SetBadBishopMask(B3, C4); SetBadBishopMask(B3, D5);
	 SetBadBishopMask(C3, D4);
	 SetBadBishopMask(D3, E4);
	 SetBadBishopMask(E3, D4);
	 SetBadBishopMask(F3, E4);
	 SetBadBishopMask(G3, F4); SetBadBishopMask(G3, E5);
	 SetBadBishopMask(H3, G4); SetBadBishopMask(H3, F5);
	 SetBadBishopMask(C4, D5);
	 SetBadBishopMask(F4, E5);
}

void sData::SetBadBishopMask(int bishSq, int pawnSq)
{
     bbBadBishopMasks[WHITE][bishSq] ^= SqBb(pawnSq);
	 bbBadBishopMasks[BLACK][ REL_SQ(bishSq,BLACK) ] ^= RelSqBb(pawnSq, BLACK);
}

void sData::InitSinglePiece(int pc, int mat, int del, int phase, int att) 
{
     matValue[pc]      = mat;   
	 deltaValue[pc]    = del;
	 phaseValue[pc]    = phase;     
	 attMultiplier[pc] = att;      
}

void sData::InitMobBonus(void) 
{
	 for (int i = 0; i < 28; i++) {
		 mobBonusMg[N][i] = n_mob_mg[i];
		 mobBonusEg[N][i] = n_mob_eg[i];
		 mobBonusMg[B][i] = b_mob_mg[i];
		 mobBonusEg[B][i] = b_mob_eg[i];
		 mobBonusMg[R][i] = r_mob_mg[i];
		 mobBonusEg[R][i] = r_mob_eg[i];
		 mobBonusMg[Q][i] = q_mob_mg[i];
		 mobBonusEg[Q][i] = q_mob_eg[i];
	 }
}

void sData::InitAttackBonus(void) 
{
    int startCurve = 10; // 7 is worse
	int logMult    = 20; // 20 is OK, raising doesn't seem to help

	 for (int i=0; i < 256; i++)
     attackBonus[i] = i / 3;

	 for (int i=0; i < 256; i++) {
		attackBonus[i+startCurve] = logMult * log((double) i) + i / 2;
		attackBonus[startCurve] = startCurve / 2;
		attackBonus[startCurve+1] = startCurve / 2+2;
	    //printf("%2d %d\n", i, attackBonus[i] );
  }
}

void sData::InitCastleMask(void) 
{
  for (int i = 0; i < 64; i++) castleMask[i] = 15;
  castleMask[A1] = 13;
  castleMask[E1] = 12;
  castleMask[H1] = 14;
  castleMask[A8] = 7;
  castleMask[E8] = 3;
  castleMask[H8] = 11;
}

void sData::InitSearchData(void) 
{
     // History of moveIsLate:lmrStep tuning:
	 // 10:5 best, 8:4 best, 8:5 fails, 6:4 best, 5:4 fails)

     aspiration       = 30;
	 useNull          = 1;
	 minimalNullDepth = ONE_PLY;
	 minimalLmrDepth  = 2 * ONE_PLY;
	 moveIsLate       = 6;
	 lmrStep          = 4; 
	 lmrHistLimit     = 70;      // 70 is better than 60
	 futilityBase     = 100;
	 futilityStep     = 20;
	 futilityDepth    = 4;
	 useDeltaPruning  = 1;
	 deltaMargin      = 150;
	 goodCaptMargin   = 30;      // BxN is OK, RxB isn't
	 lazyMargin       = 180;
	 verbose          = 0;       // no additional display
	 elo              = MAX_ELO; // no weakening
	 isAnalyzing      = 0;
	 useBook          = 1;
	 useWeakening     = 0;
	 useLearning      = 0;
}

void sData::InitDistanceBonus(void) 
{
    for (int i = 0; i < 64; ++i) {
         for (int j = 0; j < 64; ++j) {
         distance[i][j] = 14 - ( Abs( Rank(i) - Rank(j) ) + Abs( File(i) - File(j) ) );
         }
     }
}

void sData::InitMaterialValues(void) 
{
	//             name,  val, del, pha, att
	 InitSinglePiece( P,  0,   150,  0,   0 );  // pawn material is evaluated in eval.c
	 InitSinglePiece( N,  325, 475,  1,   2 );
	 InitSinglePiece( B,  335, 485,  1,   2 );
	 InitSinglePiece( R,  500, 650,  2,   4 );  // 510 is worse
	 InitSinglePiece( Q,  975, 1125, 4,   7 );
	 InitSinglePiece(NO_TP, 0, 150,  0,   0 );
};

void sData::InitEvalVars(void) 
{
	 ownMobility     = 110;
	 oppMobility     = 110;
	 ownAttack       = 100; 
	 oppAttack       = 100; 
	 bishopPair      =  50; 
	 doubledPawn[MG] = -20;
     doubledPawn[EG] = -10;
	 rookOpenMg      = 10;
     rookOpenEg      = 10;
	 rookSemiOpenMg  = 5;
	 rookSemiOpenEg  = 5;
	 rookOpenAttack  = 10;
	 rookSemiOpenAttack = 5;
	 rookSeventhMg   = 20;
	 rookSeventhEg   = 20;
	 pawnIsolatedOnOpen = -15;
	 pawnBackwardOnOpen = -15;
}

// used at the beginning of search to set scaling factors for eval components
void sData::InitAsymmetric(int side) 
{
     mobSidePercentage[side]      = ownMobility;
     mobSidePercentage[Opp(side)] = oppMobility;
     attSidePercentage[side]      = ownAttack;
     attSidePercentage[Opp(side)] = oppAttack;
}