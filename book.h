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

struct sBookEntry {
  U64 hash;
  int move;
  int freq;
};

struct sBook {
sBookEntry myBook[1024000];
sBookEntry guideBook[48000];
int nOfRecords;
int nOfGuideRecords;
void Init(sPosition *p);

// variables used for move selection
int moves[100];
int nOfChoices;

char testString [12];

int ReadOwnBookFile(char *filename);
void SaveBookInOwnFormat(char *filename);
void ParseBookEntry(char * ptr, int line_no);

void BookDoctor(sPosition * p);
void ReadInternalToGuideBook(sPosition *p);
int  ReadTextFileToGuideBook(sPosition *p, char *fileName, int excludedColor);
void ReadMainBookFromOwnFile(sPosition *p, char *fileName, int excludedColor, int verifyDepth);
int PrintMissingMoves(sPosition *p);
void SplitContinousBookFormat(char *fileName);
void SortMainBook(void);
void AddMoveToGuideBook(U64 hash, int move, int val);
void AddMoveToMainBook(U64 hash, int move, int val);
void AddLineToGuideBook(sPosition *p, char *ptr, int excludedColor);
void AddLineToMainBook(sPosition *p, char *ptr, int excludedColor, int verifyDepth);
U64 GetBookHash(sPosition *p);
int GetBookMove(sPosition *p, int canPrint, int *flagIsProblem);
void FeedMainBook(sPosition *p, int verifyDepth);
int IsInfrequent(int val, int maxFreq);
int IsMoveInBook(U64 hashKey, int move);
};

extern sBook Book;