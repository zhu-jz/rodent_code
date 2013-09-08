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

  const int att_N[12] = { 0,  4,  6,  7,  8,  9,  10,  11,  12,  13,  14,  15}; // increasing hurts
  const int att_B[12] = { 0,  4,  6,  7,  8,  9,  10,  11,  12,  13,  14,  15}; // increasing hurts
  const int att_R[12] = { 0,  8, 12, 16, 19, 22,  24,  26,  28,  30,  31,  32};   
  const int att_Q[12] = { 0, 10, 16, 22, 27, 33,  39,  46,  50,  52,  54,  56};  

  #define QUEEN_CONTACT_CHECK   30
  #define MINOR_ATTACKED_BY_P  -10
  #define MINOR_DEFENDED_BY_P    2

  //                           P   N   B   R   Q   K   -
  const int outpostBase [7] = {0,  4,  4,  0,  0,  0,  0};
  const int canCheckWith[7] = {0,  0,  1,  4, 10,  0,  0};
 
void sEvaluator::ScoreN(sPosition *p, int side) 
{
  int sq;
  U64 bbControl, bbAttZone;
  U64 bbPieces = bbPc(p, side, N);

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);      // set piece location and clear it from bbPieces
	bbControl = bbKnightAttacks[sq];  // set control bitboard
	bbAllAttacks[side] |= bbControl;  // update attack data
	bbControl &= ~p->bbCl[side];      // exclude squares occupied by own pieces

    // knight attacked / defended by pawn
	if ( SqBb(sq) & bbPawnControl[Opp(side)] )  AddMiscOne(side, MINOR_ATTACKED_BY_P);
	else 
    if ( SqBb(sq) & bbPawnControl[side] )  AddMiscOne(side, MINOR_DEFENDED_BY_P);

	ScoreOutpost(p, side, N, sq);

	bbAttZone = bbControl & bbKingZone[side][p->kingSquare[Opp(side)]];    // king attacks
	if (bbAttZone && p->pcCount[side][Q] ) 
       AddPieceAttack(side, att_N[PopCntSparse(bbAttZone)] ); 

	bbControl &= ~bbPawnControl[Opp(side)];      // exclude squares controlled by enemy pawns
	AddMobility(N, side, PopCnt15(bbControl) );  // evaluate mobility
  }
}

void sEvaluator::ScoreB(sPosition *p, int side) 
{
  int sq;
  U64 bbControl, bbAttZone;
  U64 bbPieces      = bbPc(p, side, B);
  U64 bbOccupied    = OccBb(p) ^ bbPc(p, side, Q);   // accept mobility through own queen

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                     // set piece location and clear it from bbPieces
	bbControl = GenCache.GetBishMob(bbOccupied, sq); // set control/mobility bitboard
	bbAllAttacks[side] |= bbControl;                 // update attack data

	// bishop attacked / defended by pawn
	if ( SqBb(sq) & bbPawnControl[Opp(side)] )  AddMiscOne(side, MINOR_ATTACKED_BY_P);
	else 
    if ( SqBb(sq) & bbPawnControl[side] )  AddMiscOne(side, MINOR_DEFENDED_BY_P);

	ScoreOutpost(p, side, B, sq);

    // check threats (with false positive due to queen transparency)
	if (bbControl & ( kingDiagChecks[Opp(side)] ) )
		attCount[side] += canCheckWith[B]; 

    // If we can attack zone around enemy king from the current square, 
	// test this possibility on the actual board.
	if (bbBCanAttack[sq] [KingSq(p, side ^ 1) ] 
	&& (bbControl & bbKingZone[side][p->kingSquare[Opp(side)]] ) ) {

       bbAttZone = bbControl & bbKingZone[side][p->kingSquare[Opp(side)]];
	   if (bbAttZone 
	   && p->pcCount[side][Q] ) // no attack eval without queens on board
		  AddPieceAttack( side, att_B[PopCntSparse(bbAttZone)] ); 
   }

	bbControl &= ~bbPawnControl[Opp(side)];      // exclude squares controlled by enemy pawns
	AddMobility(B, side, PopCnt15(bbControl) );  // evaluate mobility

	// penalize bishop blocked by own pawns
	if ( bbBadBishopMasks[side][sq] & bbPc(p, side, P) )
       AddMiscOne(side, Data.badBishopPenalty[side][sq]);
  }
}

void sEvaluator::ScoreR(sPosition *p, int side) 
{
  int sq;
  U64 bbControl, bbAttZone;
  U64 bbPieces   = bbPc(p, side, R);
  U64 bbOccupied = OccBb(p) ^ bbPc(p, side, Q) ^ bbPc(p, side, R);
  const U64 bbSeventh[2] = { bbRANK_7, bbRANK_2};
  const U64 bbEighth [2] = { bbRANK_8, bbRANK_1};

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                     // set piece location and clear it from bbPieces
	bbControl = GenCache.GetRookMob(bbOccupied, sq); // set control/mobility bitboard
	bbAllAttacks[side] |= bbControl;                 // update attack data

	ScoreOutpost(p, side, R, sq);

	// evaluate rook on an open file
	U64 bbFrontSpan = GetFrontSpan(SqBb(sq), side );

	if ( !(bbFrontSpan & bbPc(p,side, P) ) ) {
	   if ( !(bbFrontSpan & bbPc(p, Opp(side), P) ) ) 
	   {
		  AddMiscTwo(side, Data.rookOpenMg, Data.rookOpenEg);
		  if (bbFrontSpan & bbKingZone[side][p->kingSquare[Opp(side)]] ) attCount[side] += Data.rookOpenAttack;
	   }
	  else
	  {
		  AddMiscTwo(side, Data.rookSemiOpenMg, Data.rookSemiOpenEg);
		  if (bbFrontSpan & bbKingZone[side][p->kingSquare[Opp(side)]] ) attCount[side] += Data.rookSemiOpenAttack;
	  }
	}

	// evaluate rook on 7th rank if it attacks pawns or cuts off enemy king
	if (SqBb(sq) & bbSeventh[side] ) {
       if ( bbPc(p, Opp(side), P) & bbSeventh[side]
	   || bbPc(p, Opp(side), K) & bbEighth[side]  
	   )  AddMiscTwo(side, Data.rookSeventhMg, Data.rookSeventhEg);
	}

    // check threats (including false positives due to queen/rook transparency)
	if (bbControl & kingStraightChecks[Opp(side)] )
		attCount[side] += canCheckWith[R];

	// if we can attack enemy king from current square, test this possibility
	if ( bbRCanAttack[sq] [KingSq(p, side ^ 1) ]  
	&& ( bbControl & bbKingZone[side][p->kingSquare[Opp(side)]] ) ) {

       bbAttZone = bbControl & bbKingZone[side][p->kingSquare[Opp(side)]];
	   if (bbAttZone && p->pcCount[side][Q]) 
		  AddPieceAttack(side, att_R[ PopCntSparse(bbAttZone) ] );
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
  U64 bbCanCheckFrom = kingStraightChecks[Opp(side)] | kingDiagChecks[Opp(side)];

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);                      // set piece location and clear it from bbPieces 
	bbControl = GenCache.GetQueenMob(bbOccupied, sq); // set control/mobility bitboard
	bbAllAttacks[side] |= bbControl;                  // update attack data

	if (bbControl & bbCanCheckFrom ) {

		// queen check threats (unlike with other pieces, we *count the number* of possible checks here)
		attCount[side] += PopCntSparse( bbControl & bbCanCheckFrom ) * canCheckWith[Q];

        // safe contact checks
	    bbContact = bbControl & bbKingAttacks[ p->kingSquare[Opp(side)] ];
	    while (bbContact) {
           contactSq = PopFirstBit(&bbContact);

	       if ( Swap(p, sq, contactSq) >= 0 ) {
              attCount[side] += QUEEN_CONTACT_CHECK; 
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
	   if (bbAttZone) 
		   AddPieceAttack(side, att_Q[PopCntSparse(bbAttZone)] );	   
	}
	
	AddMobility(Q, side, PopCnt(bbControl) );
  }
}

void sEvaluator::ScoreP(sPosition *p, int side) 
{
  int sq;
  U64 bbPieces = bbPc(p, side, P);
  U64 bbOccupied = OccBb(p);
  U64 bbStop, bbObstacles;
  int passUnitMg, passUnitEg; 

  while (bbPieces) {
    sq = PopFirstBit(&bbPieces);
	bbStop = ShiftFwd(SqBb(sq), side);

	if (bbStop &~bbOccupied) {           // this pawn is mobile
	   if (Data.pstMg[side][P][sq] > 0)  // and placed on a good square
		   AddMiscTwo(side, Data.pstMg[side][P][sq] / 5, Data.pstEg[side][P][sq] / 5);
	   else AddMiscTwo(side, 2, 1);
	}
	   
	bbObstacles = bbPassedMask[side][sq] & bbPc(p, Opp(side), P);

	// additional evaluation of a passed pawn 
	if (!bbObstacles) {
		passUnitMg = Data.pawnProperty[PASSED][MG][side][sq] / 5;
		passUnitEg = Data.pawnProperty[PASSED][EG][side][sq] / 5;

		// blocked and unblocked passers
		if (bbStop &~bbOccupied) AddMiscTwo(side,  passUnitMg,  passUnitEg);
		else                     AddMiscTwo(side, -passUnitMg, -passUnitEg);

		// control of stop square
		if (bbStop &~ bbAllAttacks[Opp(side)] ) {
           AddMiscTwo(side,  passUnitMg,  passUnitEg);
           if (bbStop & bbAllAttacks[side] ) AddMiscTwo(side,  passUnitMg,  passUnitEg);
		}
	}
  }
}

void sEvaluator::AddPieceAttack(int side, int val)
{
    attNumber[side] += 1;
	attCount [side] += val;
}

void sEvaluator::AddMobility( int pc, int side, int cnt)
{
	mgMobility[side] += Data.mobBonusMg [pc] [cnt];
	egMobility[side] += Data.mobBonusEg [pc] [cnt];
}

void sEvaluator::AddMiscOne(int side, int val)
{
	mgMisc[side] += val;
	egMisc[side] += val;
}

void sEvaluator::AddMiscTwo(int side, int mg, int eg)
{
	mgMisc[side] += mg;
	egMisc[side] += eg;
}

void sEvaluator::ScoreOutpost(sPosition *p, int side, int piece, int sq) 
{
	// constant bonus if piece occupies hole of enemy pawn structure
	if ( SqBb(sq) & ~bbPawnCanControl[Opp(side)] ) {
		AddMiscOne(side, outpostBase[piece] );

	   // additional pst bonus if defended by a pawn
	   if ( SqBb(sq) & bbPawnControl[side] ) 
		   AddMiscOne(side, Data.outpost[side][piece][sq] );
	}
}