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

#include "bitboard/bitboard.h"
#include "data.h"
#include "rodent.h"
#include "trans.h"

void SetPosition(sPosition *p, char *epd)
{
  int i, j, pc;
  static const char pc_char[13] = "PpNnBbRrQqKk";

  p->phase           = 0;
  p->castleFlags     = 0;
  p->reversibleMoves = 0;
  p->head            = 0;

  for (i = 0; i < 2; i++) {
    p->bbCl[i]      = 0ULL;
    p->pieceMat [i] = 0;
    p->pstMg[i]     = 0;
	p->pstEg[i]     = 0;

	for (j = 0; j < 6; j++) {
		p->pcCount[i][j] = 0; // clear piece counts
	}
  }

  for (i = 0; i < 6; i++)
    p->bbTp[i] = 0ULL;

  for (i = 56; i >= 0; i -= 8) {
    j = 0;
    while (j < 8) {
      if (*epd >= '1' && *epd <= '8')
        for (pc = 0; pc < *epd - '0'; pc++) {
          p->pc[i + j] = NO_PC;
          j++;
        }
      else {
        for (pc = 0; pc_char[pc] != *epd; pc++)
          ;
        p->pc[i + j] = pc;
        p->bbCl[Cl(pc)] ^= SqBb(i + j);
        p->bbTp[Tp(pc)] ^= SqBb(i + j);

        // set king location
		if (Tp(pc) == K) p->kingSquare[Cl(pc)] = i + j;

        // update material, game phase and pst values
		p->pieceMat[Cl(pc)]  += Data.matValue[Tp(pc)];
		p->phase             += Data.phaseValue[Tp(pc)];
        p->pstMg[Cl(pc)]     += Data.pstMg[Cl(pc)][Tp(pc)][i + j];
		p->pstEg[Cl(pc)]     += Data.pstEg[Cl(pc)][Tp(pc)][i + j];
		p->pcCount[Cl(pc)] [Tp(pc)]++;
        j++;
      }
      epd++;
    }
    epd++;
  }

  if (*epd++ == 'w') p->side = WHITE;
  else               p->side = BLACK;

  epd++;
  if (*epd == '-')
    epd++;
  else {
    if (*epd == 'K') {
      p->castleFlags |= 1;
      epd++;
    }
    if (*epd == 'Q') {
      p->castleFlags |= 2;
      epd++;
    }
    if (*epd == 'k') {
      p->castleFlags |= 4;
      epd++;
    }
    if (*epd == 'q') {
      p->castleFlags |= 8;
      epd++;
    }
  }
  epd++;
  if (*epd == '-')
    p->epSquare = NO_SQ;
  else {
    p->epSquare = Sq(*epd - 'a', *(epd + 1) - '1');
    if (!(bbPawnAttacks[Opp(p->side)][p->epSquare] & bbPc(p, p->side, P)))
      p->epSquare = NO_SQ;
  }
  p->hashKey = TransTable.InitHashKey(p);
  p->pawnKey = TransTable.InitPawnKey(p);
}