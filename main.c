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

#include "data.h"
#include "bitboard/bitboard.h"
#include "rodent.h"
#include "bitboard/gencache.h"
#include "eval/eval.h"
#include "search/search.h"
#include "timer.h"
#include "trans.h"
#include "hist.h"
#include "parser.h"
#include "learn.h"
#include "book.h"

int flagProtocol;

// struct instances
sParser     Parser;       // UCI parser  
sData       Data;         // configurable data affecting engine performance
sEvaluator  Eval;         // evaluation function and subroutines 
sGenCache   GenCache;     // caching generated bitboards for minimal speedup
sManipulator Manipulator; // functions for making and unmaking moves
sSearcher   Searcher;     // search function and subroutines
sTimer      Timer;        // setting and observing time limits
sTransTable TransTable;   // transposition table
sHistory    History;      // history and killer tables
sLearner    Learner;      // position learning facility
sBook       Book;         // opening book 

int main()
{
  sPosition p;
  flagProtocol = PROTO_TXT;
  Init();
  Parser.ReadIniFile("rodent.ini"); // initialize variables governing how the engine appears to a GUI
  History.OnNewGame();
  Learner.Init("lrn.dat");
  Book.Init(&p);
  Searcher.Init();
  Book.bookName = "rodent.bin";
  Book.OpenPolyglot();
  Parser.UciLoop();
  Book.ClosePolyglot();
  Learner.Save("lrn.dat");
  return 0;
}
