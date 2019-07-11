#!/usr/bin/expect

# Use gnuchess to decide on next move.
#
# Always returns coordinate notation (from-square to-square, such as "e1g1")
# whether it is a move, a capture, or a castle (in which case, only the 
# king's move is given).  This is clear and unambiguous for the robot.
#
# 1. reads current position from file named $1
# 2. decides what move to make (as white), using gnuchess
# 3. prints out move

A=`cat $1`

spawn /usr/games/gnuchess --uci

send "position fen $A - \r"

expect -timeout 1 readyok

send "go depth 2 \r"

expect -timeout 2 readyok

send "quit \r"


