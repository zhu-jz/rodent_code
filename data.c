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
static const int n_mob_mg[28] = { -160-40, -120-20,  -60,  00,  40,  80,  120, 160, 160};
static const int n_mob_eg[28] = { -160-40, -120-20,  -60,  00,  40,  80,  120, 160, 160};
static const int b_mob_mg[28] = { -160-40, -120-20,  -80, -40, -20,  00,  20,  40,  60,  80, 100, 120, 140, 140, 160                                 };
static const int b_mob_eg[28] = { -160-40, -120-20,  -80, -40, -20,  00,  20,  40,  60,  80, 100, 120, 140, 140, 160                                 };
static const int r_mob_mg[28] = { -100-40,  -80-20,  -60,  40,  20,  00,  20,  40,  60,  80, 100, 100, 110, 120, 130,                                };
static const int r_mob_eg[28] = { -200-40, -160-20, -120,  80,  40,  00,  40,  80, 120, 160, 200, 200, 220, 240, 260,                                };
static const int q_mob_mg[28] = {  -60-40,  -40-20,  -20,  00,  00,  10,  10,  10,  20,  20,  20,  30,  30,  30,  30,  40,  40,  40,  40,  50,  50,  50,  50 };
static const int q_mob_eg[28] = { -120-40,  -80-20,  -40, -20,  00,  10,  20,  30,  40,  50,  60,  60,  70,  70,  80,  80,  80,  90,  90,  90, 100, 100, 100 };

void sData::InitBadBishop(void)
{
   // first clear bad bishop masks
   for (int side = 0; side < 2; side++) {
      for (int sq = 0; sq < 64; sq++) {  
         bbBadBishopMasks [side][sq] = 0ULL;
         badBishopPenalty[side][sq] = 0;
      }
   }

   SetBadBishopMask(F1, E2, -20);
   SetBadBishopMask(C1, D2, -20);
   SetBadBishopMask(A2, B3, -10); SetBadBishopMask(A2, C4, -10);
   SetBadBishopMask(B2, C3, -5);
   SetBadBishopMask(C2, D3, -5);  SetBadBishopMask(C2, E4, -5);
   SetBadBishopMask(D2, E3, -5);
   SetBadBishopMask(E2, D3, -5);
   SetBadBishopMask(F2, E3, -5);  SetBadBishopMask(F2, D4, -5);
   SetBadBishopMask(G2, F3, -5);
   SetBadBishopMask(H2, G3, -10); SetBadBishopMask(H2, F4, -10);
   SetBadBishopMask(A3, B4, -5);  SetBadBishopMask(A3, C5, -5);
   SetBadBishopMask(B3, C4, -10); SetBadBishopMask(B3, D5, -10);
   SetBadBishopMask(C3, D4, -5);
   SetBadBishopMask(D3, E4, -5);
   SetBadBishopMask(E3, D4, -5);
   SetBadBishopMask(F3, E4, -5);
   SetBadBishopMask(G3, F4, -10); SetBadBishopMask(G3, E5, -10);
   SetBadBishopMask(H3, G4, -5);  SetBadBishopMask(H3, F5, -5);
   SetBadBishopMask(C4, D5, -3);
   SetBadBishopMask(F4, E5, -3);
}

void sData::SetBadBishopMask(int bishSq, int pawnSq, int val)
{
   bbBadBishopMasks[WHITE][bishSq] ^= SqBb(pawnSq);
   bbBadBishopMasks[BLACK][ REL_SQ(bishSq,BLACK) ] ^= RelSqBb(pawnSq, BLACK);
   badBishopPenalty[WHITE][bishSq] = val;
   badBishopPenalty[BLACK][REL_SQ(bishSq,BLACK)] = val;
}

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
		 straightDistance[i][j] = 8 - (((Abs(Rank(i) - Rank(j)) + Abs(File(i) - File(j)))));
         }
     }
}

void sData::InitOptions(void) // init user-accessible stuff
{
   safetyStyle     = KS_QUADRATIC;
   ownAttack       = 100; // WAS 100
   oppAttack       = 100; // WAS 100
   ownMobility     = 100;
   oppMobility     = 110; // WAS 100
   passedPawns     = 105; // 100 is worse, 110 might be marginally better
   pawnStruct      = 100; // WAS 100
   bishopPair      = 50; 
   verbose         = 0;       // no additional display
   elo             = MAX_ELO; // no weakening
   contempt        = 12;
   isAnalyzing     = 0;
   useBook         = 1;
   useWeakening    = 0;
   useLearning     = 0;
   bookFilter      = 10;
   tropismWeight   = 0;
   timeDivisor     = 36;      // WAS 40
}

// used at the beginning of search to set scaling factors for eval components
void sData::InitAsymmetric(int side) 
{
   mobSidePercentage[side]      = ownMobility;
   mobSidePercentage[Opp(side)] = oppMobility;
   attSidePercentage[side]      = ownAttack;
   attSidePercentage[Opp(side)] = oppAttack;
}
