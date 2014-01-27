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

#pragma once

enum ePanelStyle   { PANEL_NORMAL, PANEL_POWER };
enum eSafetyStyle  { KS_QUADRATIC, KS_HANDMADE };
enum ePawnProperty { PASSED, CANDIDATE, PHALANX, ISOLATED, BACKWARD, PAWN_PROPERTIES };

struct sData {
public:

 // eval data
 int matValue[7];
 int deltaValue[7];
 int phaseValue[7];
 int distance[64][64];
 int pstMg[2][6][64];
 int pstEg[2][6][64];
 int outpost[2][6][64];
 int pawnProperty [PAWN_PROPERTIES][2][2][64];
 int mobBonusMg[6][28];
 int mobBonusEg[6][28];
 int mobSidePercentage[2];
 int attSidePercentage[2];
 int ownMobility;
 int oppMobility;
 int ownAttack;
 int oppAttack;
 int passedPawns;
 int pawnStruct;
 int bishopPair;
 int badBishopPenalty[2][64];
 int kingDanger[100];
 int safetyStyle;
 int contempt;
 int tropismWeight;
 int timeDivisor;

 // search data
 int useNull;          // shall we use null move?
 int lmrHistLimit;     // what value of history counter prevents lmr?
 int deltaMargin;      // margin for a delta pruning in quiescence search 
 int goodCaptMargin;   // margin of a loss that can be incurred without classifying capture as "bad"
 int lazyMargin;       // margin for lazy evaluation cutoff
 int verbose;          // shall we output more information about search than bare minimum?
 int useLearning;      // shall we use position learning?
 int isAnalyzing;
 int useBook;

 // book data
 int bookFilter;

 // modus operandi
 char styleList[512];
 char levelList[512];
 char bookList[512];
 char currLevel[32];
 char currStyle[32];
 char currBook[32];
 int panelStyle;
 int useWeakening;
 int elo;

 // functions
 void InitSinglePiece(int pc, int mat, int del, int phase);
 void InitPstValues(void);
 void InitMobBonus(void);
 void InitCastleMask(void);
 void InitSearchData(void);
 void InitDistanceBonus(void);
 void InitOptions(void);
 void InitBadBishop(void);
 void InitAsymmetric(int side);
 void SetBadBishopMask(int bishSq, int pawnSq, int val);
 int GetPawnMgPst(int sq);
 int GetRookMgPst(int sq);
 int GetPhalanxPstMg(int sq);
};

extern struct sData Data;
