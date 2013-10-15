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

#define PV_NODE   0
#define CUT_NODE  1
#define ALL_NODE -1
#define NEW_NODE(type)     (-(type))

#define WAS_NULL  1
#define NO_NULL   0

enum eStatEntries { FAIL_HIGH, FAIL_FIRST, Q_NODES, END_OF_STATS};

struct sSearcher {
private:
	int flagAbortSearch;
	void CheckInput(void);
	int isReporting;
	U64 stat[END_OF_STATS];

	int Blunder(sPosition *p, int ply, int depth, int flag, int move, int lastMove, int flagInCheck);
	
	// report.c
	void IncStat(int slot);
	void ClearStats(void);
	void DisplayStats(void);
	void DisplayCurrmove(int move, int movesTried);
	void DisplayDepth(void);
	void DisplayPv(int score, int *pv);
	void DisplayRootInfo(void);
	void DisplaySettings(void);
	void DisplaySavedIterationTime();
	void DisplaySpeed(void);
	void PrintTxtHeader(void);
	U32  GetNps(int nodes, int time);

	// search.c
	int nodesPerBranch;
	int DrawBy50Moves(sPosition *p);
	void Iterate(sPosition *p, int *);
	int IsRepetition(sPosition *p);
	int IsMoveOrdinary(int flagMoveType);
	int AvoidReduction(int move, int flagMoveType);
	int Perft(sPosition *p, int ply, int depth);
	int SearchRoot(sPosition *p, int alpha, int beta, int depth, int *pv);
	int Search(sPosition *p, int ply, int alpha, int beta, int depth, int nodeType, int wasNull, int lastMove, int *pv);
	int SetFutilityMargin(int depth);
	int SetNullDepth(int depth);
	
	int RecognizeDraw(sPosition *p);
	int nodes;
	int rootDepth; 
	int minimalLmrDepth;
    int minimalNullDepth;
	double lmrSize[3][MAX_PLY * ONE_PLY][MAX_MOVES];
	sFlatMoveList rootList;
public:
	void Init(void);
	int bestMove;
	int Quiesce(sPosition *p, int ply, int qDepth, int alpha, int beta, int isRoot, int *pv);
	int QuiesceSmart(sPosition *p, int ply, int qDepth, int alpha, int beta, int isRoot, int *pv); 
	void Think(sPosition *, int *);
	void ShowPerft(sPosition *p, int depth);
	void Divide(sPosition *p, int ply, int depth);
	void Bench(int depth);
	int VerifyValue(sPosition *p, int depth, int move);
};

extern struct sSearcher Searcher;