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
#include <string.h>
#include "../rodent.h"
#include "../trans.h"
#include "../move/move.h"
#include "../hist.h"
#include "../selector.h"
#include "../timer.h"
#include "search.h"

void sSearcher::Bench(int depth)
{
	sPosition p[1];
	int pv[MAX_PLY];
	char *test[] =  // test positions taken from DiscoCheck by Lucas Braesch
{
"r1bqkbnr/pp1ppppp/2n5/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq -",
"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
"4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
"rq3rk1/ppp2ppp/1bnpb3/3N2B1/3NP3/7P/PPPQ1PP1/2KR3R w - - 7 14",
"r1bq1r1k/1pp1n1pp/1p1p4/4p2Q/4Pp2/1BNP4/PPP2PPP/3R1RK1 w - - 2 14",
"r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15",
"r1bbk1nr/pp3p1p/2n5/1N4p1/2Np1B2/8/PPP2PPP/2KR1B1R w kq - 0 13",
"r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16",
"4r1k1/r1q2ppp/ppp2n2/4P3/5Rb1/1N1BQ3/PPP3PP/R5K1 w - - 1 17",
"2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11",
"r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16",
"3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22",
"r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18",
"4k2r/1pb2ppp/1p2p3/1R1p4/3P4/2r1PN2/P4PPP/1R4K1 b - - 3 22",
"3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26",
NULL
};

	printf("Bench test started (depth %d): \n", depth);
	Timer.Clear();
	Timer.SetData(MAX_DEPTH, depth );
	Timer.SetStartTime();
	ClearStats();

	for (int i = 0; test[i]; ++i) {
		TransTable.Clear();
		History.OnNewGame();
		printf(test[i]);
		SetPosition(p, test[i]);
		printf("\n");
		if (flagProtocol == PROTO_TXT) PrintTxtHeader();
		Iterate(p, pv);
	}

	int endTime = Timer.GetElapsedTime();
	U32 nps  = GetNps(nodes, endTime);
	printf("%d nodes searched in %d, speed %u nps (Score: %.3f)\n", nodes, endTime, nps, (float)nps/430914.0);
	DisplayStats();
}

int sSearcher::Perft(sPosition *p, int ply, int depth) 
{
	UNDO  undoData[1];  // data required to undo a move
	sSelector Selector;
	int move = 0;
	int flagMoveType;
	int refutationSq = 0;
	int nOfMoves = 0;

    Selector.InitMoves(p, move, ply);

    // LOOP THROUGH THE MOVE LIST
    while ( move = Selector.NextMove(refutationSq, &flagMoveType) ) 
	{
	   // MAKE A MOVE
	   Manipulator.DoMove(p, move, undoData);    
	
	   // UNDO ILLEGAL MOVES
	   if (IllegalPosition(p)) { 
		   Manipulator.UndoMove(p, move, undoData); 
		   continue; 
	   }

       if (depth == 1) nOfMoves++;
	   else            nOfMoves += Perft(p, ply+1, depth-1);
	   Manipulator.UndoMove(p, move, undoData);
  }

	return nOfMoves;
}

void sSearcher::ShowPerft(sPosition *p, int depth)
{
	 Timer.SetStartTime();
	 printf("perft %d: %12d ", depth, Searcher.Perft(p, 0, depth ) );
	 printf("%d \n", Timer.GetElapsedTime() ); 
}

void sSearcher::Divide(sPosition *p, int ply, int depth)
{
	UNDO  undoData[1];  // data required to undo a move
	sSelector Selector;
	int move = 0;
	int flagMoveType;
	int refutationSq = 0;
	int nOfMoves = 0;

	Timer.SetStartTime();

    Selector.InitMoves(p, move, ply);

    // LOOP THROUGH THE MOVE LIST
    while ( move = Selector.NextMove(refutationSq, &flagMoveType) ) 
	{
	   // MAKE A MOVE
	   Manipulator.DoMove(p, move, undoData);    
	
	   // UNDO ILLEGAL MOVES
	   if (IllegalPosition(p)) { 
		   Manipulator.UndoMove(p, move, undoData); 
		   continue; 
	   }

       if (depth == 1) nOfMoves++;
	   else  
	   {   
		   int fraction = Perft(p, ply+1, depth-1);
		   nOfMoves += fraction;
		   PrintMove(move);
		   printf(": %d\n", fraction );
	   }

	   Manipulator.UndoMove(p, move, undoData);
  }

	printf("total: %d\n", nOfMoves);
	printf("%d \n", Timer.GetElapsedTime() ); 
}