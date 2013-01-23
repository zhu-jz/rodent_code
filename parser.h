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

struct sParser {
private:
	void ParseGo(sPosition *, char *);
	void ParseMoves(sPosition *p, char *ptr);
	void ParsePosition(sPosition *, char *);
	void PrintBoard(sPosition *p);
	void PrintUciOptions(void);
    void PrintEngineHeader(void);
	void ReadPersonality(char *fileName);
	void SetOption(char *);
public:
	void ReadIniFile(char *fileName);
	char *ParseToken(char *, char *);
	void ReadLine(char *, int);
	void UciLoop(void);
};

extern sParser Parser;