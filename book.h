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

enum eFileTasks {SPLIT_CONTINOUS, ADD_QUOTES};

struct sBookEntry {
  U64 hash;
  int move;
  int freq;
};

struct polyglot_move {
    U64	key;
    int	move;
    int weight;
    int	n;
    int	learn;
};

struct sBook {
private:
   sBookEntry myBook[2048000];
   sBookEntry guideBook[48000];
   int nOfGuideRecords;
   int moves[100];
   int nOfChoices;
   char testString [12];
   void AddMoveToGuideBook(U64 hash, int move, int val);
   int AddLineToGuideBook(sPosition *p, char *ptr);
   U64 GetBookHash(sPosition *p);
   int IsInfrequent(int val, int maxFreq);
   void ParseBookEntry(char * ptr, int line_no);
   void PrintMissingMoves(sPosition *p);
   int FindPos(U64 key);
   void ReadEntry(polyglot_move * entry, int n);
   U64 ReadInteger(int size);
public:
   int GetPolyglotMove(sPosition *p, int printOutput);
   U64 GetPolyglotKey(sPosition *p);
   void OpenPolyglot(char *fileName);
   void ClosePolyglot(char *fileName);
   void Init(sPosition *p);
   int ReadTextFileToGuideBook(sPosition *p, char *fileName);
   int GetBookMove(sPosition *p, int canPrint, int *flagIsProblem);
};

extern sBook Book;
