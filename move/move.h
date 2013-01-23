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

#define Fsq(x)          ((x) & 63)          // "from" square of a move "x"
#define Tsq(x)          (((x) >> 6) & 63)   // "to" square of a move "x"
#define MoveType(x)     ((x) >> 12)         // type of a move "x" (see eMoveType)
#define IsProm(x)       ((x) & 0x4000)      // is this move a promotion?
#define PromType(x)     (((x) >> 12) - 3)   // kind of promoted piece