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

#include "stdio.h"
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

const int safety[ 256 ] = { // handmade king safety values
	  0,     1,    1,    2,    2,    3,    3,    4,   4,     5,
	  6,     7,    8,    9,   10,   11,   11,   12,   12,   13,
	 13,    14,   14,   15,   16,   17,   18,   19,   20,   22,
	 24,    26,   28,   30,   32,   34,   36,   39,   42,   45,
	 48,    52,   56,   60,   64,   68,   72,   76,   80,   83,
	 86,    89,   92,   94,   96,   98,  100,  101,  102,  103,
	104,   105,  106,  107,  108,  109,  110,  111,  112,  113,
	114,   115,  116,  117,  118,  119,  120,  121,  122,  123,
	124,   125,  126,  127,  128,  129,  130,  131,  132,  133,
	134,   135,  136,  137,  138,  139,  140,  141,  142,  143,
	144,   145,  146,  147,  148,  149,  150,  151,  152,  153,
	154,   155,  156,  157,  158,  159,  160,  161,  162,  163,
	164,   165,  166,  167,  168,  169,  170,  171,  172,  173,
	174,   175,  176,  177,  178,  179,  180,  181,  182,  183,
	184,   185,  186,  187,  188,  189,  190,  191,  192,  193,
	194,   195,  196,  197,  198,  199,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200,  200,  200,  200,  200,
	200,   200,  200,  200,  200,  200                       };

// mobility values
static const int n_mob_mg[28] = {-16-4, -8-2, -4, +0, +4, +8,+11,+13,+14,+14,+14,+14,+14,+14,+14,+14 };
static const int n_mob_eg[28] = {-14-4, -7-2, -3, +0, +3, +6, +8, +9,+10,+10,+10,+14,+14,+14,+14,+14 };
static const int b_mob_mg[28] = {-20,-15,-10, -5, +0, +5, +9,+12,+14,+15,+16,+17,+18,+19,+19,+19 };
static const int b_mob_eg[28] = {-22,-17,-12, -7, -2, +3, +7,+10,+12,+13,+14,+15,+16,+17,+18,+19 };
static const int r_mob_mg[28] = {-10, -8, -6, -4, -2, +0, +2, +4, +5, +6, +7, +7, +7, +7, +7, +7 };
static const int r_mob_eg[28] = {-20,-16,-12, -8, -4, +0, +4, +8,+12,+15,+17,+18,+19,+20,+21,+22 };
static const int q_mob_mg[32] = { -6, -5, -4, -3, -2, -1, +0, +1, +2, +3, +4, +4, +5, +5, +6, +6,
                                  +7, +7, +7, +7, +7, +7, +7, +7, +7, +7, +7, +7, +7, +7, +7, +7 };
static const int q_mob_eg[32] = {-12, -10, -8 -6, -4, -2, +0, +2, +4, +6, +7, +8, +9,+10,+11,+11,
                                 +12,+12,+12,+12,+12,+12,+12,+12,+12,+12,+12,+12,+12,+12,+12,+12 };

void sData::InitSinglePiece(int pc, int mat, int del, int phase) 
{
   matValue[pc]      = mat;   
   deltaValue[pc]    = del;
   phaseValue[pc]    = phase;         
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

void sData::InitCastleMask(void) 
{
   for (int sq = 0; sq < 64; sq++) castleMask[sq] = 15;
   castleMask[A1] = 13;
   castleMask[E1] = 12;
   castleMask[H1] = 14;
   castleMask[A8] = 7;
   castleMask[E8] = 3;
   castleMask[H8] = 11;
}

void sData::InitSearchData(void) 
{
   useNull          = 1;
   lmrHistLimit     = 60;      // 70 is better in very fast games (10s per game)
   deltaMargin      = 150;
   goodCaptMargin   = 30;      // BxN is OK, RxB isn't
}

void sData::InitDistanceBonus(void) 
{
   for (int i = 0; i < 64; ++i) {
      for (int j = 0; j < 64; ++j) {
         distance[i][j] = 14 - ( Abs( Rank(i) - Rank(j) ) + Abs( File(i) - File(j) ) );
      }
   }
}

void sData::InitOptions(void) // init user-accessible stuff
{
   safetyStyle  = KS_QUADRATIC;
   ownMobility  = 110; // WAS 110
   oppMobility  = 110; // WAS 110
   ownAttack    = 100; // WAS 100
   oppAttack    = 100; // WAS 100
   passedPawns  = 105; // 100 is worse, 110 might be marginally better
   pawnStruct   = 100;
   bishopPair   = 50; 
   verbose      = 0;       // no additional display
   elo          = MAX_ELO; // no weakening
   contempt     = 12;
   isAnalyzing  = 0;  
   useBook      = 1;
   useWeakening = 0;
   useLearning  = 0;
   bookFilter   = 10;
   lazyMargin   = 220;
}

// used at the beginning of search to set scaling factors for eval components
void sData::InitAsymmetric(int side) 
{
   int oppo = Opp(side);
   mobSidePercentage[side] = ownMobility;
   mobSidePercentage[oppo] = oppMobility;
   attSidePercentage[side] = ownAttack;
   attSidePercentage[oppo] = oppAttack;
   for (int i = 0; i <=255; i++) {
	  if (safetyStyle == KS_QUADRATIC) {
         danger[side][i] = ( kingDanger[i] * attSidePercentage[side] ) / 100;
         danger[oppo][i] = ( kingDanger[i] * attSidePercentage[oppo] ) / 100;
	  }
	  if (safetyStyle == KS_HANDMADE) {
         danger[side][i] = ( safety[i] * attSidePercentage[side] ) / 100;
         danger[oppo][i] = ( safety[i] * attSidePercentage[oppo] ) / 100;
	  }
   }
}
