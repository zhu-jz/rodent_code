/*
  Rodent, a UCI chess playing engine derived from Sungorus 1.4
  Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
  Copyright (C) 2011-2014 Pawel Koziol

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

#pragma once

struct sGenCache {
private:
  U64 bbROcc[64];         // arrays for hashing generated bitboards
  U64 bbRMob[64];
  U64 bbBOcc[64];        
  U64 bbBMob[64];
public:
  U64 GetRookMob(U64 bbOccupied, int sq);
  U64 GetBishMob(U64 bbOccupied, int sq);
  U64 GetQueenMob(U64 bbOccupied, int sq);
};

extern sGenCache GenCache;
