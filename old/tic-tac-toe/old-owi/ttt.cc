
/********************************************************************* 
 ** Program Filename: tictactoe.cpp
 ** Author: Spike Madden
 ** Date: 03.16.2014
 ** Description: Tic tac toe (1 player and 2 player)
 *********************************************************************/ 

#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

struct tic_tac_toe {
	char board[3][3];
	char player1;
	char player2;
	char cpu;
};

/********************************************************************* 
 ** Function: initalize_board
 ** Description: Fills the 9 spots on the tic-tac-toe board with periods.
 ** Parameters: char[3][3]
 ** Pre-Conditions: Board was created in main.
 ** Post-Conditions: Board is initalized with periods.
 *********************************************************************/ 
void initalize_board (char board[3][3]) {

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			board[i][j] = '.';
		}
	}
}

/********************************************************************* 
 ** Function: print_board
 ** Description: Prints out the tic-tac-toe board.
 ** Parameters: char[3][3]
 ** Pre-Conditions: None.
 ** Post-Conditions: None.
 *********************************************************************/ 
void print_board(char board[3][3]) {

	for (int i = 0; i < 3; i++) {
		cout << endl;
		for (int j = 0; j < 3; j++) {
			cout << "  ";
			cout << board[i][j];
		}
	}
	cout << endl << endl;
}

/********************************************************************* 
 ** Function: check_win
 ** Description: Checks for three in a row in all directions.
 ** Parameters: char[3][3]
 ** Pre-Conditions: None.
 ** Post-Conditions: Returns the character that that the three in a row consists of.
 *********************************************************************/ 
char check_win(char board[3][3]) {
	
	// Check horizontal, vertical & diagonal through [0][0]
	if (board[0][0] != '.' && (board[0][0] == board[0][1] && board[0][0] == board[0][2] ||
		board[0][0] == board[1][0] && board[0][0] == board[2][0] ||
		board[0][0] == board[1][1] && board[0][0] == board[2][2])) 
	
		return board[0][0];
	
	// Check horizontal, vertical & diagonal through [1][1]
	if (board[1][1] != '.' && (board[1][1] == board[1][0] && board[1][1] == board[1][2] ||
		board[1][1] == board[0][1] && board[1][1] == board[2][1] ||
		board[1][1] == board[2][0] && board[1][1] == board[0][2])) 
	
		return board[1][1];
	
	// Check horizontal, vertical & diagonal through [2][2]
	if (board[2][2] != '.' && (board[2][2] == board[0][2] && board[2][2] == board[1][2] ||
		board[2][2] == board[2][0] && board[2][2] == board[2][1]))
	
		return board[2][2];
	
	return 0;
}

int negamax(char board[3][3], char player1, char player2);
/********************************************************************* 
 ** Function: pick_best_move
 ** Description: Finds the best possible move given the current board.
 ** Parameters: char[3][3], char, char
 ** Pre-Conditions: None.
 ** Post-Conditions: None.
 *********************************************************************/ 
int pick_best_move(char board[3][3], char player1, char player2) {

	int best_move_score = -9999;
	int best_move_row = -9999;
	int best_move_col = -9999;
	int score_for_this_move = 0;

	for (int r = 0; r < 3; r++) {
		for (int c = 0; c < 3; c++) {
			if (board[r][c] == '.') {
				board[r][c] = player1; //Try test move.
				score_for_this_move = -(negamax(board, player2, player1));
				board[r][c] = '.'; //Put back test move.
				if (score_for_this_move >= best_move_score) {
					best_move_score = score_for_this_move;
					best_move_row = r;
					best_move_col = c;
				} 
			}
		}
	}
	return (10*best_move_row + best_move_col);
}

/********************************************************************* 
 ** Function: negamax
 ** Description: Recursively calls itself to play out every possible
 ** position.
 ** Parameters: char[3][3], char, char
 ** Pre-Conditions: pick_best_move has been called.
 ** Post-Conditions: Returns best_score_move to pick_best_move
 *********************************************************************/ 
int negamax(char board[3][3], char player1, char player2) {

	int best_move_score = -9999;
	int score_for_this_move = 0;
	
	//If player 1 wins, then the score is high (good for player1)
	if (check_win(board) == player1)
		return 1000;
	
	//If player 2 loses, then the score is low (bad for player1)
	else if (check_win(board) == player2)
		return -1000;

	for (int r = 0; r < 3; r++) {
		for (int c = 0; c < 3; c++) {
			if (board[r][c] == '.') {
				board[r][c] = player1; //Try test move.
				score_for_this_move = -(negamax(board, player2, player1));
				board[r][c] = '.'; //Put back test move.
				if (score_for_this_move >= best_move_score) {
					best_move_score = score_for_this_move;
				}
			}
		}
	}
	
	if (best_move_score == -9999 || best_move_score == 0)
		return 0;
	
	else if (best_move_score < 0)
		return best_move_score + 1;

	else if (best_move_score > 0)
		return best_move_score - 1; //As the game goes longer, and the recursion goes deeper, the moves near the end are less favorable than in the beginning.
	
}

/********************************************************************* 
 ** Function: player_palcement
 ** Description: Asks the user for a row and a column and places their
 ** character in that spot.
 ** Parameters: char[3][3], char
 ** Pre-Conditions: Board has been initialized.
 ** Post-Conditions: Player's character will be placed on the board.
 *********************************************************************/ 
void player_placement(char board[3][3], char player) {
	
	while (1) {
		string string_row, string_col;
		int row = 0, col = 0;
		while (1) {
			cout << "Where would you like to play? " << endl << "Enter the row: ";
			cin >> string_row;
			row = atoi(string_row.c_str());

			if (row >= 1 && row <= 3)
				break;
			cout << "You need to enter a row on the board (between 1 and 3, inclusive)." << endl;
		}

		while (1) {
			cout << "Enter the column: ";
			cin >> string_col;
			col = atoi(string_col.c_str());

			if (col >= 1 && col <= 3)
				break;
			cout << "You need to enter a column on the board (between 1 and 3, inclusive)." << endl;	
		}
		if (board[row-1][col-1] == '.') {
			board[row-1][col-1] = player;
			break;
		}
		else
			cout << "Someone already played there." << endl << endl;
	}
}

/********************************************************************* 
 ** Function: determine_cpu_choice
 ** Description: Decides whether the computer is playing as x's or o's.
 ** Parameters: char
 ** Pre-Conditions: Player 1 has selected a character.
 ** Post-Conditions: The computer's character will be selected.
 *********************************************************************/ 
char determine_cpu_choice (char player1) {

	char cpu_char;
	cout << "Hello, I am the computer you will be playing." << endl;
	cout << "If you chose x's, I will be o's. If you chose o's, I will be x's." << endl;
	cout << "If you chose neither x or o, I will default to x." << endl << endl;

	if (player1 == 'x' || player1 == 'X') 
		cpu_char = 'o';

	else 
		cpu_char = 'x';

	return cpu_char;
}

/********************************************************************* 
 ** Function: determine_player_choice
 ** Description: Asks the player to select a character.
 ** Parameters: string
 ** Pre-Conditions: Variable in main to store user's choice.
 ** Post-Conditions: Player's character will be selected.
 *********************************************************************/ 
char determine_player_choice (string s) {	
	
	while (1) {
		string choice;
		cout << s << ": What would you like your character to be? ";
		cin >> choice;
		if (choice.size() > 1) {
			cout << "You inputted more than one character. Please try again." << endl;
			continue;
		}
		cout << endl;
		return choice[0];
	}
}

/********************************************************************* 
 ** Function: play_game_1_player
 ** Description: Plays a game of tic-tac-toe with 1 player.
 ** Parameters: char[3][3], char, char
 ** Pre-Conditions: User has indicated that he/she wants to play against an AI.
 ** Post-Conditions: Game is played.
 *********************************************************************/ 
void play_game_1_player (char board[3][3], char player1, char cpu) {
	
	int moves = 0;
	while (moves < 9) {
		player_placement(board, player1);
		moves++;
		print_board(board);
		if (check_win(board)) {
			cout << player1 << " won!" <<endl;
			exit(1);
		}
		if (moves == 9)
			break;
		int where_to_move = pick_best_move(board, cpu, player1);
		int row = where_to_move / 10;
		int col = where_to_move % 10;
		board[row][col] = cpu;
		moves++;
		print_board(board);
		if (check_win(board)) {
			cout << cpu << " won!" <<endl;
			cout << "If I were you, I would've played at: " << "(row " << where_to_move/10 + 1 << ", col " << where_to_move % 10 + 1 << ")" << endl;
			exit(2);
		}
	}
	cout << "Cat's game!" << endl;
}

/********************************************************************* 
 ** Function: play_game_2_player
 ** Description: Plays a game of tic-tac-toe with 2 players.
 ** Parameters: char[3][3], char, char
 ** Pre-Conditions: User has indicated that he/she wants to play with 2 people.
 ** Post-Conditions: Game is played.
 *********************************************************************/ 
void play_game_2_player (char board[3][3], char player1, char player2) {
	
	for (int move_num = 0; move_num < 9; ++move_num) {
		player_placement(board,(move_num & 1) ? player1 : player2);
		print_board(board);
		char winner = check_win(board);
		if (winner) {
			cout << winner << " won!" << endl;
			exit(0);
		}
	}
	cout << "Cat's game!" << endl;
}

/********************************************************************* 
 ** Function: question_two_player
 ** Description: Asks the user whether they want to play 1 player or 2 player.
 ** Parameters: None.
 ** Pre-Conditions: None.
 ** Post-Conditions: Game is played out as 1 player or 2 player.
 *********************************************************************/ 
bool question_two_player() {
	
	while (1) {
		while (1) {
			string choice;
			int players = 0;
			cout << "Would you like to play with 2 players or against a computer?" << endl;
			cout << "Enter 1 for one player or 2 for two players: ";
			cin >> choice;
			cout << endl;
			players = atoi(choice.c_str());
			if (players != 1 && players != 2) {
				cout << "Please enter either a 1 or a 2." << endl << endl;
			}
			else if (players == 1)
				return false;
			else if (players == 2)
				return true;
			break;
		}
	}
}

int main() {
	
	tic_tac_toe game;
	initalize_board(game.board);

	bool two_player = question_two_player();

	if (two_player == false) {
		game.player1 = determine_player_choice("Player 1");
		game.cpu = determine_cpu_choice(game.player1);
		print_board(game.board);
		play_game_1_player(game.board, game.player1, game.cpu);
	}
	
	else if (two_player == true) {
		game.player1 = determine_player_choice("Player 1");

		while (1) {
			game.player2 = determine_player_choice("Player 2");
			if (game.player2 != game.player1)
				break;
			else
				cout << "You entered the same character as Player 1. Please select a different character." << endl;
		}
		print_board(game.board);
		play_game_2_player(game.board, game.player1, game.player2);
	}
	return 0;
}
