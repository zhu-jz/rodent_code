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
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "rodent.h"
#include "timer.h"
#include "trans.h"
#include "hist.h"
#include "book.h"
#include "eval/eval.h"
#include "bitboard/bitboard.h"  // for SqBb and REL_SQ macros
#include "search/search.h"
#include "parser.h"

void sParser::ReadLine(char *str, int n)
{
  char *ptr;

  if (fgets(str, n, stdin) == NULL)
    exit(0);
  if ((ptr = strchr(str, '\n')) != NULL)
    *ptr = '\0';
}

char *sParser::ParseToken(char *string, char *token)
{
  while (*string == ' ')
    string++;
  while (*string != ' ' && *string != '\0')
    *token++ = *string++;
  *token = '\0';
  return string;
}

void sParser::UciLoop(void)
{
  char command[4096], token[80], *ptr;
  sPosition p[1];

  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  SetPosition(p, START_POS);
  TransTable.Alloc(16);

  for (;;) {
    ReadLine(command, sizeof(command));
    ptr = ParseToken(command, token);

	// boolean options
    if (strstr(command, "setoption name UCI_LimitStrength value"))	
		Data.useWeakening = (strstr(command, "value true") != 0);
	if (strstr(command, "setoption name Analyse value"))
		Data.isAnalyzing = (strstr(command, "value true") != 0);
	if (strstr(command, "setoption name UseBook value"))
		Data.useBook = (strstr(command, "value true") != 0);
	if (strstr(command, "setoption name PositionLearning value"))
		Data.useLearning = (strstr(command, "value true") != 0);
	if (strstr(command, "setoption name Verbose value"))
		Data.verbose = (strstr(command, "value true") != 0);

    if (strcmp(token, "uci") == 0) {
      flagProtocol = PROTO_UCI;
      PrintEngineHeader();
      PrintUciOptions(); 
	  printf("uciok\n");
	} else if (strcmp(token, "txt") == 0) {
      flagProtocol = PROTO_TXT;
    } else if (strcmp(token, "isready") == 0) {
      printf("readyok\n");
    } else if (strcmp(token, "setoption") == 0) {
      SetOption(ptr);
    } else if (strcmp(token, "position") == 0) {
      ParsePosition(p, ptr);
    } else if (strcmp(token, "ucinewgame") == 0) {
	  History.OnNewGame();
    } else if (strcmp(token, "go") == 0) {
      ParseGo(p, ptr);
    } else if (strcmp(token, "step") == 0) {
      ParseMoves(p, ptr);
    } else if (strcmp(token, "split") == 0) {
	  Book.FileFixer("dense.txt","loose.txt", SPLIT_CONTINOUS);
    } else if (strcmp(token, "quotes") == 0) {
      Book.FileFixer("noquote.txt","quote.txt", ADD_QUOTES);
    } else if (strcmp(token, "feedbook") == 0) {
	  ptr = ParseToken(ptr, token);
      Book.FeedMainBook(p, atoi(token));
    } else if (strcmp(token, "bookdoctor") == 0) {
      Book.BookDoctor(p);
    } else if (strcmp(token, "print") == 0) {
      PrintBoard(p);
    } else if (strcmp(token, "eval") == 0) {
      printf(" %d \n", Eval.ReturnFull(p, -32000,32000) );
    } else if (strcmp(token, "signature") == 0) {
      printf(" Command \"bench 8\" should search %d nodes \n", BENCH_8 );
    } else if (strcmp(token, "bench") == 0) {
		ptr = ParseToken(ptr, token);
		Searcher.Bench(atoi(token) );
    } else if (strcmp(token, "perft") == 0) {
		ptr = ParseToken(ptr, token);
		Searcher.ShowPerft(p, atoi(token) );
    } else if (strcmp(token, "divide") == 0) {
		ptr = ParseToken(ptr, token);
		Searcher.Divide(p, 0, atoi(token) );
    } else if (strcmp(token, "quit") == 0) {
      exit(0);
    }
  }
}

void sParser::SetOption(char *ptr)
{
  char token[80], name[80], value[80];
  sPosition p;

  ptr = ParseToken(ptr, token);
  name[0] = '\0';

  for (;;) {
    ptr = ParseToken(ptr, token);
    if (*token == '\0' || strcmp(token, "value") == 0)
      break;
    strcat(name, token);
    strcat(name, " ");
  }

  name[strlen(name) - 1] = '\0';

  if (strcmp(token, "value") == 0) {
    value[0] = '\0';

    for (;;) {
      ptr = ParseToken(ptr, token);
      if (*token == '\0')
        break;
      strcat(value, token);
      strcat(value, " ");
    }

    value[strlen(value) - 1] = '\0';
  }

    if (strcmp(name, "Queen") == 0) {
	Data.matValue[Q] = atoi(value);
  } else if (strcmp(name, "Rook") == 0) {
	Data.matValue[R] = atoi(value);
  } else if (strcmp(name, "Bishop") == 0) {
	Data.matValue[B] = atoi(value);
  } else if (strcmp(name, "Knight") == 0) {
	Data.matValue[N] = atoi(value);
  } else if (strcmp(name, "BishPair") == 0) {
	Data.bishopPair = atoi(value);
  } else if (strcmp(name, "OwnMobility") == 0) {
	Data.ownMobility = atoi(value);
  } else if (strcmp(name, "OppMobility") == 0) {
	Data.oppMobility = atoi(value);
  } else if (strcmp(name, "OwnAttack") == 0) {
	Data.ownAttack = atoi(value);
  } else if (strcmp(name, "OppAttack") == 0) {
	Data.oppAttack = atoi(value);
  } else if (strcmp(name, "LMRHistLimit") == 0) {
	Data.lmrHistLimit = atoi(value);
  } else  if (strcmp(name, "FutilityDepth") == 0) {
	Data.futilityDepth = atoi(value);
  } else  if (strcmp(name, "FutilityBase") == 0) {
	Data.futilityBase = atoi(value);
  } else if (strcmp(name, "FutilityStep") == 0) {
	Data.futilityStep = atoi(value);
  } else if (strcmp(name, "UCI_Elo") == 0) {
	Data.elo = atoi(value);
	if (Data.panelStyle == PANEL_NORMAL) Data.useWeakening = 1;
  } else if (strcmp(name, "Hash") == 0) {
    TransTable.Alloc(atoi(value));
  } else if (strcmp(name, "Clear Hash") == 0) {
    TransTable.Clear();
  } else if (strcmp(name, "Strength") == 0) {
	   char styleName[30] = "personalities/";	   
	   strcat(styleName,value);
	   strcat(styleName,".txt");
	   ReadPersonality(styleName);
	   strcpy(Data.currLevel, value);
  } else if (strcmp(name, "Style") == 0) {

	   char styleName[30] = "personalities/";	   
	   strcat(styleName,value);
	   strcat(styleName,".txt");
	   ReadPersonality(styleName);
	   strcpy(Data.currStyle, value);
  } else if (strcmp(name, "Book") == 0) {

	   char bookName[30] = "books/";	   
	   strcat(bookName,value);
	   strcat(bookName,".txt");
	   strcpy(Data.currBook, value);
	   Book.ReadTextFileToGuideBook(&p, bookName, NO_CL);
  }
}

void sParser::PrintUciOptions() 
{		
	if (Data.panelStyle == PANEL_POWER) {
	printf("option name Queen type spin default %d min 0 max 1200\n", Data.matValue[Q] );
	printf("option name Rook type spin default %d min 0 max 1200\n", Data.matValue[R] );
	printf("option name Bishop type spin default %d min 0 max 1200\n", Data.matValue[B] );
	printf("option name Knight type spin default %d min 0 max 1200\n", Data.matValue[N] );
	printf("option name BishPair type spin default %d min 0 max 100\n", Data.bishopPair );
	printf("option name OwnMobility type spin default %d min 0 max 300\n", Data.ownMobility);
	printf("option name OppMobility type spin default %d min 0 max 300\n", Data.oppMobility);
	printf("option name OwnAttack type spin default %d min 0 max 300\n", Data.ownAttack);
	printf("option name OppAttack type spin default %d min 0 max 300\n", Data.oppAttack);
	printf("option name LMRHistLimit type spin default %d min 0 max 100\n", Data.lmrHistLimit);
	printf("option name FutilityDepth type spin default %d min 0 max 8\n", Data.futilityDepth);
	printf("option name FutilityBase type spin default %d min 0 max 1000\n", Data.futilityBase);
	printf("option name FutilityStep type spin default %d min 0 max 100\n", Data.futilityStep);
	printf("option name UCI_Elo type spin default %d min 600 max 2600\n", Data.elo);
	printf("option name UCI_LimitStrength type check default false\n", Data.useWeakening);
	}

	// normal user gets a list of predefined personalities of different playing strengths
	if (Data.panelStyle == PANEL_NORMAL) {
	   printf("option name Strength type combo");
	   printf(Data.levelList);
	   printf(" default GM\n");	   

	   printf("option name Style type combo");
	   printf(Data.styleList);
	   printf(" default Rodent\n");	   
	}

	   printf("option name Book type combo");
       printf(Data.bookList);
       printf(" default tournament\n");

	printf("option name Verbose type check default false\n", Data.verbose);
	printf("option name Analyse type check default false\n", Data.isAnalyzing);
	printf("option name UseBook type check default true\n", Data.useBook);
	printf("option name PositionLearning type check default false\n", Data.useLearning);
    printf("option name Hash type spin default 16 min 1 max 4096\n");
    printf("option name Clear Hash type button\n");
}

void sParser::ParsePosition(sPosition *p, char *ptr)
{
  char token[80], fen[80];

  ptr = ParseToken(ptr, token);
  if (strcmp(token, "fen") == 0) {
    fen[0] = '\0';
    for (;;) {
      ptr = ParseToken(ptr, token);
      if (*token == '\0' || strcmp(token, "moves") == 0)
        break;
      strcat(fen, token);
      strcat(fen, " ");
    }
    SetPosition(p, fen);
  } else {
    ptr = ParseToken(ptr, token);
    SetPosition(p, START_POS);
  }
  if (strcmp(token, "moves") == 0)
     ParseMoves(p, ptr);
}

void sParser::ParseMoves(sPosition *p, char *ptr)
{
  char token[80];
  UNDO u[1];
  int move;
  
    for (;;) {
      ptr = ParseToken(ptr, token);

      if (*token == '\0')
        break;
	  
	  move = StrToMove(p, token);
	  /*if (!IsLegal(p, move)) printf("move input error\n");
      else*/                 Manipulator.DoMove(p, move, u);
      
	  if (p->reversibleMoves == 0)
        p->head = 0;
	}
}

void sParser::ParseGo(sPosition *p, char *ptr)
{
  char token[80], bestmoveString[6];
  int pv[MAX_PLY];
  // TODO: move PV to search class

  Timer.Clear();
  pondering = 0;

  for (;;) {
    ptr = ParseToken(ptr, token);
    if (*token == '\0')
      break;
    if (strcmp(token, "ponder") == 0) {
      pondering = 1;
    } else if (strcmp(token, "wtime") == 0) {
      ptr = ParseToken(ptr, token);
      Timer.SetData(W_TIME, atoi(token) );
    } else if (strcmp(token, "btime") == 0) {
      ptr = ParseToken(ptr, token);
      Timer.SetData(B_TIME, atoi(token) );
    } else if (strcmp(token, "winc") == 0) {
      ptr = ParseToken(ptr, token);
      Timer.SetData(W_INC, atoi(token) );
    } else if (strcmp(token, "binc") == 0) {
      ptr = ParseToken(ptr, token);
      Timer.SetData(B_INC, atoi(token) );
    } else if (strcmp(token, "movestogo") == 0) {
      ptr = ParseToken(ptr, token);
      Timer.SetData(MOVES_TO_GO, atoi(token) );
    } else if (strcmp(token, "movetime") == 0) {
      ptr = ParseToken(ptr, token);
      Timer.SetData(MOVE_TIME, atoi(token) );
    } else if (strcmp(token, "nodes") == 0) {
      ptr = ParseToken(ptr, token);
	  printf("%d node limit", atoi(token));
      Timer.SetData(MAX_NODES, atoi(token) );
    } else if (strcmp(token, "depth") == 0) {
      ptr = ParseToken(ptr, token);
      Timer.SetData(MAX_DEPTH, atoi(token) );
	}
  }

  Timer.SetSideData(p->side);
  Timer.SetMoveTiming();
  
  Searcher.Think(p, pv);
  MoveToStr(pv[0], bestmoveString);
  if (pv[1]) {
    MoveToStr(pv[1], ponder_str);
    printf("bestmove %s ponder %s\n", bestmoveString, ponder_str);
  } else
    printf("bestmove %s\n", bestmoveString);
}

void sParser::ReadPersonality(char *fileName)
{
	 FILE *personalityFile; 
	 char line[256];
	 int lineNo = 0;
	 char token[80], *ptr;

      // exit if this personality file doesn't exist
	  if ( (personalityFile = fopen(fileName, "r")) == NULL ) 
		 return;

	  // read options line by line
	  while ( fgets(line, 256, personalityFile) ) {
		    ptr = ParseToken(line, token);
			if (strcmp(token, "setoption") == 0) 
				SetOption(ptr);
	  }
      
	  fclose(personalityFile);	  
}

void sParser::ReadIniFile(char *fileName)
{
	 FILE *iniFile; 
	 char line[512];

     // exit and load defaults if ini file doesn't exist
	 if ( (iniFile = fopen(fileName, "r")) == NULL ) {
         Data.panelStyle = PANEL_POWER;
		 strcpy(Data.bookList, " var active var broad var classic var hypermodern var gambit var normal var rodent var tournament");
         strcpy(Data.styleList, " var rodent");
		 strcpy(Data.levelList, " var GM");
		 return;
	 }

     // parse ini file
	 while ( fgets(line, 512, iniFile) ) {
		    if(line[0] == ';') continue; // don't process comment lines
			else if (strstr(line, "user normal") ) Data.panelStyle = PANEL_NORMAL;
			else if (strstr(line, "user power" ) ) Data.panelStyle = PANEL_POWER;
			else {
              char token[250];
 	          char *value;
 
              value = line;
              value = Parser.ParseToken(value, token);
			  if (strcmp(token, "bookstyle") == 0) strcpy(Data.bookList,value);
			  if (strcmp(token, "playstrength") == 0) strcpy(Data.levelList,value);
			  if (strcmp(token, "playstyle") == 0) strcpy(Data.styleList, value);
			}
	  }

      fclose(iniFile);
}

void sParser::PrintEngineHeader() 
{
    printf("id name Rodent 1.1");
	printf(" (build %d)\n", BUILD);
    printf("id author Pawel Koziol (based on Sungorus by Pablo Vazquez)\n");
}

void sParser::PrintBoard(sPosition *p)
{
  printf("--------------------------------------------\n");
  for (int sq = 0; sq < 64; sq++) {
	  if      (p->bbTp[P] & RelSqBb(sq,BLACK) ) { if ( p->bbCl[WHITE] & RelSqBb(sq,BLACK) ) printf("P "); else printf("p "); }
	  else if (p->bbTp[N] & RelSqBb(sq,BLACK) ) { if ( p->bbCl[WHITE] & RelSqBb(sq,BLACK) ) printf("N "); else printf("n "); }
	  else if (p->bbTp[B] & RelSqBb(sq,BLACK) ) { if ( p->bbCl[WHITE] & RelSqBb(sq,BLACK) ) printf("B "); else printf("b "); }
	  else if (p->bbTp[R] & RelSqBb(sq,BLACK) ) { if ( p->bbCl[WHITE] & RelSqBb(sq,BLACK) ) printf("R "); else printf("r "); }
	  else if (p->bbTp[Q] & RelSqBb(sq,BLACK) ) { if ( p->bbCl[WHITE] & RelSqBb(sq,BLACK) ) printf("Q "); else printf("q "); }
	  else if (p->bbTp[K] & RelSqBb(sq,BLACK) ) { if ( p->bbCl[WHITE] & RelSqBb(sq,BLACK) ) printf("K "); else printf("k "); }
	  else printf(". ");
	  if ( (sq+1) % 8 == 0) printf(" %d\n", 9 - ((sq+1) / 8) );
  }
  printf("\na b c d e f g h\n");
  printf("Incremental  hash: %llu pawn: %llu \n", p->hashKey, p->pawnKey);
  printf("Recalculated hash: %llu pawn: %llu \n", TransTable.InitHashKey(p), TransTable.InitPawnKey(p));
  printf("Incremental  pst : mg %d eg %d\n", p->pstMg[WHITE]-p->pstMg[BLACK], p->pstEg[WHITE]-p->pstEg[BLACK]);
  Eval.DebugPst(p);
  printf("\n");

  printf("--------------------------------------------\n");
}