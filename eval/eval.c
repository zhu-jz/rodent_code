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

#include <stdio.h>
#include "../bitboard/bitboard.h"
#include "../data.h"
#include "../rodent.h"
#include "eval.h"

const int safety[ 256 ] = {
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

const int n_of_att[ 24 ] =   { 0, 6, 12, 18, 24, 32, 48, 52, 56, 60, 64, 66, 68, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70 };

const int mgTwoOnSeventh  = 5;
const int egTwoOnSeventh  = 10;

void sEvaluator::InitStaticScore(void) 
{
     mgScore            = 0;   egScore            = 0;  // clear midgame/endgame score component
     pawnScoreMg[WHITE] = 0;   pawnScoreMg[BLACK] = 0;  // clear midgame pawn scores
     pawnScoreEg[WHITE] = 0;   pawnScoreEg[BLACK] = 0;  // clear endgame pawn scores	 
     passerScoreMg[WHITE] = 0;   passerScoreMg[BLACK] = 0;  // clear midgame pawn scores
     passerScoreEg[WHITE] = 0;   passerScoreEg[BLACK] = 0;  // clear endgame pawn scores
}

void sEvaluator::InitDynamicScore(sPosition *p) 
{
     attScore[WHITE]         = 0;   attScore[BLACK]    = 0;  // clear attack scores
	 attNumber[WHITE]        = 0;   attNumber[BLACK]   = 0;  // clear no. of attackers
	 attCount[WHITE]         = 0;   attCount[BLACK]    = 0;
	 checkCount[WHITE]       = 0;   checkCount[BLACK]  = 0;
	 attWood[WHITE]          = 0;   attWood[BLACK]     = 0;  // clear "wood weight" of attack
	 kingTropism[WHITE]      = 0;   kingTropism[BLACK] = 0;
	 mgMisc[WHITE]           = 0;   mgMisc[BLACK]      = 0;  // clear miscelanneous midgame scores
	 egMisc[WHITE]           = 0;   egMisc[BLACK]      = 0;  // clear miscelanneous endgame scores
	 mgMobility[WHITE]       = 0;   mgMobility[BLACK]  = 0;  // clear midgame mobility
	 egMobility[WHITE]       = 0;   egMobility[BLACK]  = 0;  // clear endgame mobility	 
	 bbPawnTakes[WHITE]    = GetWPControl( bbPc(p, WHITE, P) );
	 bbPawnTakes[BLACK]    = GetBPControl( bbPc(p, BLACK, P) );
	 bbPawnCanTake[WHITE] = FillNorth( bbPawnTakes[WHITE] );
	 bbPawnCanTake[BLACK] = FillSouth( bbPawnTakes[BLACK] );
	 bbAllAttacks[WHITE]     = bbPawnTakes[WHITE];
	 bbAllAttacks[BLACK]     = bbPawnTakes[BLACK];
	 bbMinorCoorAttacks[WHITE]  = 0ULL;
	 bbMinorCoorAttacks[BLACK]  = 0ULL;
 	 bbRookCoorAttacks[WHITE]   = 0ULL;
	 bbRookCoorAttacks[BLACK]   = 0ULL;

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
	U64 bbHanging    = p->bbCl[Opp(side)] & ~bbPawnTakes[Opp(side)]; 
	U64 bbThreatened = p->bbCl[Opp(side)] & bbPawnTakes[side];
	bbHanging |= bbThreatened;            // piece attacked by our pawn isn't well defended
	bbHanging &= bbAllAttacks[side];      // obviously, hanging piece has to be attacked
	bbHanging &= ~bbPc(p, Opp(side), P);  // currently we don't evaluate threats against pawns

	U64 bbSpace = UnoccBb(p) & bbAllAttacks[side];
	bbSpace &= ~bbRelRank[side][RANK_1];    // controlling home ground is not space advantage
	bbSpace &= ~bbRelRank[side][RANK_2];
	bbSpace &= ~bbRelRank[side][RANK_3];
	bbSpace &= ~bbPawnTakes[Opp(side)]; // squares attacked by enemy pawns aren't effectively controlled
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

	if (Data.safetyStyle == KS_QUADRATIC) {
		int attUnit = attCount[side];    // attacks on squares near enemy king
		attUnit += checkCount[side];     // check and contact check threats
		attUnit += (attWood[side] / 2);  // material involved in the attack
		if (attUnit > 99) attUnit = 99;  // bounds checking
		if (attNumber[side] < 2) attScore[side] = 0;
		//TODO: find some way to include attacks with less than two pieces
		attScore[side] = Data.kingDanger[attUnit] * 10;
	}
	if (Data.safetyStyle == KS_HANDMADE) {
		int attUnit = attCount[side] + checkCount[side];
		attScore[side] = safety[ attUnit + n_of_att[attNumber[side]] ] * 10;
	}
	ScaleValue(&attScore[side], Data.attSidePercentage[side]);
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

void sEvaluator::ScorePatterns(sPosition *p, int side)
{
	// more than one rook on the 7th rank
	if (MoreThanOne( ( bbPc(p,side,R) ) & bbRelRank[side][RANK_7])) 
		AddMisc(side, mgTwoOnSeventh,egTwoOnSeventh);
}

int sEvaluator::ReturnFull(sPosition *p)
{
#ifdef HASH_EVAL
	int addr = p->hashKey % EVAL_HASH_SIZE;
	if (EvalTT[addr].key == p->hashKey) {
		int hashScore = EvalTT[addr].score;
		return p->side == WHITE ? hashScore : -hashScore;
	}
#endif

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

	  // PATTERNS
	  ScorePatterns(p, WHITE);
	  ScorePatterns(p, BLACK);
      
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
	  mgScore += ( (kingTropism[WHITE] - kingTropism[BLACK]) * Data.tropismWeight ) / 100;
	  score   += Interpolate();    // merge middlegame and endgame scores
	  score   += ( attScore[WHITE]  - attScore[BLACK] );


  score = PullToDraw(p, score);    // decrease score in drawish endgames
  score = FinalizeScore(p, score); // bounds, granulatity and weakening

#ifdef HASH_EVAL
  EvalTT[addr].key = p->hashKey;
  EvalTT[addr].score = score;
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

void sEvaluator::PrintEval(sPosition *p) 
{
    Data.InitAsymmetric(p->side);
	int result = ReturnFull(p);
	if (p->side == BLACK) result *= -1;
	printf("Total      : %d\n", result);
	printf("Phase      : %2d of 24 \n", mgFact);
	printf("Material   : %d\n", GetMaterialScore(p) );
	printf("Pst        : "); PrintEvalFactor( p->pstMg[WHITE], p->pstMg[BLACK], p->pstEg[WHITE], p->pstEg[BLACK]);
	printf("Mobility   : "); PrintEvalFactor( mgMobility[WHITE], mgMobility[BLACK], egMobility[WHITE], egMobility[BLACK]);
	printf("King attack: "); PrintEvalFactor( attScore[WHITE], attScore[BLACK], attScore[WHITE], attScore[BLACK]);
	printf("Others     : "); PrintEvalFactor( mgMisc[WHITE], mgMisc[BLACK], egMisc[WHITE], egMisc[BLACK]);
}
 
void sEvaluator::PrintEvalFactor(int mgW, int mgB, int egW, int egB)
{
	int mg = mgW - mgB;
	int eg = egW - egB;
	printf("mg: %4d, eg:%3d, scaled: %3d\n", mg, eg,  (mgFact * mg ) / 24 + (egFact * eg ) / 24   );
}
