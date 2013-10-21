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

  This file contains functions evaluating the pieces; please note
  that trapped pieces evaluation is coded in eval_trapped.c
*/

#include "../rodent.h"
#include "../bitboard/bitboard.h"
#include "../data.h"
#include "../bitboard/gencache.h"
#include "eval.h"
#include <stdio.h>

  // eval data not exposed to the user
  const int outpostBase       [7] = { 0,   4,   4,   0,   0,   0,  0};
  const int rookOpenAttack    [2] = { 100, 0 };
  const int rookSemiOpenAttack[2] = {  50, 0 };
  const int queenContactCheck [2] = { 300, 6 };
  const int rookContactCheck  [2] = { 200, 4 };
  const int rookSeventhMg   = 20;
  const int rookSeventhEg   = 20; // BEST 20, 40 is worse
  const int rookOpenMg      = 10;
  const int rookOpenEg      = 10;
  const int rookSemiOpenMg  = 5;
  const int rookSemiOpenEg  = 5;

  // data for attack evaluation:        for Rodent curve                    for Stockfish-like curve
  //                                    P    N    B    R    Q    K          P   N   B   R   Q   K
  const int attPerPc     [2]  [7] = { { 0,  10,  10,  20,  40,   0,  0} , { 0,  2,  2,  3,  5,  0,  0} };
  const int canCheckWith [2]  [7] = { { 0,   0,  10,  40,  100,  0,  0} , { 0,  1,  1,  2,  3,  0,  0} }; 
  const int woodPerPc         [7] =   { 0,   1,   1,   2,   4,   0,  0};

void sEvaluator::ScoreN(sPosition *p, int side) 
{
  int sq;
  U64 bbControl, bbAttZone;
  U64 bbPieces = bbPc(p, side, N);

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                     // set piece location and clear it from bbPieces
	bbControl = bbKnightAttacks[sq];                 // set control bitboard
	bbAllAttacks[side] |= bbControl;                 // update attack data
	bbControl &= ~p->bbCl[side];                     // exclude squares occupied by own pieces
	ScoreMinorPawnRelation(p, side, sq);             // knight attacked / defended by pawn
	ScorePieceInHole(p, side, N, sq);                    

	// king attacks (if our queen is present)
	bbAttZone = bbControl & bbKingZone[side][p->kingSquare[Opp(side)]];
	if (bbAttZone && p->pcCount[side][Q] ) {
		AddPieceAttack(side, N, PopCntSparse(bbAttZone) ); 
		bbMinorCoorAttacks[side] ^= bbAttZone;
	}

	bbControl &= ~bbPawnControl[Opp(side)];          // exclude squares controlled by enemy pawns
	AddMobility(N, side, PopCnt15(bbControl) );      // evaluate mobility

    // check threats (excluding checks from squares controlled by enemy pawns)
	if (bbControl & bbKnightChecks[Opp(side)] )
		attCount[side] += canCheckWith[Data.safetyStyle][N]; 
  }
}

void sEvaluator::ScoreB(sPosition *p, int side) 
{
  int sq;
  U64 bbControl, bbAttZone;
  U64 bbPieces    = bbPc(p, side, B);
  U64 bbOccupied  = OccBb(p) ^ bbPc(p, side, Q);     // accept mobility through own queen

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                     // set piece location and clear it from bbPieces
	bbControl = GenCache.GetBishMob(bbOccupied, sq); // set control/mobility bitboard
	bbAllAttacks[side] |= bbControl;                 // update attack data
	ScoreMinorPawnRelation(p, side, sq);             // bishop attacked / defended by pawn
	ScorePieceInHole(p, side, B, sq);               

    // king attack (if our queen is present)
	if (bbBCanAttack[sq] [KingSq(p, side ^ 1) ] 
	&& (bbControl & bbKingZone[side][p->kingSquare[Opp(side)]] ) ) {
       bbAttZone = bbControl & bbKingZone[side][p->kingSquare[Opp(side)]];
	   if (bbAttZone && p->pcCount[side][Q] ) {
		   AddPieceAttack( side, B, PopCntSparse(bbAttZone) ); 
		   bbMinorCoorAttacks[side] ^= bbAttZone;
	   }
   }

	bbControl &= ~bbPawnControl[Opp(side)];          // exclude squares controlled by enemy pawns
	AddMobility(B, side, PopCnt15(bbControl) );      // evaluate mobility

    // check threats (with false positive due to queen transparency)
	if (bbControl & bbDiagChecks[Opp(side)] )
		attCount[side] += canCheckWith[Data.safetyStyle][B];

	// penalize bishop blocked by own pawns
	if ( bbBadBishopMasks[side][sq] & bbPc(p, side, P) )
       AddMisc(side, Data.badBishopPenalty[side][sq], Data.badBishopPenalty[side][sq]);
  }
}

void sEvaluator::ScoreR(sPosition *p, int side) 
{
  int sq, contactSq;
  U64 bbControl, bbAttZone, bbContact;
  U64 bbPieces   = bbPc(p, side, R);
  U64 bbOccupied = OccBb(p) ^ bbPc(p, side, Q) ^ bbPc(p, side, R); // R and Q are considered transparent

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                     // set piece location and clear it from bbPieces
	bbControl = GenCache.GetRookMob(bbOccupied, sq); // set control/mobility bitboard
	bbAllAttacks[side] |= bbControl;                 // update attack data
	ScorePieceInHole(p, side, R, sq);                

	U64 bbFrontSpan = GetFrontSpan(SqBb(sq), side );
	if (bbFrontSpan & bbPc(p, Opp(side), Q) ) AddMisc(side, 5, 5); // rook and enemy queen in the same file

	// evaluate rook on an open file
	if ( !(bbFrontSpan & bbPc(p,side, P) ) ) {                     // no own pawns in front of the rook
	   if ( !(bbFrontSpan & bbPc(p, Opp(side), P) ) ) {            // no enemy pawns - open file
		  AddMisc(side, rookOpenMg, rookOpenEg);
		  if (bbFrontSpan & bbKingZone[side][p->kingSquare[Opp(side)]] ) attCount[side] += rookOpenAttack[Data.safetyStyle];
	   } else {                                                    // enemy pawns present - semi-open file
		  AddMisc(side, rookSemiOpenMg, rookSemiOpenEg);
		  if (bbFrontSpan & bbKingZone[side][p->kingSquare[Opp(side)]] ) attCount[side] += rookSemiOpenAttack[Data.safetyStyle];
	   }
	}

	// evaluate rook on 7th rank if it attacks pawns or cuts off enemy king
	if (SqBb(sq) & bbRelRank[side][RANK_7] ) {
       if ( bbPc(p, Opp(side), P) & bbRelRank[side][RANK_7]
	   || bbPc(p, Opp(side), K) & bbRelRank[side][RANK_8]
	   )  AddMisc(side, rookSeventhMg, rookSeventhEg);
	}

    // check threats (including false positives due to queen/rook transparency)
	if (bbControl & bbStraightChecks[Opp(side)] ) {
		attCount[side] += canCheckWith[Data.safetyStyle][R];
        // safe contact checks
	    bbContact = bbControl & bbKingAttacks[ p->kingSquare[Opp(side)] ] & bbStraightChecks[Opp(side)];
	    while (bbContact) {
           contactSq = PopFirstBit(&bbContact);

	       if ( Swap(p, sq, contactSq) >= 0 ) {
			  attCount[side] += rookContactCheck[Data.safetyStyle]; 
		      break;
	       }
	    }
	}

	// king attack (if our queen is present)
	if ( bbRCanAttack[sq] [KingSq(p, side ^ 1) ]  
	&& ( bbControl & bbKingZone[side][p->kingSquare[Opp(side)]] ) ) {
       bbAttZone = bbControl & bbKingZone[side][p->kingSquare[Opp(side)]];
	   if (bbAttZone && p->pcCount[side][Q]) {
		   AddPieceAttack(side, R, PopCntSparse(bbAttZone) );
		   attCount[side] += PopCntSparse( bbAttZone & bbMinorCoorAttacks[side] ); // rook-minor coordination
	   }
	}
	
	AddMobility(R, side, PopCnt15(bbControl) );
  }
}

void sEvaluator::ScoreQ(sPosition *p, int side) 
{
  int sq, contactSq;
  U64 bbControl, bbAttacks, bbContact, bbAttZone;
  U64 bbPieces       = bbPc(p, side, Q); 
  U64 bbOccupied     = OccBb(p); // real occupancy, since we'll look for contact checks
  U64 bbTransparent  = bbPc(p, side, R) | bbPc(p, side, B);
  U64 bbCanCheckFrom = bbStraightChecks[Opp(side)] | bbDiagChecks[Opp(side)];

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                      // set piece location and clear it from bbPieces 
	bbControl = GenCache.GetQueenMob(bbOccupied, sq); // set control/mobility bitboard
	bbAllAttacks[side] |= bbControl;                  // update attack data

	// evaluate queen on 7th rank if it attacks pawns or cuts off enemy king
	if (SqBb(sq) & bbRelRank[side][RANK_7] ) {
       if ( bbPc(p, Opp(side), P) & bbRelRank[side][RANK_7]
	   || bbPc(p, Opp(side), K) & bbRelRank[side][RANK_8]
	   )  AddMisc(side, 10, 5); // correct would be 5/10, but it is for the next test
	}

	if (bbControl & bbCanCheckFrom ) {

		// queen check threats (unlike with other pieces, we *count the number* of possible checks here)
		attCount[side] += PopCntSparse( bbControl & bbCanCheckFrom ) * canCheckWith[Data.safetyStyle][Q];

        // safe contact checks
	    bbContact = bbControl & bbKingAttacks[ p->kingSquare[Opp(side)] ];
	    while (bbContact) {
           contactSq = PopFirstBit(&bbContact);

	       if ( Swap(p, sq, contactSq) >= 0 ) {
			  attCount[side] += queenContactCheck[Data.safetyStyle]; 
		      break;
	       }
	    }
	}

    if (bbQCanAttack[sq] [KingSq(p, side ^ 1) ]
	&& (attCount[side] > 0 || p->pcCount[side][Q] > 1) ) // otherwise queen attack won't change score
	{
	   // consider also attacks through other pieces
	   if (bbControl && bbTransparent)
	      bbAttacks = QAttacks(bbOccupied ^ bbTransparent ^ SqBb(sq), sq);
	   else 
	      bbAttacks = bbControl;

	   // count attacks
	   bbAttZone = bbAttacks & bbKingZone[side][p->kingSquare[Opp(side)]];
	   if (bbAttZone) {
		   AddPieceAttack(side, Q, PopCntSparse(bbAttZone) );	   
		   attCount[side] += PopCntSparse( bbAttZone & bbMinorCoorAttacks[side] ); // coordinated Queen - minor attacks
	   }
	}
	
	AddMobility(Q, side, PopCnt(bbControl) );
  }
}

void sEvaluator::ScoreP(sPosition *p, int side) 
{
  int sq, passUnitMg, passUnitEg;
  U64 bbPieces = bbPc(p, side, P);
  U64 bbOccupied = OccBb(p);
  U64 bbStop, bbObstacles;

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);
	bbStop = ShiftFwd(SqBb(sq), side);

	if (bbStop &~bbOccupied) {           // this pawn is mobile
	   if (Data.pstMg[side][P][sq] > 0)  // and placed on a good square
		   AddMisc(side, Data.pstMg[side][P][sq] / 5, Data.pstEg[side][P][sq] / 5);
	   else AddMisc(side, 2, 1);
	}
	   
	bbObstacles = bbPassedMask[side][sq] & bbPc(p, Opp(side), P);

	// additional evaluation of a passed pawn 
	if (!bbObstacles) {
		passUnitMg = Data.pawnProperty[PASSED][MG][side][sq] / 5;
		passUnitEg = Data.pawnProperty[PASSED][EG][side][sq] / 5;

		// enemy king distance to a passer (failed to find good value for a friendly king)
		AddMisc(side, 0, (-Data.distance[sq] [p->kingSquare[Opp(side)]] * passUnitEg) / 6);

		// blocked and unblocked passers
		if (bbStop &~bbOccupied) AddMisc(side,  passUnitMg,  passUnitEg);
		else                     AddMisc(side, -passUnitMg, -passUnitEg);

		// control of stop square
		if (bbStop &~ bbAllAttacks[Opp(side)] ) {
           AddMisc(side,  passUnitMg,  passUnitEg);
           if (bbStop & bbAllAttacks[side] ) AddMisc(side,  passUnitMg,  passUnitEg);
		}
	}
  }
}

void sEvaluator::AddPieceAttack(int side, int pc, int cnt)
{
    attNumber[side] += 1;
	attCount [side] += attPerPc[Data.safetyStyle][pc] * cnt;
	attWood  [side] += woodPerPc[pc];
}

void sEvaluator::AddMobility( int pc, int side, int cnt)
{
	mgMobility[side] += Data.mobBonusMg [pc] [cnt];
	egMobility[side] += Data.mobBonusEg [pc] [cnt];
}

void sEvaluator::AddMisc(int side, int mg, int eg)
{
	mgMisc[side] += mg;
	egMisc[side] += eg;
}

void sEvaluator::ScorePieceInHole(sPosition *p, int side, int piece, int sq) 
{
	// constant bonus if piece occupies hole of enemy pawn structure
	if ( SqBb(sq) & ~bbPawnCanControl[Opp(side)] ) {
		AddMisc(side, outpostBase[piece], outpostBase[piece] );

	   // additional pst bonus if defended by a pawn
	   if ( SqBb(sq) & bbPawnControl[side] ) 
		   AddMisc(side, Data.outpost[side][piece][sq], Data.outpost[side][piece][sq] );
	}
}

void sEvaluator::ScoreMinorPawnRelation(sPosition *p, int side, int sq)
{
	if ( SqBb(sq) & bbPawnControl[Opp(side)] )  AddMisc(side, -10, -10); // attacked by pawn
	else if ( SqBb(sq) & bbPawnControl[side] )  AddMisc(side, 2, 2);     // defended by pawn
}