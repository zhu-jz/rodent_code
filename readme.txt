Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2012 Pawel Koziol
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
along with this program.  If not, see <http://www.gnu.org/licenses/>.

II. Acknowledgments

Rodent is an open source chess program derived from Sungorus 1.4 
by Pablo Vazquez. Seeing the simplicity of his code I instantly
fell in love with it, started adding chess knowledge and advanced
search algorithms. Hopefully code quality didn't deteriorate too
much because of that attempt at learning by emulation.

The second most important source is excellent document called
"Toga LOG user manual" by josephd, which is accessible from
http://members.aon.at/josefd/Toga%20LOG.html . It describes 
evaluation function of Fruit/Toga in a natural language. First
draft of Rodent evaluation function relied heavily on this
document. Currently at least king safety code is different, 
but for example weak pawn evaluation remains basically the same.

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

*Dann Corbit* and *Denis Mendoza* supplied me with 64-bit compiles.

III. Project goals:

1. Creating a didactic bitboard engine of reasonable strength, 
   using object-oriented design, modern search techniques and
   relatively small codebase. There are many superb open source
   engines, like Fruit, Crafty, Stockfish, to name but a few,
   and Rodent is unlikely to compete with them on equal footing
   for some time to come. One thing it can give, however, is 
   a search function that can be read at one sitting (thank You,
   Pablo Vazquez, it would not be possible without Your work!)
   
2. Providing a simple framework for testing new ideas and showcasing
   them (I'm planning a short paper on a "sliding LMR" concept) 
   
3. Providing an engine that is fun to play against, with adjustable
   strength and different personalities.
   
      
IV. The most important changes (comparing with Sungorus 1.4) include:

- making most of the code object-oriented (in progress)
- enabling fractional extensions and reductions
- rewriting piece/square code, so that we can use asymmetric pst tables 
  and interpolate between midgame and endgame scores
- adding futility pruning
- adding "sliding LMR" (see section on search)
- adding eval pruning (inspired by DiscoCheck)  
- adding many evaluation features and weights as described 
  in Toga LOG user manual  
- adding logarithmic king safety function  
- some speed optimizations, including pawn hash table and specialized
  PopCnt functions (many thanks to Dann Corbit for his patch making use
  of advanced 64-bit instructions)
- about 300 Elo gain over the original
- opening book in a silly proprietary format
- position learning
- weaker levels of play

V. SEARCH (featuring "sliding LMR" proof of concept)

- fail soft alpha beta with principal variation search (from Sungorus)
- transposition table (from Sungorus)
- null move with variable reduction depth (modified)
  and with threat detection in order to sort evasion higher (modified)
- futility pruning (added)
- "sliding" late move reduction (added)
- late move pruning (added)
- eval pruning (a.k.a. static null move)
- internal iterative deepening in pv nodes

Rodent uses PVS search with null move and transposition table inherited 
from Sungorus. It has been enhanced in several standard ways - by adding 
futility pruning, internal iterative deepening in pv nodes, check extension,
Stockfish-like null move pruning and using capture threats uncovered by 
null move for better move sorting.

The only non-standard solution is what I call "sliding Late Move Reduction".
Since Rodent from the very beginning uses fractional plies, it made sense
to check whether smoothly scaling reduction can help. And it certainly did.

It works as follows. Instead of a fixed one ply reduction, Rodent uses
something like:

QUARTER_PLY * ( (moves_tried / 5)+3 )

where moves_tried counts moves accepted by search as legal. Thus the first 
reduction occurs much later under normal scheme, but as the search proceeds, 
the effect of accumulated quarter ply reductions more than compensates for 
it, without hurting the playing strength. This kind of reduction has a chance 
of being much less detrimental to the tactical ability of the engine. After 
all, it is geared towards reducing more deeply when the engine encounters 
a series of moves that have low probability of changing the node value 
(move ordering, especially using history heuristic, is a measure of 
that probability).

Imagine a tactical shot requiring two quiet moves, the rest being
forcing and therefore exempt from reduction. Both are sorted around
move 20. Under normal scheme both would get reduced, the new one 
reduces just one. Instead, it trims the lines where many moves are 
sorted down the list. It's true that finding really deep combintations, 
full of quiet moves, is equally difficult under both schemes. However,
probability of such a combination is  

P(one_move_cuts) * P(next_move_cuts) ... * P(last_required_move_cuts) 

IIRC (I've never been much into maths, much less into the English math 
terminology) the right word for this kind of function is "exponential". 
If we manage to keep the growth of a reduction factor just a bit slower, 
we'll have a beautifully scalable reduction algorithm.

VI. EVALUATION

- Fruit-like piece/square tables and mobility values
- Fruit-like weak pawns eval
- passed pawns eval
- strong squares (B, N, even R)
- logaritmic, and therefore dynamic King safety evaluation

Creating version 0.13 I looked for more aggressive King safety function,
using Rebel-like additive table approach. I also wanted my function to 
ne initialized at startup using some relatively simple mathematical formula.
After some experiments with logistic function, I settled for an addition
of logarithmic and linear function. I'd like to express my gratitude
to Agnieszka Ziomek for discussing these mathematical matters with a noob
like me.

Mobility evaluation is sometimes inexact due to the concept of "transparent 
pieces". For example, own queen is considered transparent for the bishop. 
This might cause occasional error in evaluating whether a bishop can check 
enemy king, or force the engine to rely on search in order to avoid certain
blockages. However, tests has shown that this solution is better than more
exact mobility calculation.

VII. OPENING BOOK

Since version 0.14 Rodent uses its own opening book, encoded in a rather
inefficient manner. At startup, it reads two files: book.txt written in
a human readable format, and bigbook.wtf using Rodent's own ugly format
(hence the file extension). 

Rodent first looks for moves from the book.txt, and if none is found, it 
falls back to big main book.

book.txt encodes one opening in each line, and its format looks like that:

e2e4 a7a6? d2d4 b7b5 a2a4 c8b7 b1d2
e2e4 d7d5!! e4d5 d8d5 b1c3 d8d6!

It allows users to write down their own book preferences if they for example 
want to practice Ruy Lopez or Pirc Defense or force Rodent to play unusual 
openings. Moves may be followed by punctuation marks: ? (frequency -= 100),
! (frequency += 100), ?? (frequency -= 5000), !! (frequency += 5000). 
Additionally, a move with "xx" appended will be deleted from the main book
permanently.

A really desperate user might even create main opening book. All he needs
is any of files called feed.txt, feedwhite.txt and feedblack.txt placed 
in the program directory, Rodent opened in the console mode and manually 
typed command "feedbook". Program first reads bigbook.wtf (if found), then 
supplements it with the lines froom feed files, then chokes itself on shaker 
sort algorithm while displaying number of its iterations, and finally saves 
a file called newbook.wtf. At this stage user MUST RENAME newbook.wtf 
to bigbook.wtf. This is a precaution allowing to save old book in case that
user considers feed files corrupt.

Rodent contains small utility reformatting book files from something like:

e2e4e7e6d2d4d7d5b1c3f8b4

to version with spaces between the moves. In order to do this, one needs
to copy "dense" opening book into the file "dense.txt", open Rodent in the
console mode and type command "split". Result will be saved in "loose.txt".

Another additional command is "bookdoctor". This command causes Rodent to
investigate its main book file and stop at first point where it finds
either an infrequent move or missing transposition into book line. In both
cases it would be prudent to decide whether a move should be upgraded
or marked as unplayable.

Rodent may use big book in two modes: it may pick moves according to their 
frequency or conduct short verification search. The latter causes it to play
moves that it likes and to reject errors that are likely to be found in the 
big book. The second mode can be turned on using UCI option "VerifyBook"

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
by a pawn or (re)captures of recently moved piece) would look unnatural, except
of the lowest levels. Engine forgetting about escapes from check would happily
sack material for false checkmates. Engine not missing tactical moves would not
be weak enough.

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

- an asterisk before a comment announcing a code segment
  means that this part of code is undergoing modification 
  and testing
- variable names start with small letters
- function names start with captital letters
- variables treated as flags have "f" prefix, i.e. f_in_check
- bitboard variables have "bb" prefix, i.e. bbControl 
  in piece evaluators
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
- we DO NOT split Search function into pv and non-pv, 
  since resulting complexity isn't worth small speed gain.