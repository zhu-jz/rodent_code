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
#include "stdlib.h"
#include "rodent.h"
#include "data.h"
#include "trans.h"
#include "parser.h"
#include "learn.h"

void sLearner::Init(char *fileName)
{
	 FILE *learnFile; 
	 char line[256];
	 int lineNo = 0;

      // exit if learn file doesn't exist
	  if ( (learnFile = fopen(fileName, "r")) == NULL ) 
		 return;

     
	 // TODO: read learn file line by line, increasing age of each entry
	  	  while ( fgets(line, 256, learnFile) ) {
		    ++lineNo;
		    ParseLearnEntry(line, lineNo);
	  }

      learnSize = lineNo;
      fclose(learnFile);
}

void sLearner::ParseLearnEntry(char * ptr, int line_no) {

  char token[256];
  int token_no = 1;

  for (;;) 
  {
    ptr = Parser.ParseToken(ptr, token);
	if (token_no == 1) learnData[line_no].hash = atoull(token);
	if (token_no == 2) learnData[line_no].depth = atoi(token); 
	if (token_no == 3) learnData[line_no].val = atoi(token); 
	token_no++;
	  
    if (*token == '\0') break;
  }

}


void sLearner::WriteLearnData(U64 hash, int depth, int val)
{
    hash = hash / 4;
	if ( hash < 0 ) hash *= -1;

    // update existing entry 
	for (int i = 0; i < learnSize; i++ ) 
	 {
		 if ( learnData[i].hash == hash )
		 {
		    if ( learnData[i].depth <= depth )
			{
				learnData[i].depth = depth;
				learnData[i].val   = val;
			}
		 return;
		 }
	 } 

	 if (learnSize < MAX_LEARN_SIZE)
	 {
	   learnData[learnSize].hash  = hash;
	   learnData[learnSize].depth = depth;
	   learnData[learnSize].val   = val;
	   learnData[learnSize].age   = 0;
	   learnSize++;
	 }
	 // TODO: else replace the shallowest of the oldest entries
}

int sLearner::ReadLearnData(U64 hash, int depth)
{
    hash = hash / 4;
	if ( hash < 0 ) hash *= -1;

   // TODO: reset age on a visit
     for (int i = 0; i < learnSize; i++ ) 
	 {
		 if ( learnData[i].hash == hash && learnData[i].depth >= depth )
			 return learnData[i].val;
     }

   return INVALID;
}

void sLearner::Save(char *fileName) {
	 FILE *learnFile; 

     if (!Data.useLearning) return;

     learnFile = fopen(fileName,"w"); 
	 for (int i = 0; i < learnSize; i++ ) 
	 {
		 if (learnData[i].hash != 0)
		 fprintf(learnFile, "%I64u, %d, %d \n", learnData[i].hash, learnData[i].depth, learnData[i].val);
	 }
     fclose(learnFile);
}
