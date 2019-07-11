# chess-robot

Code for a chess-playing robot that uses:
  - Kinova Mico robot arm
  - ROS Kinetic robot operating system
  - DGT sensory chessboard
  - GNUchess engine
  - Very old telephone

To initialize robot arm:
  - cd catkin*; source devel/setup.bash; launch

To launch chess player:
  - cd catkin*; srouce devel/setup.bash; cd src/chess-robot; ./dgt-chess-play

Top-level control

- dgt.chess.play:  Top-level controller for chess player.

Robot target positions

- dgt.chess.arrays:  Robot arm positions for grasping pieces and hovering over them.

- make-arrays:  Help populate arm position arrays.

Reading board

- dgt.chess.getboard:  Retrieve current position from board, converts to .epd format.

- getboard/kk4.o:  Directory getboard has C program (kk4.c) to read board.

Deciding move

- dgt.chess.decide:  Take board position and use GNUchess to return best move, in algebraic notation.

- dgt.chess.exe:  Transient file that interactively calls GNUchess.

- dgt.chess.startpos: File that stores start position, for checking.

- dgt.chess.pos*: Transient files that records position.

- dgt.chess.nextpos:  Transient file that records position.

- dgt.chess.phone, phone.wav:  Cause phone to ring, and direct robot to pick it up.

Moving pieces

- dgt.chess.makemove:  Execute move given in algebraic notation.

- moveto:  Move robot arm to destination (given by joint positions).

- positions.names:  Helps convert board position (like e5) into number (1-64).

- dgt.chess.init:  Move robot into neutral position.

Not used

- dgt.test: Not used.

- getrobotpos:  Not used.

- dgt.chess.gnu: Not used. 

- moveto1, moveto.134.2, moveto2, moveto3:  Not used.

- output.epd:  Not used.

- ttt: precursor of the chess robot that plays tic-tac-toe.

Issues

- After a couple of days of no-use, robot can't move arm (but can move fingers).

- Want to port to Raspberry Pi.


===================================================
================= OLD NOTES =======================
===================================================

Chess engines I looked at:

1. gnuchess
2. fruit
3. hoichess
4. phalanx
5. stockfish
6. fairychess
7. toga (?)
8. glarung (?)
9. pychess

=============================================================
How do programs today currently interact with chess engines?

A good example is xboard, an X-Windows graphical user interface. 

gnuchess and xboard run as separate linux processes that talk to each other.

Things that xboard communicates to gnuchess:
  https://www.gnu.org/software/xboard/engine-intf.html

Things that gnuchess communicates to xboard:
  https://www.gnu.org/software/xboard/engine-intf.html#9

Need to understand & duplicate how these servers communicate.

=============================================================
How is a chess position represented formally?

As a string.  There are different standards.  One is EPD.  For example,
right after first move e2e4, the position is:

rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3

Notice there are four parts to this, separated by spaces:

  Part 1: Where pieces are on the board
  Part 2: Whose move it is
  Part 3: Whether certain pieces have moved prior in the game (long history)
  Part 4: Whether square that was just "skipped" by a pawn (short history)

Parts 3 & 4 tell whether it's legal to castle, or whether it's legal
to capture a pawn "en passant".

So to check whether a move is legal, it's not sufficient to simply look 
at the board!  You need to remember the history of the game.

A chess engine like gnuchess will keep track of where you are in the game, 
whereas an interface like xboard will not, since it merely needs to show 
where the pieces are.

=============================================================
How should the main loop program communicate with the robot arm?

What is the robot arm's Linux API?

What operations are supported?

=============================================================
How should the robot arm execute a move?

1. Positioning the arm: 
      How to arrive at a given <x,y,z> position of piece to move?
2. Does z need to depend on the height of the piece being moved?
3. Gripping: Does gripping tolerate noise?
4. Moving: Raise piece to above-king height, move to <x,y,z> descend, drop.
5. Advanced moving: Only raise to height necessary for clearing any 
     intervening pieces between source square and destination square.
6. Capturing: 
     a) Remove captured human piece.  Captured pieces should be 
        dropped into a bin (one bin per player).
     b) Move own robot piece.
7. Castling: 
     a) Move king.
     b) Move rook.
8. If robot has checkmated human: 
     a) Make move.
     b) Make victory gesture.
9. If robot has checked human:
     a) Make move.
     b) Gesture towards human's king.
10. No matter what, always return robot arm to initial position.

=============================================================
How should the robot understand when the human's move is complete?

1. Position should change, then stay unchanged for 3 seconds.
2. Exception: One human piece is "in the air" and all other pieces
   are unchanged.  In this case, keep waiting.

=============================================================
How should the human's move be interpreted and responded to?

1. Legal move (non-checkmate).
   Interpretation: Normal move.
   Action: Robot should move next.

2. Legal move that checkmates robot.
   Interpretation: Human won!
   Action: Robot should wait 3 seconds, then knock over own king
           and return robot arm to start position.

3. Illegal move resulting in check of human's king.
   Interpretation: Human either failed to escape check, or moved a 
                   piece that resulted in "revealed" check.
   Action: Robot make "no, no" gesture.
           Return board to previous state by moving human piece.
           (If there was a capture, we can't fix it!)
           Gesture at human's king.
           Wait for human to move again.

4. Illegal move due to other cause.
   Interpretation: Human made some kind of mistake.
   Action: Robot make "no, no" gesture.
           Return board to previous state by moving human piece.
           (If there was a capture, we can't fix it!)
           Wait for human to move again.

5. Position changes radically to legal start position.
   Interpretation: Human wants to start over.
   Action: If robot has white pieces, then robot should move.
           If robot has black pieces, then robot should wait.

6. Position changes to legal start position, except K and Q reversed.
   Interpretation: Human wants to start over, but set it up wrong.
   Action: Swap relevant king(s) and queen(s).
           If robot has white pieces, then robot should move.
           If robot has black pieces, then robot should wait.

7. Radical shift of the board to a new, non-starting position.
   Action: Wait for human to move.  
           This assigns color to human.
           Then robot should take the other side, and move.
      OR
   Action: Guess robot's color based on positions of pieces.
           Robot should move.

8. Other?  Treat like 7.

=============================================================
How should robot move in order to knock over a king, upon game's end?

1. Need robot motion for this.

=============================================================

=============================================================

=============================================================

=============================================================

=============================================================

=============================================================


% rosversion -d
kinetic


