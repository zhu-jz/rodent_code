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
#include "bitboard/bitboard.h"
#include "data.h"
#include "rodent.h"

U64 bbRAttacksOnEmpty[64];
U64 bbBAttacksOnEmpty[64];
U64 bbRCanAttack[64][64];
U64 bbBCanAttack[64][64];
U64 bbQCanAttack[64][64];
U64 bbPawnSupport[2][64];

void Init(void)
{

  	for (int t=0, i=1; i < 100; ++i)
    {
        t = Min(1280, Min(int(0.4 * i * i), t + 30));
		Data.kingDanger[i] = (t * 100) / 256;
	}

  InitKindergartenBitboards();
  InitPawnAttacks();
  InitKnightAttacks();
  InitKingAttacks();     
  InitPassedMask();      // passed pawn bitmasks initialization
  InitAdjacentMask();
  InitZobrist();
  InitPossibleAttacks();
  InitPawnSupport();
  Data.InitBadBishop();  // bad bishop bitboards initialization
  Data.InitEvalVars();
  //                 name,  val, delta, phase
  Data.InitSinglePiece( P,  0,     150,  0);  // pawn material is evaluated in eval.c, so no value set here
  Data.InitSinglePiece( N,  325,   475,  1);
  Data.InitSinglePiece( B,  335,   485,  1);
  Data.InitSinglePiece( R,  500,   650,  2);  // 510 is worse
  Data.InitSinglePiece( Q,  975,  1125,  4);
  Data.InitSinglePiece(NO_TP, 0,   150,  0);
  Data.InitPstValues();
  Data.InitMobBonus();
  Data.InitCastleMask();
  Data.InitSearchData();
  Data.InitDistanceBonus();
}

void InitZobrist() 
{
  for (int i = 0; i < 12; i++)
    for (int j = 0; j < 64; j++)
      zobPiece[i][j] = Random64();

  for (int i = 0; i < 16; i++)
    zobCastle[i] = Random64();

  for (int i = 0; i < 8; i++)
    zobEp[i] = Random64();
}
