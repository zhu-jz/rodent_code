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

  This file contains functions evaluating the pieces; please note
  that trapped pieces evaluation is coded in eval_trapped.c
*/

#include "../rodent.h"
#include "../bitboard/bitboard.h"
#include "../data.h"
#include "../bitboard/gencache.h"
#include "eval.h"
#include <stdio.h>
	  
  const int pawnDefendsMg     [7] = { 0,   2,   2,   2,   0,   0,  0 };
  const int pawnDefendsEg     [7] = { 0,   2,   2,   4,   0,   0,  0 };
  
  // (NOTE: we don't evaluate N attacks N, B attacks B and R attacks R)
  const int pAttacks          [7] = { 0,  10,  10,  15,   0,   0,  0 };
  const int nAttacks          [7] = { 1,   5,   5,  10,  10,   0,  0 };  // N/B
  const int rAttacks          [7] = { 1,   3,   3,   5,   5,   0,  0 };
  const int qAttacks          [7] = { 1,   3,   3,   5,   5,   0,  0 };

  const int outpostBase       [7] = { 0,   4,   4,   0,   0,   0,  0 };
  const int rookOpenAttack    [2] = { 0,  2 };
  const int rookSemiOpenAttack[2] = { 0,  1 };
  const int queenContactCheck [2] = { 6, 10 };
  const int rookContactCheck  [2] = { 4,  5 };
  const int rookSeventhMg   = 20;
  const int rookSeventhEg   = 20; // BEST 20, 30 is worse
  const int rookOpenMg      = 10;
  const int rookOpenEg      = 10;
  const int rookSemiOpenMg  = 5;
  const int rookSemiOpenEg  = 5;

  // data for attack evaluation:        for Stockfish-like curve      for old Glass curve
  //                                    P   N   B   R   Q   K         P   N   B   R   Q   K
  const int attPerPc     [2]  [7] = { { 0,  2,  2,  3,  5,  0,  0}, { 0,  1,  1,  2,  3,  0,  0} };
  const int canCheckWith [2]  [7] = { { 0,  1,  1,  3,  4,  0,  0}, { 0,  1,  1,  2,  3,  0,  0} };
  const int woodPerPc         [7] =   { 0,  1,  1,  2,  4,  0,  0};

void sEvaluator::ScoreN(sPosition *p, int side) 
{
  int sq;
  const int oppo = Opp(side);
  U64 bbMob, bbAtt;
  U64 bbPieces = bbPc(p, side, N);

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);               // set piece location and clear it from bbPieces
	bbMob = bbKnightAttacks[sq];               // set control bitboard
	bbAllAttacks[side] |= bbMob;               // update attack data
	bbMob &= ~p->bbCl[side];                   // exclude squares occupied by own pieces
	ScoreRelationToPawns(p, side, N, sq);      // outpost, attacked or defended by a pawn

	// attacks on enemy pieces
	if (bbMob & bbPc(p, oppo, P) ) AddMisc(side, nAttacks[P], nAttacks[P]);
	if (bbMob & bbPc(p, oppo, B) ) AddMisc(side, nAttacks[B], nAttacks[B]);
	if (bbMob & bbPc(p, oppo, R) ) AddMisc(side, nAttacks[R], nAttacks[R]);
	if (bbMob & bbPc(p, oppo, Q) ) AddMisc(side, nAttacks[Q], nAttacks[Q]);

	// king attacks (if our queen is present)
	bbAtt = bbMob & bbKingZone[side][p->kingSquare[oppo]];
	if (bbAtt && p->pcCount[side][Q] ) {
		AddKingAttack(side, N, PopCntSparse(bbAtt) );
		bbMinorCoorAttacks[side] ^= bbAtt;
	}

	// defending own king (if enemy queen is present)
	if (bbMob & bbKingZone[oppo][p->kingSquare[side]] 
	&& p->pcCount[oppo][Q] ) AddMisc(side, 5, 0);

	bbMob &= ~bbPawnTakes[oppo];               // exclude squares controlled by enemy pawns
	AddMobility(N, side, PopCnt15(bbMob) );    // evaluate mobility

    // check threats (excluding checks from squares controlled by enemy pawns)
	if (bbMob & bbKnightChecks[oppo] ) 
		checkCount[side] += canCheckWith[Data.safetyStyle][N]; 
  }
}

void sEvaluator::ScoreB(sPosition *p, int side) 
{
  int sq, ownPawnCnt, oppPawnCnt;
  const int oppo = Opp(side);
  U64 bbMob, bbAtt;
  U64 bbPieces    = bbPc(p, side, B);
  U64 bbOccupied  = OccBb(p) ^ bbPc(p, side, Q); // accept mobility through own queen

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                 // set piece location and clear it from bbPieces
	bbMob = GenCache.GetBishMob(bbOccupied, sq); // set control/mobility bitboard
	bbAllAttacks[side] |= bbMob;                 // update attack data
	ScoreRelationToPawns(p, side, B, sq);    

	// pawns on bishop color
	if (bbWhiteSq & SqBb(sq) ) { 
		ownPawnCnt = PopCntSparse( bbWhiteSq & bbPc(p, side, P) ) - 4;
		oppPawnCnt = PopCntSparse( bbWhiteSq & bbPc(p, oppo, P) ) - 4;
	} else {
		ownPawnCnt = PopCntSparse( bbBlackSq & bbPc(p, side, P) ) - 4;
		oppPawnCnt = PopCntSparse( bbBlackSq & bbPc(p, oppo, P) ) - 4; 
	}
	AddMisc(side,-3*ownPawnCnt-oppPawnCnt, -3*ownPawnCnt-oppPawnCnt);

	// attacks on enemy pieces
	if (bbMob & bbPc(p, oppo, P) ) AddMisc(side, nAttacks[P], nAttacks[P]);
	if (bbMob & bbPc(p, oppo, N) ) AddMisc(side, nAttacks[N], nAttacks[N]);
	if (bbMob & bbPc(p, oppo, R) ) AddMisc(side, nAttacks[R], nAttacks[R]);
	if (bbMob & bbPc(p, oppo, Q) ) AddMisc(side, nAttacks[Q], nAttacks[Q]);

    // king attack (if our queen is present)
	if (bbBCanAttack[sq] [KingSq(p, oppo) ] 
	&& (bbMob & bbKingZone[side][p->kingSquare[oppo]] ) ) {
       bbAtt = bbMob & bbKingZone[side][p->kingSquare[oppo]];
	   if (bbAtt && p->pcCount[side][Q] ) {
		   AddKingAttack( side, B, PopCntSparse(bbAtt) ); 
		   bbMinorCoorAttacks[side] ^= bbAtt;
	   }
   }

	// defending own king (if enemy queen is present)
	if (bbMob & bbKingZone[oppo][KingSq(p, side)] 
	&& p->pcCount[oppo][Q] ) AddMisc(side, 5, 0);

	bbMob &= ~bbPawnTakes[oppo];              // exclude squares controlled by enemy pawns
	AddMobility(B, side, PopCnt15(bbMob) );   // evaluate mobility

    // check threats (including false positives due to queen transparency)
	if (bbMob & bbDiagChecks[Opp(side)] )
		checkCount[side] += canCheckWith[Data.safetyStyle][B];

	// bishop blocked by own pawns
	if ( bbBadBishopMasks[side][sq] & bbPc(p, side, P) )
       AddMisc(side, Data.badBishopPenalty[side][sq], Data.badBishopPenalty[side][sq]);
  }
}

void sEvaluator::ScoreR(sPosition *p, int side) 
{
  int sq, contactSq;
  const int oppo = Opp(side);
  U64 bbMob, bbAtt, bbContact;
  U64 bbPieces   = bbPc(p, side, R);
  U64 bbOccupied = OccBb(p) ^ bbPc(p, side, Q) ^ bbPc(p, side, R); // R and Q are considered transparent

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                 // set piece location and clear it from bbPieces
	bbMob = GenCache.GetRookMob(bbOccupied, sq); // set control/mobility bitboard
	bbAllAttacks[side] |= bbMob;                 // update attack data
	ScoreRelationToPawns(p, side, R, sq);         

	// attacks on enemy pieces
	if (bbMob & bbPc(p, oppo, P) ) AddMisc(side, rAttacks[P], rAttacks[P]);
	if (bbMob & bbPc(p, oppo, N) ) AddMisc(side, rAttacks[N], rAttacks[N]);
	if (bbMob & bbPc(p, oppo, B) ) AddMisc(side, rAttacks[B], rAttacks[B]);
	if (bbMob & bbPc(p, oppo, Q) ) AddMisc(side, rAttacks[Q], rAttacks[Q]);

	U64 bbFrontSpan = GetFrontSpan(SqBb(sq), side );
	if (bbFrontSpan & bbPc(p, oppo, Q) ) AddMisc(side, 5, 5); // rook and enemy queen in the same file

	// rook on an open file (ignoring pawns behind a rook)
	if ( !(bbFrontSpan & bbPc(p,side, P) ) ) {                // no own pawns in front of the rook
	   if ( !(bbFrontSpan & bbPc(p, oppo, P) ) ) {            // no enemy pawns - open file
		  AddMisc(side, rookOpenMg, rookOpenEg);
		  if (bbFrontSpan & bbKingZone[side][KingSq(p, oppo)] ) attCount[side] += rookOpenAttack[Data.safetyStyle];
	   } else {                                               // enemy pawns present - semi-open file
		  AddMisc(side, rookSemiOpenMg, rookSemiOpenEg);
		  if (bbFrontSpan & bbKingZone[side][KingSq(p, oppo)] ) attCount[side] += rookSemiOpenAttack[Data.safetyStyle];
	   }
	}

	// rook on 7th rank attacking pawns or cutting off enemy king
	if (SqBb(sq) & bbRelRank[side][RANK_7] ) {
       if ( bbPc(p, oppo, P) & bbRelRank[side][RANK_7]
	   ||   bbPc(p, oppo, K) & bbRelRank[side][RANK_8]
	   )  AddMisc(side, rookSeventhMg, rookSeventhEg);
	}

    // check threats (including false positives due to queen/rook transparency)
	if (bbMob & bbStraightChecks[oppo] ) {
		checkCount[side] += canCheckWith[Data.safetyStyle][R];
        // safe contact checks
	    bbContact = bbMob & bbKingAttacks[ KingSq(p, oppo) ] & bbStraightChecks[oppo];
	    while (bbContact) {
           contactSq = PopFirstBit(&bbContact);

	       if ( Swap(p, sq, contactSq) >= 0 ) {
			  checkCount[side] += rookContactCheck[Data.safetyStyle]; 
		      break;
	       }
	    }
	}

	// king attack (if our queen is present)
	if ( bbRCanAttack[sq] [KingSq(p, oppo) ]  
	&& ( bbMob & bbKingZone[side][p->kingSquare[oppo]] ) ) {
       bbAtt = bbMob & bbKingZone[side][p->kingSquare[oppo]];
	   if (bbAtt && p->pcCount[side][Q]) {
		   AddKingAttack(side, R, PopCntSparse(bbAtt) );
		   attCount[side] += PopCntSparse( bbAtt & bbMinorCoorAttacks[side] ); // rook-minor coordination
		   attCount[side] += PopCntSparse( bbAtt & bbRookCoorAttacks[side] );  // rook-rook coordination
		   bbRookCoorAttacks[side] ^= bbAtt;
	   }
	}
	
	AddMobility(R, side, PopCnt15(bbMob) );
  }
}

void sEvaluator::ScoreQ(sPosition *p, int side) 
{
  int sq, contactSq;
  const int oppo = Opp(side);
  U64 bbMob, bbAttacks, bbContact, bbAtt;
  U64 bbPieces       = bbPc(p, side, Q); 
  U64 bbOccupied     = OccBb(p); // real occupancy, since we'll look for contact checks
  U64 bbTransparent  = bbPc(p, side, R) | bbPc(p, side, B);
  U64 bbCanCheckFrom = bbStraightChecks[oppo] | bbDiagChecks[oppo];

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                  // set piece location and clear it from bbPieces 
	bbMob = GenCache.GetQueenMob(bbOccupied, sq); // set control/mobility bitboard
	bbAllAttacks[side] |= bbMob;                  // update attack data

	// attacks on enemy pieces
	if (bbMob & bbPc(p, oppo, P) ) AddMisc(side, qAttacks[P], qAttacks[P]);
	if (bbMob & bbPc(p, oppo, N) ) AddMisc(side, qAttacks[N], qAttacks[N]);
	if (bbMob & bbPc(p, oppo, B) ) AddMisc(side, qAttacks[B], qAttacks[B]);
	if (bbMob & bbPc(p, oppo, R) ) AddMisc(side, qAttacks[N], qAttacks[R]);

	// queen on 7th rank attacking pawns or cutting off enemy king
	if (SqBb(sq) & bbRelRank[side][RANK_7] ) {
       if ( bbPc(p, oppo, P) & bbRelRank[side][RANK_7]
	   ||   bbPc(p, oppo, K) & bbRelRank[side][RANK_8]
	   )  AddMisc(side, 10, 5);
	}

	if (bbMob & bbCanCheckFrom ) {
		// queen check threats (unlike with other pieces, we *count the number* of possible checks here)
		checkCount[side] += PopCntSparse( bbMob & bbCanCheckFrom ) * canCheckWith[Data.safetyStyle][Q];

        // safe contact checks
	    bbContact = bbMob & bbKingAttacks[ KingSq(p, oppo) ];
	    while (bbContact) {
           contactSq = PopFirstBit(&bbContact);

	       if ( Swap(p, sq, contactSq) >= 0 ) {
			  checkCount[side] += queenContactCheck[Data.safetyStyle]; 
		      break;
	       }
	    }
	}

    if (bbQCanAttack[sq] [KingSq(p, oppo) ]
	&& (attCount[side] > 0 || p->pcCount[side][Q] > 1) ) // otherwise queen attack won't change score
	{
	   // factor in queen attacks through other pieces
	   if (bbMob && bbTransparent)
	      bbAttacks = QAttacks(bbOccupied ^ bbTransparent ^ SqBb(sq), sq);
	   else 
	      bbAttacks = bbMob;

	   // count attacks
	   bbAtt = bbAttacks & bbKingZone[side][KingSq(p, oppo)];
	   if (bbAtt) {
		   AddKingAttack(side, Q, PopCntSparse(bbAtt) );	   
		   attCount[side] += PopCntSparse( bbAtt & bbMinorCoorAttacks[side] ); // coordinated Queen - minor attacks
		   attCount[side] += PopCntSparse( bbAtt & bbRookCoorAttacks[side] ); // coordinated Queen - rook attacks
	   }
	}
	
	AddMobility(Q, side, PopCnt(bbMob) );
  }
}

void sEvaluator::ScoreP(sPosition *p, int side) 
{
  const int oppo = Opp(side);
  int sq, passUnitMg, passUnitEg, flagIsWeak;
  U64 bbPieces = bbPc(p, side, P);
  U64 bbOccupied = OccBb(p);
  U64 bbStop, bbBack, bbObstacles;

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);
	bbStop = ShiftFwd(SqBb(sq), side);
	bbBack = SqBb(sq) ^ ShiftFwd(SqBb(sq), oppo);
	flagIsWeak = ( ( bbPawnSupport[side][sq] & bbPc(p,side, P) ) == 0);

	if ( !flagIsWeak                       // technically speaking, this pawn is not weak, 
	&&   !(bbBack & bbPawnTakes[side]) )   // but it has lost contact  with the pawn mass,
	AddMisc(side,-4,-8);                   // so it is at least slightly vulnerable.
	
	if (bbStop &~bbOccupied) {             // this pawn is mobile
	   if (Data.pstMg[side][P][sq] > 0)    // bonus gets bigger for well positioned pawns
		   AddMisc(side, Data.pstMg[side][P][sq] / 5, 2);
	   else AddMisc(side, 2, 1);
	}
	   
	bbObstacles = bbPassedMask[side][sq] & bbPc(p, oppo, P);

	// additional evaluation of a passed pawn 
	if (!bbObstacles) {
		passUnitMg = ( Data.pawnProperty[PASSED][MG][side][sq] * Data.passedPawns ) / 500;
		passUnitEg = ( Data.pawnProperty[PASSED][EG][side][sq] * Data.passedPawns ) / 500;

		// enemy king distance to a passer (failed to find good value for a friendly king)
		AddMisc(side, 0, (-Data.distance[sq] [p->kingSquare[Opp(side)]] * passUnitEg) / 6);

		// blocked and unblocked passers
		if (bbStop &~bbOccupied) AddMisc(side,  passUnitMg,  passUnitEg);
		else                     AddMisc(side, -passUnitMg, -passUnitEg);

		// control of stop square
		if (bbStop &~ bbAllAttacks[oppo] ) {
                   AddMisc(side,  passUnitMg,  passUnitEg);
                   if (bbStop & bbAllAttacks[side] ) AddMisc(side,  passUnitMg,  passUnitEg);
		}
	}
  }
}

void sEvaluator::AddKingAttack(int side, int pc, int cnt)
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

void sEvaluator::ScoreRelationToPawns(sPosition *p, int side, int piece, int sq) 
{
	const int oppo = Opp(side);

	if ( SqBb(sq) & bbPawnTakes[oppo] )  
		AddMisc(side, -pAttacks[piece], -pAttacks[piece]); // piece attacked by a pawn
	else if ( SqBb(sq) & bbPawnTakes[side] )  
		AddMisc(side, pawnDefendsMg[piece], pawnDefendsEg[piece]); // piece defended by a pawn

	// constant bonus if piece occupies hole of enemy pawn structure
	if ( SqBb(sq) & ~bbPawnCanTake[oppo] ) {
		AddMisc(side, outpostBase[piece], outpostBase[piece] );

	   // additional pst bonus if piece occupying a hole is defended by a pawn
	   if ( SqBb(sq) & bbPawnTakes[side] ) 
		   AddMisc(side, Data.outpost[side][piece][sq], Data.outpost[side][piece][sq] );
	}
}
