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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "timer.h"
#include "rodent.h"
#include "data.h"
#include "parser.h"
#include "trans.h"
#include "search/search.h"
#include "book.h"

#define DELETE_MOVE 888

void sBook::Init(sPosition * p) 
{
	 Timer.SetStartTime();
	 nOfRecords = 0;
	 nOfGuideRecords = 0;
	  if (START_POS !=  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -") {
		  printf("Unusual start position, opening book disabled!");
		  return;
	  }
	 
	 int hasMainBook  = ReadOwnBookFile("bigbook.wtf"); // read main book
	 int hasGuideBook = ReadTextFileToGuideBook(p, "guidebook.txt", NO_CL);
	 if (!hasMainBook && !hasGuideBook) ReadInternalToGuideBook(p);
}

void sBook::BookDoctor(sPosition * p) {
	 int move;
	 char moveStr[6];
	 UNDO u;

	 FILE *doctorFile; 
     doctorFile = fopen("doctor.txt","a+"); 

	 int flagIsProblem = 0;
	 printf("First line that might need Your decision will be saved to doctor.txt\n");
	 
	 for (int i = 0; i < 10000; i++ ) {
		 SetPosition(p, START_POS);
		 printf("\n");
		 fprintf(doctorFile, "\n");

        for (;;) {
		   move = GetBookMove(p, 0, &flagIsProblem);
		   if (move) { 
			  Manipulator.DoMove(p, move, &u);
              if (flagIsProblem) break; 
			  PrintMove(move);
			  MoveToStr(move, moveStr);
			  fprintf(doctorFile, "%s", moveStr);
	          printf(" ");
			  fprintf(doctorFile, " ");
		 }
		 else break;
		}
		if (flagIsProblem) break;
	 }
     fclose(doctorFile);
}

int sBook::GetBookMove(sPosition *p, int canPrint, int *flagIsProblem) {
	int i;
	int floorVal = 0;
	int curVal   = 0;
	int bestVal  = 0;
	int choice   = 0;
	int maxFreq  = 0;
	int values[100];
	U64 localHash = GetBookHash(p);

	nOfChoices = 0;

	if (Data.isAnalyzing) return 0; // book movea aren't returned in analyse mode

	for (i = 0; i < nOfGuideRecords; i++ ) {
		if (guideBook[i].hash == localHash
		&& IsLegal(p, guideBook[i].move ) )
		{
			moves[nOfChoices]  = guideBook[i].move;
			if (guideBook[i].freq > 0 ) values[nOfChoices] = guideBook[i].freq + 20;
			else                        values[nOfChoices] = -1;
	       // we add 20, so that any move not marked as bad in the text-based
		   // opening book has a chance to be played
			nOfChoices++;
		}
	}

	if (nOfChoices) {

       srand(Timer.GetMS() );

	   if (canPrint) printf("info string ");
	   for (i = 0; i < nOfChoices; i++ ) {
		   
		   // display info about possible choices
		   if (canPrint) printf(" ");
		   MoveToStr(moves[i], testString);
		   printf( testString );
		   if (canPrint) { 
			   if (values[i] > 0 ) printf(" %d ", values[i] );
		       else                printf("? ");
		   }

           // pick move with the best random value based on frequency
		   if (values[i] > 0) curVal = 1 + rand() % values[i]; 
		   else curVal = -1;

		   if (curVal > bestVal) {
			   bestVal = curVal;
			   choice  = i;
		   }
	   }
	   if (canPrint) printf(" guide \n");

       return moves[choice];
	}

	// find a convenient starting point to avoid looping through entire book
	int iStart  = 0;
	i = 1;
	if (nOfRecords > 10000) {
	   for(;;) {
         if (myBook[i*10000].hash < localHash) iStart = i*10000;
	     else break;         
	     i++;
	     if (i*10000 > nOfRecords) break;
	   }
	}

	// find possible book moves
	for (i = iStart; i < nOfRecords; i++ ) {
        if (myBook[i].hash > localHash) break;

		if (myBook[i].hash == localHash
		&& IsLegal(p, myBook[i].move ) ) {
			moves[nOfChoices]  = myBook[i].move;
			values[nOfChoices] = myBook[i].freq;
			nOfChoices++;
		}
	}

	if (nOfChoices) {
       // get maximum frequency of a move - it will be used
       // to filter out moves that were played not often enough
	   for (i = 0; i < nOfChoices; i++ ) {
          if (values[i] > maxFreq ) maxFreq = values[i];
	   }

       srand(Timer.GetMS() );

	   for (i = 0; i < nOfChoices; i++ ) {
		   
		   // display info about possible choices
		   if (canPrint) printf("info string ");
		   MoveToStr(moves[i], testString);
		   if (canPrint || (values[i] > 0 && IsInfrequent(values[i], maxFreq) ) ) printf( testString );

		   if (values[i] > 0 ) 
		   { 
			   if ( IsInfrequent(values[i], maxFreq) ) {
					values[i] = 0;
					printf(" infrequent ");
					*flagIsProblem = 1;
			   }
			   if (canPrint) printf(" %d ", values[i] );
		   }
		   else if (canPrint) printf("? ");
		   
           // pick move randomly, based on its value
		   // (we add 5 to ensure choice between equally rare moves)
		   if (values[i] > 0) curVal = 1 + rand() % (values[i] + 5); 
		   else curVal = -1;
		   if (curVal > bestVal) {
			   bestVal = curVal;
			   choice  = i;
		   }
		   if (canPrint) printf("\n");
	   }

      if ( PrintMissingMoves(p) ) *flagIsProblem = 1;
      if (bestVal > 0) return moves[choice];
	}

	if ( PrintMissingMoves(p) ) *flagIsProblem = 1;
    return 0;
}

int sBook::PrintMissingMoves(sPosition *p) {
    // show non-book moves leading to book positions
    sSelector Selector;
	UNDO undoData[1];
	int move = 0;
	int flagMoveType;
	int isProblem = 0;

    Selector.InitMoveList(p, move, MAX_PLY);

    while ( move = Selector.NextMove(0, &flagMoveType) ) {

	   Manipulator.DoMove(p, move, undoData);    
	
	   if (IllegalPosition(p)) { 
		  Manipulator.UndoMove(p, move, undoData); 
		  continue; 
	   }
     
	   for (int i = 0; i < nOfRecords; i++) 
	   {
		   if (myBook[i].hash == GetBookHash(p) ) {

              int isUsed = 0;
		      for (int j = 0; j < nOfChoices; j++) {
			      if (moves[j] == move) isUsed = 1;
		      }

		      if (!isUsed) {
		         printf("info string missing ");
			     isProblem = 1;
		         MoveToStr(move, testString);
		         printf( testString );
		         printf("\n");
		      }
		      break;
	       }
	   }

	   Manipulator.UndoMove(p, move, undoData); 
  }

  return isProblem;
}

void sBook::AddMoveToGuideBook(U64 hashKey, int move, int val) {

    // if move is already in the book, just change its frequency 
	for (int i = 0; i < nOfGuideRecords; i++ ) {
         if ( guideBook[i].hash == hashKey 
		 &&   guideBook[i].move == move) {
			  guideBook[i].freq += val;	 
			  return;
		 }
	 }  

	 // otherwise save it in the last slot
	 guideBook[nOfGuideRecords].hash = hashKey;
	 guideBook[nOfGuideRecords].move = move;
	 guideBook[nOfGuideRecords].freq = val;
	 nOfGuideRecords++;
}

void sBook::AddMoveToMainBook(U64 hashKey, int move, int val) {

    // if move is already in the book, just change its frequency 
	for (int i = 0; i < nOfRecords; i++ ) 
	{
         if ( myBook[i].hash == hashKey 
		 &&   myBook[i].move == move) {
			 if (val == DELETE_MOVE) myBook[i].hash = 0;
			 if (myBook[i].freq <= 20000 || val != 1) myBook[i].freq += val;
			 return;
		 }
	 }  

	 // otherwise save it in the last slot
	 if (val != DELETE_MOVE) {
	    myBook[nOfRecords].hash = hashKey;
	    myBook[nOfRecords].move = move;
	    myBook[nOfRecords].freq = val;
	 }
	 nOfRecords++;
}

int sBook::IsMoveInBook(U64 hashKey, int move)
{
	if (nOfRecords == 0) return 0;

	for (int i = 0; i < nOfRecords; i++ ) {
		if ( myBook[i].hash == hashKey 
		&&   myBook[i].move == move) {
		 return 1;
		}
	}
	return 0;
}

int sBook::AddLineToGuideBook(sPosition *p, char *ptr, int excludedColor)
{
  char token[512];
  UNDO u[1];
  int move;
  int freq;
  int flagIsProblem = 0;
    
  SetPosition(p, START_POS);

  for (;;) {
    ptr = Parser.ParseToken(ptr, token);
	  
    if (*token == '\0') break;

	move = StrToMove(p, token);
      
	if (IsLegal(p, move)) {
		// apply move frequency modifiers
		freq = 1;
        if (strstr(token, "?")) freq = -100;
		if (strstr(token, "!")) freq = +100;
        if (strstr(token, "??")) freq = -4900;
		if (strstr(token, "!!")) freq = +900;

		// add move to book if we accept moves of a given color
		if (p->side != excludedColor)
		   AddMoveToGuideBook( GetBookHash(p), move, freq);
        
		Manipulator.DoMove(p, move, u);
	}
	else { flagIsProblem = 1; break; };

    if (p->reversibleMoves == 0)
        p->head = 0;
    }
    return flagIsProblem;
}

void sBook::AddLineToMainBook(sPosition *p, char *ptr, int excludedColor, int verifyDepth)
{
  char token[2048];
  UNDO u[1];
  int move;
  int freq;
    
  SetPosition(p, START_POS);

  for (;;) {
    ptr = Parser.ParseToken(ptr, token);
	  
    if (*token == '\0') break;

	move = StrToMove(p, token);
      
	if (IsLegal(p, move)) {
		// apply move frequency modifiers
		freq = 1;
        if (strstr(token, "?")) freq  = -100;
		if (strstr(token, "!")) freq  = +100;
        if (strstr(token, "??")) freq = -4900;
		if (strstr(token, "!!")) freq = +900;
		if (strstr(token, "xx")) freq = DELETE_MOVE;

		// if asked for, verify new move with a search
        if (freq == 1 && !IsMoveInBook( GetBookHash(p), move ) && p->side != excludedColor && verifyDepth)
		{
			if ( Searcher.VerifyValue(p, verifyDepth, move) < -75 ) freq = -100;
		}

		// add move to book if we accept moves of a given color
		if (p->side != excludedColor)
		   AddMoveToMainBook( GetBookHash(p), move, freq);
        
		Manipulator.DoMove(p, move, u);
	}
	  else break;

    if (p->reversibleMoves == 0)
        p->head = 0;
    }
}


int sBook::ReadTextFileToGuideBook(sPosition *p, char *fileName, int excludedColor)
{
    FILE *bookFile; 
	char line[256];

    // exit if book file doesn't exist
	if ( (bookFile = fopen(fileName, "r")) == NULL ) return 0;

    nOfGuideRecords = 0; // clear any preexisting guide book

    // process book file line by line 
	while ( fgets(line, 250, bookFile) ) {
	  if ( line[0] == ';' ) continue; // don't process comment lines
	  if ( AddLineToGuideBook(p, line, excludedColor) ) { printf("Guide book error: "); printf(line); printf("\n"); }
	}
	fclose(bookFile);
	return 1;
 }

void sBook::FileFixer(char *inFileName, char *outFileName, int task)
{
    FILE *inFile, *outFile;
	char line[256];

    // exit if input file doesn't exist
	if ( (inFile = fopen(inFileName, "r")) == NULL ) { printf("File %s not found!\n", inFileName); return; };

	outFile = fopen(outFileName,"w"); 

    // process input file line by line 
	while ( fgets(line, 250, inFile) ) {
	   int length = strlen(line);

	   if (task == SPLIT_CONTINOUS) {
	      for (int i = 0; i < length;  i++) {
		     printf("%c", line[i]);
		     fprintf(outFile, "%c", line[i]);
		     if ( (i+1) % 4 == 0) fprintf(outFile," ");
	      }
	   }
	   else if (task == ADD_QUOTES) {
		  fprintf(outFile, "%c", '"');

		  for (int i = 0; i < length;  i++) {
			 printf("%c", line[i]);
			 fprintf(outFile, "%c", line[i]);
			 if (i== length-2) fprintf(outFile, "%c", '"');
		  }
	   }
	}
	fclose(inFile);
	fclose(outFile);
 }

void sBook::ReadMainBookFromOwnFile(sPosition *p, char *fileName, int excludedColor, int verifyDepth)
{
	FILE *bookFile; 
	char line[2048];
	int line_no = 0;

    // exit if book file doesn't exist
	if ( (bookFile = fopen(fileName, "r")) == NULL ) { printf("File %s not found!\n", fileName); return; };

    // process book file line by line 
	while ( fgets(line, 2040, bookFile) ) {
	    ++line_no;
		if ( line_no % 100 == 0 ) printf("Adding line no. %d\r",line_no);
	    if(line[0] == ';') continue; // don't process comment lines
	    AddLineToMainBook(p, line, excludedColor, verifyDepth);
	}

	printf("Adding line no. %d\r",line_no);
	fclose(bookFile);
 }


U64 sBook::GetBookHash(sPosition *p) 
{
	U64 bookKey = p->hashKey / 4;
	if (bookKey < 0) bookKey *= -1;
    return (signed long long) (bookKey);
}

void sBook::SaveBookInOwnFormat(char *fileName) 
{
	FILE *bookFile; 

    bookFile = fopen(fileName,"a+"); 
	for (int i = 0; i < nOfRecords; i++ ) {
		if (myBook[i].hash != 0)
		fprintf(bookFile, "%I64u, %d, %d \n", myBook[i].hash, myBook[i].move, myBook[i].freq);
	}
    fclose(bookFile);
}

int sBook::ReadOwnBookFile(char *fileName)
{
	FILE *bookFile; 
	char line[256];
	nOfRecords = 0;

    // exit if book file doesn't exist...
	if ( (bookFile = fopen(fileName, "r")) == NULL ) return 0;

    // ...else process book file line by line 
	while ( fgets(line, 250, bookFile) ) {
	   ParseBookEntry(line, nOfRecords);
	   ++nOfRecords;
	}

	fclose(bookFile);
	return 1;
 }

void sBook::ParseBookEntry(char * ptr, int line_no)
{
    char token[256];
    int token_no = 1;

    for (;;) {
       ptr = Parser.ParseToken(ptr, token);
	   if (token_no == 1) myBook[line_no].hash = atoull(token);
	   if (token_no == 2) myBook[line_no].move = atoi(token); 
	   if (token_no == 3) myBook[line_no].freq = atoi(token); 
	   token_no++;
	  
       if (*token == '\0') break;
   }
}

void sBook::SortMainBook(void) {
	int i, j, change;
	sBookEntry tmp;
 
   for (i=0; i<nOfRecords-1; ++i) {
       if (i % 100 == 0 ) printf ("%d   \r", i);

      change = 0;
      for (j = 0; j<nOfRecords-1-i; j++) { 
		  if (myBook[j+1].hash < myBook[j].hash // comparing neighbours
		  || (myBook[j+1].hash == myBook[j].hash && myBook[j+1].freq < myBook[j].freq ) ) {  
              tmp = myBook[j];      
              myBook[j] = myBook[j+1];
              myBook[j+1] = tmp;    // bubble goes up     
              change = 1;
          }
      }

      if(!change) break;      // no changes - book is sorted

      for (j=nOfRecords-2; j>1; j--) { 
		   if (myBook[j+1].hash < myBook[j].hash // comparing neighbours
		   || (myBook[j+1].hash == myBook[j].hash && myBook[j+1].freq < myBook[j].freq ) ) {  
              tmp = myBook[j];      
              myBook[j] = myBook[j+1];
              myBook[j+1] = tmp;    // bubble goes down     
              change = 1;
          }
      }
   }
};

void sBook::FeedMainBook(sPosition *p, int verifyDepth) 
{
	 printf("Feeding book moves for both sides\n");
	 ReadMainBookFromOwnFile(p, "feed.txt", NO_CL, verifyDepth);
	 printf("\nFeeding book moves for white only\n");
	 ReadMainBookFromOwnFile(p, "feed_white.txt", BLACK, verifyDepth);
	 printf("\nFeeding book moves for black only\n");
	 ReadMainBookFromOwnFile(p, "feed_black.txt", WHITE, verifyDepth);
	 printf("\nSorting book moves\n");
	 SortMainBook();
	 SaveBookInOwnFormat("newbook.wtf");
}

// Arbitrary definition of an infrequent move: less than 100 entries
// and less than 15% of frequency of the most popular move.
int sBook::IsInfrequent(int val, int maxFreq)
{
	return ( val < 100 && val < maxFreq / 15); 
}