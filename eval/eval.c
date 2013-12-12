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

#define LAZY_EVAL
//#define HASH_EVAL

#include <stdio.h>
#include "../bitboard/bitboard.h"
#include "../data.h"
#include "../rodent.h"
#include "eval.h"

const int attMult[15]     = {0, 0, 4, 10, 14, 26, 42, 52, 64, 70, 76, 82, 86, 92, 100};

void sEvaluator::InitStaticScore(void) 
{
     mgScore            = 0;   egScore            = 0;  // clear midgame/endgame score component
     pawnScoreMg[WHITE] = 0;   pawnScoreMg[BLACK] = 0;  // clear midgame pawn scores
     pawnScoreEg[WHITE] = 0;   pawnScoreEg[BLACK] = 0;  // clear endgame pawn scores	 
}

void sEvaluator::InitDynamicScore(sPosition *p) 
{
     attScore[WHITE]         = 0;   attScore[BLACK]    = 0;  // clear attack scores
	 attNumber[WHITE]        = 0;   attNumber[BLACK]   = 0;  // clear no. of attackers
	 attCount[WHITE]         = 0;   attCount[BLACK]    = 0;
	 checkCount[WHITE]       = 0;   checkCount[BLACK]  = 0;
	 attWood[WHITE]          = 0;   attWood[BLACK]     = 0;  // clear "wood weight" of attack
	 mgMisc[WHITE]           = 0;   mgMisc[BLACK]      = 0;  // clear miscelanneous midgame scores
	 egMisc[WHITE]           = 0;   egMisc[BLACK]      = 0;  // clear miscelanneous endgame scores
	 mgMobility[WHITE]       = 0;   mgMobility[BLACK]  = 0;  // clear midgame mobility
	 egMobility[WHITE]       = 0;   egMobility[BLACK]  = 0;  // clear endgame mobility	 
	 bbPawnControl[WHITE]    = GetWPControl( bbPc(p, WHITE, P) );
	 bbPawnControl[BLACK]    = GetBPControl( bbPc(p, BLACK, P) );
	 bbPawnCanControl[WHITE] = FillNorth( bbPawnControl[WHITE] );
	 bbPawnCanControl[BLACK] = FillSouth( bbPawnControl[BLACK] );
	 bbAllAttacks[WHITE]     = bbPawnControl[WHITE];
	 bbAllAttacks[BLACK]     = bbPawnControl[BLACK];
	 bbMinorCoorAttacks[WHITE]   = 0ULL;
	 bbMinorCoorAttacks[BLACK]   = 0ULL;
	 
	 // set squares from which king can be checked 
	 U64 bbOccupied = OccBb(p);
	 bbStraightChecks[WHITE] = RAttacks(bbOccupied, KingSq(p, WHITE) );  
	 bbStraightChecks[BLACK] = RAttacks(bbOccupied, KingSq(p, BLACK) );  
	 bbDiagChecks[WHITE]     = BAttacks(bbOccupied, KingSq(p, WHITE) );  
	 bbDiagChecks[BLACK]     = BAttacks(bbOccupied, KingSq(p, BLACK) );  
	 bbKnightChecks[WHITE]   = bbKnightAttacks[KingSq(p, WHITE)];
	 bbKnightChecks[BLACK]   = bbKnightAttacks[KingSq(p, BLACK)];
}

void sEvaluator::SetScaleFactor(sPosition *p) 
{
     mgFact = Min( p->phase, 24); // normalize for opening material
     egFact = 24 - mgFact;
}

// Interpolate between mgScore and egScore, depending on remaining material 
int sEvaluator::Interpolate(void) {
    return ( (mgFact * mgScore ) / 24 ) + ( (egFact * egScore ) / 24 );
}

void sEvaluator::ScoreHanging(sPosition *p, int side)
{
	U64 bbHanging    = p->bbCl[Opp(side)] & ~bbPawnControl[Opp(side)]; 
	U64 bbThreatened = p->bbCl[Opp(side)] & bbPawnControl[side];
	bbHanging |= bbThreatened;            // piece attacked by our pawn isn't well defended
	bbHanging &= bbAllAttacks[side];      // obviously, hanging piece has to be attacked
	bbHanging &= ~bbPc(p, Opp(side), P);  // currently we don't evaluate threats against pawns

	U64 bbSpace = UnoccBb(p) & bbAllAttacks[side];
	bbSpace &= ~bbRelRank[side][RANK_1];    // controlling home ground is not space advantage
	bbSpace &= ~bbRelRank[side][RANK_2];
	bbSpace &= ~bbRelRank[side][RANK_3];
	bbSpace &= ~bbPawnControl[Opp(side)]; // squares attacked by enemy pawns aren't effectively controlled
	AddMisc(side, PopCnt(bbSpace), 0);
	int pc, sq, val;

    while (bbHanging) {
       sq  = FirstOne(bbHanging);
	   pc  = TpOnSq(p, sq);
	   val = Data.matValue[pc] / 64;
	   AddMisc(side, 10+val, 18+val);
	   bbHanging &= bbHanging - 1;
	}
}

void sEvaluator::ScoreKingShield(sPosition *p, int side)
{
  const U64 bbQSCastle[2] = { SqBb(A1) | SqBb(B1) | SqBb(C1) | SqBb(A2) | SqBb(B2) | SqBb(C2), 
	                          SqBb(A8) | SqBb(B8) | SqBb(C8) | SqBb(A7) | SqBb(B7) | SqBb(C7) };
  const U64 bbKSCastle[2] = { SqBb(F1) | SqBb(G1) | SqBb(H1) | SqBb(F2) | SqBb(G2) | SqBb(H2), 
                              SqBb(F8) | SqBb(G8) | SqBb(H8) | SqBb(F7) | SqBb(G7) | SqBb(H7) };
  const int qCastle[2] = {B1, B8};
  const int kCastle[2] = {G1, G8};
  const int sideMult[2] = {1,-1};

  U64 bbKingFile, bbNextFile;
  int result = 0;
  int sq = p->kingSquare[side];

  // use generic square for castled king to avoid changing shield score by, say, Kh1-g1
  if (SqBb(sq) & bbKSCastle[side]) sq = kCastle[side];
  if (SqBb(sq) & bbQSCastle[side]) sq = qCastle[side];

  // evaluate pawn shield and pawn storms
  bbKingFile = FillNorth(SqBb(sq) ) | FillSouth(SqBb(sq));
  result += EvalKingFile(p, side, bbKingFile);
   
  bbNextFile = ShiftEast(bbKingFile);
  if (bbNextFile) result += EvalKingFile(p, side, bbNextFile);
  
  bbNextFile = ShiftWest(bbKingFile);
  if (bbNextFile) result += EvalKingFile(p, side, bbNextFile);

  mgScore += result * sideMult[side]; // add shield score to midgame score
}

int sEvaluator::EvalKingFile(sPosition * p, int side, U64 bbFile)
{
  int result = 0;
  if (bbFile & bbCentralFile) result += (EvalFileShelter( bbFile & bbPc(p, side, P), side ) / 2);
  else                        result +=  EvalFileShelter( bbFile & bbPc(p, side, P), side );

  return result + EvalFileStorm  ( bbFile & bbPc(p, Opp(side), P), side );
}

void sEvaluator::ScoreKingAttacks(sPosition *p, int side) 
{
	bbAllAttacks[side] |= bbKingAttacks[KingSq(p, side) ];

	if (Data.safetyStyle == KS_SECONDARY)    {
	   if (attWood[side] > 14) attWood[side] = 14;
       if (attNumber[side] > 1 ) attWood[side] += 1;
       attScore[side] = ( (attCount[side]+checkCount[side]) * attMult[attWood[side]]) / 100;
	}
	if (Data.safetyStyle == KS_DOMINANT) {
		int attUnit = attCount[side];    // attacks on squares near enemy king
		attUnit += checkCount[side];     // check and contact check threats
		attUnit += (attWood[side] / 2);  // material involved in the attack
		if(attUnit > 99) attUnit = 99;   // bounds checking
		if(attNumber[side] < 2) attUnit = 0;
		attScore[side] = Data.kingDanger[attUnit] * 10;
	}
	ScaleValue(&attScore[side]  , Data.attSidePercentage[side]);
}

int sEvaluator::EvalFileShelter(U64 bbOwnPawns, int side) 
{
	if ( !bbOwnPawns ) return -36;
	if ( bbOwnPawns & bbRelRank[side][RANK_2] ) return    2;
	if ( bbOwnPawns & bbRelRank[side][RANK_3] ) return  -11;
	if ( bbOwnPawns & bbRelRank[side][RANK_4] ) return  -20;
	if ( bbOwnPawns & bbRelRank[side][RANK_5] ) return  -27;
	if ( bbOwnPawns & bbRelRank[side][RANK_6] ) return  -32;
	if ( bbOwnPawns & bbRelRank[side][RANK_7] ) return  -35;
	return 0;
}

int sEvaluator::EvalFileStorm(U64 bbOppPawns, int side)
{
	if (!bbOppPawns) return -15;
	if (bbOppPawns & bbRelRank[side][RANK_4] ) return -3;
	if (bbOppPawns & bbRelRank[side][RANK_5] ) return -5;
	if (bbOppPawns & bbRelRank[side][RANK_6] ) return -7;
    return 0;
}

int sEvaluator::ReturnFull(sPosition *p, int alpha, int beta)
{
#ifdef HASH_EVAL
	int addr = p->hashKey % EVAL_HASH_SIZE;
	if (EvalTT[addr].key == p->hashKey) {
		int hashScore = EvalTT[addr].score;
		return p->side == WHITE ? hashScore : -hashScore;
	}
#endif

  int fullEval = 0;
  int score = GetMaterialScore(p) + CheckmateHelper(p);
  p->side == WHITE ? score+=5 : score-=5;

  InitStaticScore();
  SetScaleFactor(p);
  EvalPawns(p);
  
  score += EvalTrappedKnight(p);                 
  score += (EvalTrappedBishop(p,WHITE) - EvalTrappedBishop(p,BLACK) );                 
  score += (EvalTrappedRook(p,WHITE)   - EvalTrappedRook(p,BLACK) );  

  mgScore += (p->pstMg[WHITE] - p->pstMg[BLACK]);
  egScore += (p->pstEg[WHITE] - p->pstEg[BLACK]);
  
#ifdef LAZY_EVAL
  int temp_score = score + Interpolate();

  // lazy evaluation - avoids costly calculations
  // if score seems already very high/very low
  if (temp_score > alpha - Data.lazyMargin 
  &&  temp_score < beta +  Data.lazyMargin) {
#endif
	  fullEval = 1;
	  InitDynamicScore(p);

      ScoreN(p, WHITE);
      ScoreN(p, BLACK);
	  ScoreB(p, WHITE);
      ScoreB(p, BLACK);
      ScoreR(p, WHITE);
	  ScoreR(p, BLACK);
      ScoreQ(p, WHITE);
      ScoreQ(p, BLACK);
      ScoreKingShield(p, WHITE);
      ScoreKingShield(p, BLACK);
	  ScoreKingAttacks(p, WHITE);
	  ScoreKingAttacks(p, BLACK);
	  ScoreHanging(p, WHITE);
	  ScoreHanging(p, BLACK);

	  // ADDITIONAL PAWN EVAL
	  ScoreP(p, WHITE);
	  ScoreP(p, BLACK);
      
	  // ASYMMETRIC MOBILITY SCALING
	  ScaleValue(&mgMobility[WHITE], Data.mobSidePercentage[WHITE]);
	  ScaleValue(&mgMobility[BLACK], Data.mobSidePercentage[BLACK]);
  	  ScaleValue(&egMobility[WHITE], Data.mobSidePercentage[WHITE]);
	  ScaleValue(&egMobility[BLACK], Data.mobSidePercentage[BLACK]);

	  // MERGING SCORE
	  mgScore += ( mgMobility[WHITE] - mgMobility[BLACK] );
	  egScore += ( egMobility[WHITE] - egMobility[BLACK] );
	  mgScore += ( mgMisc[WHITE]     - mgMisc[BLACK]     );
	  egScore += ( egMisc[WHITE]     - egMisc[BLACK]     );
      score   += Interpolate();    // merge middlegame and endgame scores
	  score   += ( attScore[WHITE]  - attScore[BLACK] );
#ifdef LAZY_EVAL
  }
  else score = temp_score; 
#endif

  score = PullToDraw(p, score);    // decrease score in drawish endgames
  score = FinalizeScore(p, score); // bounds, granulatity and weakening

#ifdef HASH_EVAL
  if (fullEval) {
  EvalTT[addr].key = p->hashKey;
  EvalTT[addr].score = score;
  }
#endif

  // return score relative to the side to move
  return p->side == WHITE ? score : -score;
}

// fast evaluation function (material, pst, pawn structure)
int sEvaluator::ReturnFast(sPosition *p)
{
  int score = GetMaterialScore(p) + CheckmateHelper(p);
  p->side == WHITE ? score+=5 : score-=5;

  InitStaticScore();
  SetScaleFactor(p);
  EvalPawns(p);

  mgScore += (p->pstMg[WHITE] - p->pstMg[BLACK]);
  egScore += (p->pstEg[WHITE] - p->pstEg[BLACK]);
  
  score += Interpolate();
  score = PullToDraw(p, score);    // decrease score in drawish endgames
  score = FinalizeScore(p, score); // bounds, granulatity and weakening

  // return score relative to the side to move
  return p->side == WHITE ? score : -score;
}

int sEvaluator::Normalize(int val, int limit) 
{
	if (val > limit)       return limit;
	else if (val < -limit) return -limit;
    return val;
}

int sEvaluator::FinalizeScore(sPosition * p, int score)
{
  // add random value in weakening mode
  if (Data.elo < MAX_ELO && Data.useWeakening) {
	  int randomFactor = ( MAX_ELO - Data.elo ) / 10;
	  int randomMod = (randomFactor / 2) - (p->hashKey % randomFactor);
	  score += randomMod;
  }

  score = Normalize(score, MAX_EVAL);         // enforce bounds
  return ( score / GRAIN_SIZE) * GRAIN_SIZE;  // enforce grain
}

void sEvaluator::ScaleValue(int * value, int factor) 
{
     *value *= factor;
	 *value /= 1000;
}

void sEvaluator::DebugPst(sPosition *p) 
{
	 int sq, mg = 0, eg = 0;
	 const int clMult[2] = {1, -1};
	 U64 bbPieces;

	 for (int cl = 0; cl <= 1; cl++)
	 for (int pc = 0; pc <= 5; pc++)  
	 {
	     bbPieces = bbPc(p, cl, pc);
	     while (bbPieces) {
           sq = PopFirstBit(&bbPieces);
	       mg += Data.pstMg[cl][pc][sq] * clMult[cl];
		   eg += Data.pstEg[cl][pc][sq] * clMult[cl];
	     }
	 }
	 printf("Recalculated pst : mg %d eg %d\n", mg, eg);
}
