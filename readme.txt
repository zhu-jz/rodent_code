Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2013 Pawel Koziol
http://www.pkoziol.cal24.pl/rodent/rodent.htm
https://github.com/nescitus/rodent_code

I. GPL License

Rodent is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published 
by the Free Software Foundation, either version 3 of the License, 
or (at your option) any later version.

Rodent is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty 
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

II. Acknowledgments

Rodent is an open source chess program derived from Sungorus 1.4 
by Pablo Vazquez. Seeing the simplicity of his code I instantly
fell in love with it, started adding chess knowledge and advanced
search algorithms. Hopefully code quality didn't deteriorate too
much because of that attempt at learning by emulation.

The second most important source is "Toga LOG user manual" by josephd, 
(http://members.aon.at/josefd/Toga%20LOG.html). It describes 
evaluation function of Fruit/Toga in a natural language. First
draft of Rodent's evaluation function relied heavily on this
document, and many evaluation weights are derived directly from it.

Open source programming makes most sense as a collaborative effort.
Rodent would be clearly weaker without the following contributions:

*Kestutis Gasaitis* has made a thorough inspection of Rodent's code,
pointing out many bugs and suggesting several improvements. See
changelog and github tickets for details.

*Dann Corbit* supplied fast population count functions 
making use of advanced 64-bit instructions and first bit intrinsics
(used since version 0.11 onwards)

*Denis Mendoza* and *Graham Banks* independently pointed out 
an important time management bug in version 0.12, 
affecting repeated time controls.

*Jim Ablett*, *Dann Corbit* and *Denis Mendoza* supplied 64-bit compiles.

III. Project goals:

1. Creating a didactic bitboard engine of reasonable strength, 
   using object-oriented design, modern search techniques and
   relatively small codebase. There are many superb open source
   engines, like Fruit, Crafty, Stockfish, to name but a few,
   and Rodent is unlikely to compete with them on equal footing
   for some time to come. One thing it can give, however, is 
   a search function that can be read at one sitting (thank You,
   Pablo Vazquez, it would not be possible without Your work!)
   
2. Providing a simple framework for testing and showcasing new ideas
   
3. Providing an engine that is fun to play against, with adjustable
   strength and different personalities.
   
IV. The most important changes (comparing with Sungorus 1.4) include:

- making most of the code object-oriented
- enabling fractional extensions and reductions
- rewriting piece/square code, so that we can use asymmetric pst tables 
  and interpolate between midgame and endgame scores
- some speed optimizations, including pawn hash table and specialized
  PopCnt functions (thanks to Dann Corbit)
- Polyglot opening book
- position learning

V. SEARCH

- fail soft alpha beta with principal variation search (from Sungorus)
- transposition table (from Sungorus)
- null move with variable reduction depth (modified)
  and with threat detection in order to sort evasion higher (modified)
- futility pruning (added)
- late move reduction from Stockfish / Toga II 3.0 (added)
- late move pruning (added)
- eval pruning (a.k.a. static null move, inspired by DiscoCheck)
- internal iterative deepening in pv nodes
- move sorting using null move threat, refutation and continuation moves

VI. EVALUATION

- Fruit-like piece/square tables
- mobility (excluding squares attacked by pawns in case of minor pieces)
- Fruit-like weak pawns eval
- passed pawns eval
- outposts (B, N, even R), further bonus if defended by a pawn
- two different King safety functions, including attacks on king zone,
  check threats and attacking material
- hanging pieces
- minor pieces attacked/defended by a pawn (overlaps with hanging pieces)
- bad bishops (using bitmask of obstructing pawns)
- (half)open files for rooks with additional bonus if they target enemy king
- rook on 7th rank cutting off enemy king or attacking pawns

Mobility and check threat evaluation is sometimes inexact due to the concept 
of "transparent pieces". For example, own queen is considered transparent 
for the bishop. This might cause occasional error in evaluating whether a bishop 
can check enemy king, or force the engine to rely on search in order to avoid 
certain blockages. However, this solution passed the tests.

VII. OPENING BOOK

Rodent uses two opening books: small guide books supplied in text format
and main .bin books (polyglot format). It first looks for moves from 
the guide book, and if none is found, it falls back to big main book.

Guide books encode one opening in each line, and its format looks like that:

e2e4 a7a6? d2d4 b7b5 a2a4 c8b7 b1d2
e2e4 d7d5!! e4d5 d8d5 b1c3 d8d6!

It allows users to express their preferences. If they for example want
to practice Ruy Lopez or Pirc Defense or force Rodent to play unusual lines.
Moves may be followed by punctuation marks: ? (frequency -= 100)
! (frequency += 100), ?? (frequency -= 5000), !! (frequency += 5000). 

Because I'm not confident about my book making skills, move picking algorithm 
tries to replace manual tuning by discarding moves with weight lesser than 10% 
of best move's weight.

For now Rodent 1.3 reads only a book file called rodent.bin placed directly 
in its folder, so if You want to use different, better book, You must rename it.

VIII. WEAKENING

Rodent has a feature allowing to reduce its playing strength. For now, it is
still in the experimental phase and still undergoes tweaking and tuning, but 
basic infrastructure is already in place. It consists of three parts:

1. Weaker levels play slower. On starting a new iteration or changing a move
   Rodent "freezes" for a moment, depending on its Elo command.
2. Weaker levels add random value to evaluation score.
3. Weaker levels are more likely to blunder, because they forget 
   about calculating consequences of certain moves.

As You can probably guess, tuning the third element of the algorithm presents
the hardest challenge. Engine forgetting about certain moves (like captures
by a pawn or (re)captures of recently moved piece) would look unnatural. 
Engine forgetting about escapes from check would happily sack material for 
false checkmates. Engine not missing tactical moves would not be weak enough.

IX. INI FILE

rodent.ini file contains the following information

- panel type, determined by string "user nomal" or "user power"
- list of available strength levels
- list of available playing styles
- list of available opening books

Normal user panel lists available levels, strength and personalities, registered
in the ini file. Power user obtains the ability to manually set internal engine 
parameters via UCI options.

X. CONVENTIONS AND STRUCTURAL DECISIONS

Naming conventions:

- variable names start with small letters
- function names start with captital letters
- variables treated as flags have "f" prefix, i.e. f_in_check
- bitboard variables have "bb" prefix, i.e. bbControl 
- struct definitions have "s" prefix,  i.e. sEvaluator

Classes:

- sParser       - UCI parser  
- sTransTable   - transposition table (no functional change vs Sungorus)
- sHistory      - global history and killer tables
- sGenCache     - caching generated bitboards for minimal speedup
- sTimer        - setting and enforcing time controls
- sSelector     - picks moves in an ordered fashion
- sSearcher     - implements classical alpha-beta pvs search algorithm
- sEvaluator    - implements valuation function
- sData         - globally accessible configurable data; in the target version, 
                  if someone wants to concentrate on just one facet 
                  of a program, he will have to know just sData 
                  and the relevant class.

Structural decisions:

- we use the same square ordering as Crafty and Stockfish (A1 = 0)
- we DO NOT split Search function into pv and non-pv, to keep it simple.
