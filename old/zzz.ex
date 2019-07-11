#!/usr/bin/expect
spawn /usr/games/gnuchess --uci
send "position fen 8/5Q1K/8/8/1b6/kp6/1p6/8 w KQkq - \r"
send "go depth 2 \r"
expect bestmove
send "quit \r"
