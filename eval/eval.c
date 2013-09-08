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
#include "../bitboard/bitboard.h"
#include "../data.h"
#include "../rodent.h"
#include "eval.h"

const U64 rank1[2] = {bbRANK_1, bbRANK_8};
const U64 rank2[2] = {bbRANK_2, bbRANK_7};
const U64 rank3[2] = {bbRANK_3, bbRANK_6};
const U64 rank4[2] = {bbRANK_4, bbRANK_5};
const U64 rank5[2] = {bbRANK_5, bbRANK_4};
const U64 rank6[2] = {bbRANK_6, bbRANK_3};
const U64 rank7[2] = {bbRANK_7, bbRANK_2};
const U64 rank8[2] = {bbRANK_8, bbRANK_1};

void sEvaluator::InitStatic(void) 
{
     mgScore            = 0;   egScore            = 0;  // clear midgame/endgame score component
     pawnScoreMg[WHITE] = 0;   pawnScoreMg[BLACK] = 0;  // clear midgame pawn scores
     pawnScoreEg[WHITE] = 0;   pawnScoreEg[BLACK] = 0;  // clear endgame pawn scores	 
}

void sEvaluator::InitDynamic(sPosition *p) 
{
     attScore[WHITE]         = 0;   attScore[BLACK]    = 0;  // clear attack scores
	 attNumber[WHITE]        = 0;   attNumber[BLACK]   = 0;  // clear attack scores
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
	 
	 // set squares from which king can be checked 
	 U64 bbOccupied = OccBb(p);
	 kingStraightChecks[WHITE] = RAttacks(bbOccupied, KingSq(p, WHITE) );  
	 kingStraightChecks[BLACK] = RAttacks(bbOccupied, KingSq(p, BLACK) );  
	 kingDiagChecks[WHITE]     = BAttacks(bbOccupied, KingSq(p, WHITE) );  
	 kingDiagChecks[BLACK]     = BAttacks(bbOccupied, KingSq(p, BLACK) );  
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
	bbSpace &= ~rank1[side];              // controlling home ground is not space advantage
	bbSpace &= ~rank2[side];
	bbSpace &= ~rank3[side];
	bbSpace &= ~bbPawnControl[Opp(side)]; // squares attacked by enemy pawns aren't effectively controlled
	AddMiscTwo(side, PopCnt(bbSpace), 0);
	int pc, sq, val;

    while (bbHanging) {
       sq  = FirstOne(bbHanging);
	   pc  = TpOnSq(p, sq);
	   val = Data.matValue[pc] / 32;
	   AddMiscTwo(side, 5+val, 10+val);
	   bbHanging &= bbHanging - 1;
	}
}

 int sEvaluator::ScorePatterns(sPosition *p, int side)
 {
	 int score = 0;
      
	 if  ( ( bbPc(p, side, N) & RelSqBb(C3,side) )
	 &&    ( bbPc(p, side, P) & RelSqBb(C2,side) ) 
	 &&    ( bbPc(p, side, P) & RelSqBb(D4,side) )
	 &&   !( bbPc(p, side, P) & RelSqBb(E4,side) )
	 ) score -= 15; // avoid blocking "c" pawn with a knight

	 return score;
 }

void sEvaluator::ScoreK(sPosition *p, int side)
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
  
  bbAllAttacks[side] |= bbKingAttacks[KingSq(p, side) ];

  // we use generic score for castled king to avoid changing shield score by, say, Kh1-g1
  if (SqBb(sq) & bbKSCastle[side]) sq = kCastle[side];
  if (SqBb(sq) & bbQSCastle[side]) sq = qCastle[side];

  // TODO: shrink this code writing an EvalKingsFile function
  bbKingFile = FillNorth(SqBb(sq) ) | FillSouth(SqBb(sq));
  if (bbKingFile & bbCentralFile)
     result += (EvalFileShelter( bbKingFile & bbPc(p, side, P), side ) / 2);
  else
     result += EvalFileShelter( bbKingFile & bbPc(p, side, P), side );
  result += EvalFileStorm  ( bbKingFile & bbPc(p, Opp(side), P), side );
   
  bbNextFile = ShiftEast(bbKingFile);
  if (bbNextFile) {
     if (bbNextFile & bbCentralFile)
        result += (EvalFileShelter( bbNextFile & bbPc(p, side, P), side ) / 2);
	 else
		result += EvalFileShelter( bbNextFile & bbPc(p, side, P), side );
	 result += EvalFileStorm  ( bbNextFile & bbPc(p, Opp(side), P), side );
  }
  
  bbNextFile = ShiftWest(bbKingFile);
  if (bbNextFile) {
     if (bbNextFile & bbCentralFile)
        result += (EvalFileShelter( bbNextFile & bbPc(p, side, P), side ) / 2);
	 else
		result += EvalFileShelter( bbNextFile & bbPc(p, side, P), side );
	 result += EvalFileStorm  ( bbNextFile & bbPc(p, Opp(side), P), side );
  }

  mgScore += result * sideMult[side];                 // add shield score to midgame score

  if (attCount[side] > 255) attCount[side] = 255;     // normalize attack data for table size
  if (attNumber[side] < 2)  attCount[side] = 0;       // avoid evaluating attack by single piece
  attScore[side] = Data.attackBonus[attCount[side]];  // read attack score from the table
}

int sEvaluator::EvalFileShelter(U64 bbOwnPawns, int side)
{
	// values taken from Fruit
	if ( !bbOwnPawns ) return -36;
	if ( bbOwnPawns & rank2[side] ) return    0;
	if ( bbOwnPawns & rank3[side] ) return  -11;
	if ( bbOwnPawns & rank4[side] ) return  -20;
	if ( bbOwnPawns & rank5[side] ) return  -27;
	if ( bbOwnPawns & rank6[side] ) return  -32;
	if ( bbOwnPawns & rank7[side] ) return  -35;
	return 0;
}

int sEvaluator::EvalFileStorm(U64 bbOppPawns, int side)
{
	if (!bbOppPawns) return -15;
	if (bbOppPawns & rank4[side]) return -3;
	if (bbOppPawns & rank5[side]) return -5;
	if (bbOppPawns & rank6[side]) return -7;
    return 0;
}

int sEvaluator::ReturnFull(sPosition *p, int alpha, int beta)
{
  int score = GetMaterialScore(p) + CheckmateHelper(p);
  p->side == WHITE ? score+=5 : score-=5;

  InitStatic();
  SetScaleFactor(p);
  EvalPawns(p);
  
  score += EvalTrappedKnight(p);                 
  score += (EvalTrappedBishop(p,WHITE) - EvalTrappedBishop(p,BLACK) );                 
  score += (EvalTrappedRook(p,WHITE)   - EvalTrappedRook(p,BLACK) );  

  mgScore += (p->pstMg[WHITE] - p->pstMg[BLACK]);
  egScore += (p->pstEg[WHITE] - p->pstEg[BLACK]);
  
  int temp_score = score + Interpolate();

  // lazy evaluation - avoids costly calculations
  // if score seems already very high/very low
  if (temp_score > alpha - Data.lazyMargin 
  &&  temp_score < beta +  Data.lazyMargin) {
	  InitDynamic(p);

      ScoreN(p, WHITE);
      ScoreN(p, BLACK);
	  ScoreB(p, WHITE);
      ScoreB(p, BLACK);
      ScoreR(p, WHITE);
	  ScoreR(p, BLACK);
      ScoreQ(p, WHITE);
      ScoreQ(p, BLACK);
	  ScoreK(p, WHITE);
	  ScoreK(p, BLACK);
	  ScoreHanging(p, WHITE);
	  ScoreHanging(p, BLACK);

	  // correction of passed pawns eval based on their interactions with pieces
	  ScoreP(p, WHITE);
	  ScoreP(p, BLACK);
      
	  // ASYMMETRIC MOBILITY AND ATTACK SCALING
	  ScaleValue(&mgMobility[WHITE], Data.mobSidePercentage[WHITE]);
	  ScaleValue(&mgMobility[BLACK], Data.mobSidePercentage[BLACK]);
  	  ScaleValue(&egMobility[WHITE], Data.mobSidePercentage[WHITE]);
	  ScaleValue(&egMobility[BLACK], Data.mobSidePercentage[BLACK]);
	  ScaleValue(&attScore[WHITE]  , Data.attSidePercentage[WHITE]);
	  ScaleValue(&attScore[BLACK]  , Data.attSidePercentage[BLACK]);

	  // MERGING SCORE
	  mgScore += ( mgMobility[WHITE] - mgMobility[BLACK] );
	  egScore += ( egMobility[WHITE] - egMobility[BLACK] );
	  mgScore += ( mgMisc[WHITE]     - mgMisc[BLACK]     );
	  egScore += ( egMisc[WHITE]     - egMisc[BLACK]     );
      score   += Interpolate();    // merge middlegame and endgame scores
	  score   += ( attScore[WHITE]  - attScore[BLACK] );

      score += ScorePatterns(p, WHITE);
	  score -= ScorePatterns(p, BLACK);
  }
  else score = temp_score; 

  score = PullToDraw(p, score); // decrease score in drawish endgames

  // add random value in weakening mode
  if (Data.elo < MAX_ELO && Data.useWeakening) {
	  int randomFactor = ( MAX_ELO - Data.elo ) / 10;
	  int randomMod = (randomFactor / 2) - (p->hashKey % randomFactor);
	  score += randomMod;
  }

  score = Normalize(score, MAX_EVAL);

  // grain
  score /= GRAIN_SIZE;
  score *= GRAIN_SIZE;

  // return score relative to the side to move
  return p->side == WHITE ? score : -score;
}

// fast evaluation function (material, pst, pawn structure)
int sEvaluator::ReturnFast(sPosition *p)
{
  int score = GetMaterialScore(p) + CheckmateHelper(p);
  p->side == WHITE ? score+=5 : score-=5;

  InitStatic();
  SetScaleFactor(p);
  EvalPawns(p);

  mgScore += (p->pstMg[WHITE] - p->pstMg[BLACK]);
  egScore += (p->pstEg[WHITE] - p->pstEg[BLACK]);
  
  score += Interpolate();
  score = PullToDraw(p, score); // decrease score in drawish endgames

  // add random value in weakening mode
  if (Data.elo < MAX_ELO && Data.useWeakening) {
	  int randomFactor = ( MAX_ELO - Data.elo ) / 15;
	  int randomMod = (randomFactor / 2) - (p->hashKey % randomFactor);
	  score += randomMod;
  }

  score = Normalize(score, MAX_EVAL);

  // grain
  score /= GRAIN_SIZE;
  score *= GRAIN_SIZE;

  // return score relative to the side to move
  return p->side == WHITE ? score : -score;
}

int sEvaluator::Normalize(int val, int limit) 
{
	if (val > limit)       return limit;
	else if (val < -limit) return -limit;
    return val;
}

void sEvaluator::ScaleValue(int * value, int factor) 
{
     *value *= factor;
	 *value /= 100;
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