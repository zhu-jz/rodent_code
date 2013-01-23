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

#define MAX_LEARN_SIZE 48000

struct sLearnEntry {
   U64 hash;
   int depth;
   int val;
   int age;
};

struct sLearner {
   int learnSize;
   sLearnEntry learnData[MAX_LEARN_SIZE]; 
   void ParseLearnEntry(char * ptr, int line_no);
   void Init(char *fileName);
   void Save(char *fileName);
   void WriteLearnData(U64 hash, int depth, int val); 
   int ReadLearnData(U64 hash, int depth);
};

extern sLearner Learner;
