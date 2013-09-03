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

#pragma once

#define GRAIN_SIZE 4

struct sPawnHashEntry {
  U64 pawnKey;
  int mgPawns;
  int egPawns;
};

#define PAWN_HASH_SIZE  512 * 512

struct sEvaluator {
private:
  int attScore[2];         // king attack scores
  int mgMobility[2];       // midgame mobility scores
  int egMobility[2];       // endgame mobility scores
  int mgMisc[2];           // miscelanneous midgame scores
  int egMisc[2];           // miscelanneous endgame scores
  int pawnScoreMg[2];      // midgame pawn structure scores
  int pawnScoreEg[2];      // endgame pawn structure scores
  U64 bbPawnControl[2];    // squares controlled by pawns, used in mobility eval (pawn eval uses only occupancy masks)
  U64 bbPawnCanControl[2]; // squares that can be controlled by pawns as they advance, used in outpost eval
  U64 kingDiagChecks[2];
  U64 kingStraightChecks[2];
  U64 kingKnightChecks[2];
  U64 bbAllAttacks[2];     // squares attacked by a side
  int attCount[2];         // attack counter based on square control
  int attNumber[2];        // no. of pieces participating in the attack
  int mgFact,  egFact;     // material-driven scaling factors
  int mgScore, egScore;    // partial midgame and endgame scores (to be scaled)
  int degradation;         // for scaling down the score in endgames that are hard to win

  sPawnHashEntry PawnTT[PAWN_HASH_SIZE]; // pawn transposition table

  int GetMaterialScore(sPosition *p);
  void AddMobility(int pc, int side, int cnt);
  void AddMiscOne(int side, int val);
  void AddMiscTwo(int side, int mg, int eg);
  void AddPieceAttack(int side, int val);
  void AddPawnProperty(int pawnProperty, int side, int sq);
  int CheckmateHelper(sPosition *p);
  void InitStatic(void);            
  void InitDynamic(sPosition *p);            
  void SetScaleFactor(sPosition *p);
  int SetDegradationFactor(sPosition *p, int stronger);
  int Interpolate(void);
  int ScorePatterns(sPosition *p, int side);
  void SinglePawnScore(sPosition *p, int side); // eval_pawns.c
  void EvalPawnCenter(sPosition *p, int side);  // eval_pawns.c
  void EvalPawns(sPosition *p);				    // eval_pawns.c
  void ScoreN(sPosition *p, int side);
  void ScoreB(sPosition *p, int side);
  void ScoreR(sPosition *p, int side);
  void ScoreQ(sPosition *p, int side);
  void ScoreP(sPosition *p, int side);
  void ScoreK(sPosition *p, int side);
  void ScoreOutpost(sPosition *p, int side, int piece, int sq);
  void ScoreHanging(sPosition *p, int side);
  int  EvalFileShelter(U64 bbOwnPawns, int side);
  int  EvalFileStorm(U64 bbOppPawns, int side);
  int  EvalTrappedKnight(sPosition *p);
  int  EvalTrappedBishop(sPosition *p, int side);
  int  EvalTrappedRook(sPosition *p, int side);
  int  PullToDraw(sPosition *p, int score);
public:
  int Normalize(int val, int limit);
  void ScaleValue(int * value, int factor);
  void DebugPst(sPosition *p);
  int ReturnFast(sPosition *p);
  int Return(sPosition *p, int alpha, int beta);
};

extern struct sEvaluator Eval;

int NotOnBishColor(sPosition * p, int bishSide, int sq);
int BishopsAreDifferent(sPosition * p);

int MaterialMinor(sPosition *p, int side);
int MaterialKnight(sPosition *p, int side);
int MaterialBishop(sPosition *p, int side);
int MaterialRook(sPosition *p, int side);
int MaterialRookMinor(sPosition *p, int side);
int MaterialQueen(sPosition *p, int side);
int MaterialQueenMinor(sPosition *p, int side);
int MaterialBB(sPosition *p, int side);
int MaterialNN(sPosition *p, int side);
int MaterialBN(sPosition *p, int side);