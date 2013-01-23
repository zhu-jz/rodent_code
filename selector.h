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

// class responsible for maintaining move list 
// and returning moves in a predefined order

struct sSelector {
private:
   MOVES m[1];  // move list
   
   int MvvLva(sPosition *p, int move);
   void ScoreCaptures(int hashMove);
   void ScoreQuiet(int refutationSq);
   int SelectBest(void);
public:
	int BadCapture(sPosition *p, int move);
   void InitCaptures(sPosition *p, int hashMove);
   void InitMoves(sPosition *p, int transMove, int ply);
   int NextMove(int refutationSq, int *flag);
   int NextCapture(void);
};