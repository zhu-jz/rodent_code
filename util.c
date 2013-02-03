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

#include <string.h>
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
#  include <windows.h>
#else
#  include <unistd.h>
#  include <sys/time.h>
#endif
#include "data.h"
#include "rodent.h"

int InputAvailable(void)
{
#if defined(_WIN32) || defined(_WIN64)
  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init) {
    init = 1;
    inh = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(inh, &dw);
    if (!pipe) {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
    }
  }
  if (pipe) {
    if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL))
      return 1;
    return dw > 0;
  } else {
    GetNumberOfConsoleInputEvents(inh, &dw);
    return dw > 1;
  }
#else
  fd_set readfds;
  struct timeval tv;

  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &readfds);
#endif
}

U64 Random64(void)
{
  static U64 next = 1;

  next = next * 1103515245 + 12345;
  return next;
}

void MoveToStr(int move, char *moveString)
{
  static const char prom_char[5] = "nbrq";

  moveString[0] = File(Fsq(move)) + 'a';
  moveString[1] = Rank(Fsq(move)) + '1';
  moveString[2] = File(Tsq(move)) + 'a';
  moveString[3] = Rank(Tsq(move)) + '1';
  moveString[4] = '\0';
  if (IsProm(move)) {
    moveString[4] = prom_char[(move >> 12) & 3];
    moveString[5] = '\0';
  }
}

void PrintMove(int move)
{
	 char moveString[6];
	 MoveToStr(move, moveString);
	 printf("%s", moveString);
}

int StrToMove(sPosition *p, char *moveString)
{
  int from, to, type;

  from = Sq(moveString[0] - 'a', moveString[1] - '1');
  to = Sq(moveString[2] - 'a', moveString[3] - '1');
  type = NORMAL;
  if (TpOnSq(p, from) == K && Abs(to - from) == 2)
    type = CASTLE;
  else if (TpOnSq(p, from) == P) {
    if (to == p->epSquare) 
      type = EP_CAP;
    else if (Abs(to - from) == 16)
      type = EP_SET;
    else if (moveString[4] != '\0')
      switch (moveString[4]) {
      case 'n':
        type = N_PROM;
        break;
      case 'b':
        type = B_PROM;
        break;
      case 'r':
        type = R_PROM;
        break;
      case 'q':
        type = Q_PROM;
        break;
      }
  }
  return (type << 12) | (to << 6) | from;
}

void PvToStr(int *pv, char *pv_str)
{
  int *movep;
  char moveString[6];

  pv_str[0] = '\0';

  for (movep = pv; *movep; movep++) {
    MoveToStr(*movep, moveString);
    strcat(pv_str, moveString);
    strcat(pv_str, " ");
  }
}

void BuildPv(int *dst, int *src, int move)
{
  *dst++ = move;
  while ((*dst++ = *src++))
    ;
}

U64 atoull(const char *s)
{
  U64 x = 0;
  char c;
  while (c = *s++) {
	if (c != ' ' && c != ',') {
       x *= 10;
       x += c - '0';
	}
  }
  return x;
} 