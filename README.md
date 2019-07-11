# chess-robot

Code for a chess-playing robot that uses:

- Kinova Mico robot arm
- ROS Kinetic robot operating system
- DGT sensory chessboard
- GNUchess engine
- Very old telephone

To initialize robot arm:

- cd catkin*; source devel/setup.bash; launch

- (need ROS code for launch)

To launch chess player:

- cd catkin*; source devel/setup.bash; cd src/chess-robot; ./dgt-chess-play

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

- dgt.chess.exe:  Transient file used to interactively call GNUchess.

- dgt.chess.startpos: File that stores start position, for checking.  Robot has randomization for its first move, for variety's sake.

- dgt.chess.pos*: Transient files that records position.

- dgt.chess.nextpos:  Transient file that records position.

Moving pieces

- dgt.chess.makemove:  Execute move given in algebraic notation.

- moveto:  Move robot arm to destination (given by joint positions).  Uses ROS commands.

- positions.names:  Helps convert board position (like e5) into number (1-64).

- dgt.chess.init:  Move robot into neutral position.

Phone

- dgt.chess.phone:  Cause phone to ring, and direct robot to pick it up.

- phone.wav:  Sound of phone ringing.

Not used

- dgt.test, getrobotpos, dgt.chess.gnu, moveto1, moveto.134.2, moveto2, moveto3, output.epd

- ttt: precursor of the chess robot that plays tic-tac-toe.

Issues

- After a couple of days of no-use, robot can't move arm anymore (but can move fingers).  Very annoying -- requires power-cycling, rebooting laptop, re-initialization.

- Raspberry Pi port -- laptop is too bulky, and almost no computation is required.

- Robot fingers are slightly askew -- need to straighten and re-assemble.

- Robot sometimes fails to pick up king correctly.

- Can't simply "push" pawns forward.  Elaborately raises them up.

