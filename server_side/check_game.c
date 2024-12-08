#include <arpa/inet.h>
#include <locale.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include "check_game.h"
#include "board.h"

#define white_king   0x2654 // ♔
#define white_queen  0x2655 // ♕
#define white_rook   0x2656 // ♖
#define white_bishop 0x2657 // ♗
#define white_knight 0x2658 // ♘
#define white_pawn   0x2659 // ♙
#define black_king   0x265A // ♚
#define black_queen  0x265B // ♛
#define black_rook   0x265C // ♜
#define black_bishop 0x265D // ♝
#define black_knight 0x265E // ♞
#define black_pawn   0x265F // ♟

bool is_diagonal(int, int);
// Function declarations
bool general_errors(wchar_t **board, int player, int team, int *move, int piece_team);
bool validate_piece_move(wchar_t **board, int player, int piece_team, int x_moves, int y_moves, int *move);
bool validate_king_move(int player, int x_moves, int y_moves);
bool validate_queen_move(wchar_t **board, int player, int x_moves, int y_moves, int *move);
bool validate_rook_move(wchar_t **board, int player, int x_moves, int y_moves, int *move);
bool validate_bishop_move(wchar_t **board, int player, int x_moves, int y_moves, int *move);
bool validate_knight_move(int player, int x_moves, int y_moves);
bool validate_pawn_move(wchar_t **board, int player, int piece_team, int x_moves, int y_moves, int *move);
bool validate_pawn_capture(wchar_t **board, int player, int piece_team, int *move);
bool validate_pawn_advance(wchar_t **board, int player, int piece_team, int x_moves, int *move);

void move_piece(wchar_t **board, int *move) {
    // Move piece in board from origin to dest
    board[move[2]][move[3]] = board[*move][move[1]];
    board[*move][move[1]] = 0;
}

void freeAll(int *piece_team, int *x_moves, int *y_moves) {
    free(piece_team);
    free(x_moves);
    free(y_moves);
}

bool emit(int client, char *message, int message_size) {
    return true;
}

void translate_to_move(int *move, char *buffer) {
    printf("buffer: %s\n", buffer);
    *(move) = 8 - (*(buffer + 1) - '0');
    move[1] = (*(buffer) - '0') - 49;
    move[2] = 8 - (*(buffer + 4) - '0');
    move[3] = (*(buffer + 3) - '0') - 49;
}

void broadcast(wchar_t **board, char *one_dimension_board, int player_one, int player_two) {
    to_one_dimension_char(board, one_dimension_board);
    printf("\tSending board to %d and %d size(%lu)\n", player_one, player_two, sizeof(one_dimension_board));
    send(player_one, one_dimension_board, 64, 0);
    send(player_two, one_dimension_board, 64, 0);
    printf("\tSent board...\n");
}

int getManitud(int origin, int dest) {
    return (abs(origin - dest));
}

bool is_diagonal_clear(wchar_t **board, int *move) {
    int *x_moves = (int *)malloc(sizeof(int));
    int *y_moves = (int *)malloc(sizeof(int));

    *(x_moves) = *(move)-move[2];
    *(y_moves) = move[1] - move[3];

    int *index = (int *)malloc(sizeof(int));
    *(index) = abs(*x_moves) - 1;
    wchar_t *item_at_position = (wchar_t *)malloc(sizeof(wchar_t));

    // Iterate thru all items excepting initial posi
    while (*index > 0) {
        if (*x_moves > 0 && *y_moves > 0) {
            printf("%lc [%d,%d]\n", board[*move - *index][move[1] - *index], *move - *index, move[1] - *index);
            *item_at_position = board[*move - *index][move[1] - *index];
        }
        if (*x_moves > 0 && *y_moves < 0) {
            printf("%lc [%d,%d]\n", board[*move - *index][move[1] + *index], *move - *index, move[1] + *index);
            *item_at_position = board[*move - *index][move[1] + *index];
        }
        if (*x_moves < 0 && *y_moves < 0) {
            printf("%lc [%d,%d]\n", board[*move + *index][move[1] + *index], *move + *index, move[1] + *index);
            *item_at_position = board[*move + *index][move[1] + *index];
        }
        if (*x_moves < 0 && *y_moves > 0) {
            printf("%lc [%d,%d]\n", board[*move + *index][move[1] - *index], *move + *index, move[1] - *index);
            *item_at_position = board[*move + *index][move[1] - *index];
        }

        if (*item_at_position != 0) {
            free(index);
            free(x_moves);
            free(y_moves);
            free(item_at_position);
            return false;
        }

        (*index)--;
    }

    free(index);
    free(x_moves);
    free(y_moves);
    free(item_at_position);

    return true;
}

bool is_syntax_valid(int player, char *move) {
    // Look for -
    if (move[2] != '-') {
        send(player, "e-00", 4, 0);
        return false;
    }
    // First and 3th should be characters
    if (move[0] - '0' < 10) {
        send(player, "e-01", 4, 0);
        return false;
    }
    if (move[3] - '0' < 10) {
        send(player, "e-02", 4, 0);
        return false;
    }

    // Second and 5th character should be numbers
    if (move[1] - '0' > 10) {
        send(player, "e-03", 4, 0);
        return false;
    }
    if (move[1] - '0' > 8) {
        send(player, "e-04", 4, 0);
        return false;
    }
    if (move[4] - '0' > 10) {
        send(player, "e-05", 4, 0);
        return false;
    }
    if (move[4] - '0' > 8) {
        send(player, "e-06", 4, 0);
        return false;
    }
    // Move out of range
    if (move[0] - '0' > 56 || move[3] - '0' > 56) {
        send(player, "e-07", 4, 0);
        return false;
    }

    return true;
}

int get_piece_team(wchar_t **board, int x, int y) {
    switch (board[x][y]) {
        case white_king:
            return -1;
        case white_queen:
            return -1;
        case white_rook:
            return -1;
        case white_bishop:
            return -1;
        case white_knight:
            return -1;
        case white_pawn:
            return -1;
        case black_king:
            return 1;
        case black_queen:
            return 1;
        case black_rook:
            return 1;
        case black_bishop:
            return 1;
        case black_knight:
            return 1;
        case black_pawn:
            return 1;
    }

    return 0;
}

int get_piece_type(wchar_t piece) {
    switch (piece) {
        case white_king:
            return 0;
        case white_queen:
            return 1;
        case white_rook:
            return 2;
        case white_bishop:
            return 3;
        case white_knight:
            return 4;
        case white_pawn:
            return 5;
        case black_king:
            return 0;
        case black_queen:
            return 1;
        case black_rook:
            return 2;
        case black_bishop:
            return 3;
        case black_knight:
            return 4;
        case black_pawn:
            return 5;
    }
    return -1;
}

bool eat_piece(wchar_t **board, int x, int y) {
    return (get_piece_team(board, x, y) != 0);
}

bool is_rect(int *move) {
    int *x_moves = (int *)malloc(sizeof(int));
    int *y_moves = (int *)malloc(sizeof(int));

    *x_moves = *(move)-move[2];
    *y_moves = move[1] - move[3];

    if ((*x_moves != 0 && *y_moves == 0) || (*y_moves != 0 && *x_moves == 0)) {
        free(x_moves);
        free(y_moves);
        return true;
    }

    free(x_moves);
    free(y_moves);
    return false;
}

int is_rect_clear(wchar_t **board, int *move, int x_moves, int y_moves) {
    // Is a side rect
    int *index = (int *)malloc(sizeof(int));

    if (abs(x_moves) > abs(y_moves)) {
        *index = abs(x_moves) - 1;
    } else {
        *index = abs(y_moves) - 1;
    }

    // Iterate thru all items excepting initial position
    while (*index > 0) {
        if (*(move)-move[2] > 0) {
            if (board[*move - (*index)][move[1]] != 0) {
                free(index);
                return false;
            }
        }
        if (*(move)-move[2] < 0) {
            if (board[*move + (*index)][move[1]] != 0) {
                free(index);
                return false;
            }
        }
        if (move[1] - move[3] > 0) {
            if (board[*move][move[1] - (*index)] != 0) {
                free(index);
                return false;
            }
        }
        if (move[1] - move[3] < 0) {
            if (board[*move][move[1] + (*index)] != 0) {
                free(index);
                return false;
            }
        }

        (*index)--;
    }

    free(index);
    return true;
}

void promote_piece(wchar_t **board, int destX, int destY, int team) {
    if (team == 1) {
        board[destX][destY] = black_queen;
    } else if (team == -1) {
        board[destX][destY] = white_queen;
    }
}

bool is_diagonal(int x_moves, int y_moves) {
    if ((abs(x_moves) - abs(y_moves)) != 0) {
        return false;
    }

    return true;
}

bool is_move_valid(wchar_t **board, int player, int team, int *move) {
    int piece_team = get_piece_team(board, move[0], move[1]);
    int x_moves = getManitud(move[0], move[2]);
    int y_moves = getManitud(move[1], move[3]);

    if (!general_errors(board, player, team, move, piece_team)) {
        return false;
    }

    return validate_piece_move(board, player, piece_team, x_moves, y_moves, move);
}

// Function to handle general errors
bool general_errors(wchar_t **board, int player, int team, int *move, int piece_team) {
    if (board[move[0]][move[1]] == 0) {
        send(player, "e-08", 4, 0);
        return false; // If selected piece == 0 there's nothing selected
    }
    if (piece_team == get_piece_team(board, move[2], move[3])) {
        send(player, "e-09", 4, 0);
        return false; // Origin piece's team == destination piece's team, invalid move
    }
    // Check if user is moving their own piece
    if (team != piece_team) {
        send(player, "e-07", 4, 0);
        return false;
    }
    return true;
}

// Function to validate piece movement based on type
bool validate_piece_move(wchar_t **board, int player, int piece_team, int x_moves, int y_moves, int *move) {
    int piece_type = get_piece_type(board[move[0]][move[1]]);
    
    switch (piece_type) {
        case 0: // --- ♚ --- (King)
            return validate_king_move(player, x_moves, y_moves);
        case 1: // --- ♛ --- (Queen)
            return validate_queen_move(board, player, x_moves, y_moves, move);
        case 2: // --- ♜ --- (Rook)
            return validate_rook_move(board, player, x_moves, y_moves, move);
        case 3: // --- ♝ --- (Bishop)
            return validate_bishop_move(board, player, x_moves, y_moves, move);
        case 4: // --- ♞ --- (Knight)
            return validate_knight_move(player, x_moves, y_moves);
        case 5: // --- ♟ --- (Pawn)
            return validate_pawn_move(board, player, piece_team, x_moves, y_moves, move);
        default:
            break;
    }
    return true;
}

// Function to validate king movement
bool validate_king_move(int player, int x_moves, int y_moves) {
    if (x_moves > 1 || y_moves > 1) {
        send(player, "e-10", 5, 0);
        return false;
    }
    return true;
}

// Function to validate queen movement
bool validate_queen_move(wchar_t **board, int player, int x_moves, int y_moves, int *move) {
    if (!is_rect(move) && !is_diagonal(x_moves, y_moves)) {
        send(player, "e-queen", 4, 0);
        return false;
    } else if (is_rect(move)) {
    	  if (!is_rect_clear(board, move, x_moves, y_moves))
           send(player, "e-31", 4, 0);
            return false;
    } else {
      if (!is_diagonal(x_moves, y_moves)) {
        send(player, "e-31", 4, 0);
      	return false;
      }
    }
    if (eat_piece(board, move[2], move[3])) {
        send(player, "i-99", 4, 0);
        return true;
    }
    return true;
}

// Function to validate rook movement
bool validate_rook_move(wchar_t **board, int player, int x_moves, int y_moves, int *move) {
    if (!is_rect(move)) {
        send(player, "e-30", 5, 0);
        return false;
    }
    if (!is_rect_clear(board, move, x_moves, y_moves)) {
        send(player, "e-31", 4, 0);
        return false;
    }
    if (eat_piece(board, move[2], move[3])) {
        send(player, "i-99", 4, 0);
        return true;
    }
    return true;
}

// Function to validate bishop movement
bool validate_bishop_move(wchar_t **board, int player, int x_moves, int y_moves, int *move) {
    if (!is_diagonal(x_moves, y_moves) || !is_diagonal_clear(board, move)) {
        send(player, "e-40", 4, 0);
        return false; // Invalid diagonal move
    }
    if (eat_piece(board, move[2], move[3])) {
        send(player, "i-99", 4, 0);
        return true;
    }
    return true;
}

// Function to validate knight movement
bool validate_knight_move(int player, int x_moves, int y_moves) {
    if ((abs(x_moves) != 1 || abs(y_moves) != 2) && (abs(x_moves) != 2 || abs(y_moves) != 1)) {
        send(player, "e-50", 4, 0);
        return false;
    }
    return true;
}

// Function to validate pawn movement
bool validate_pawn_move(wchar_t **board, int player, int piece_team, int x_moves, int y_moves, int *move) {
    if (y_moves == 1) {
        return validate_pawn_capture(board, player, piece_team, move);
    } else if (y_moves == 0) {
        return validate_pawn_advance(board, player, piece_team, x_moves, move);
    }
    return false;
}

// Function to validate pawn capturing
bool validate_pawn_capture(wchar_t **board, int player, int piece_team, int *move) {
    if ((piece_team == 1 && (move[0] - move[2]) == 1) || (piece_team == -1 && (move[0] - move[2]) == -1)) {
        if (eat_piece(board, move[2], move[3])) {
            send(player, "i-99", 4, 0);
            if ((piece_team == 1 && move[2] == 0) || (piece_team == -1 && move[2] == 7)) {
                promote_piece(board, move[0], move[1], piece_team);
                send(player, "i-98", 4, 0);
            }
            return true;
        }
    }
    return false;
}

// Function to validate pawn advancing
bool validate_pawn_advance(wchar_t **board, int player, int piece_team, int x_moves, int *move) {
    if (x_moves >= 2) {
        if (!is_rect_clear(board, move, x_moves, 0)) {
            send(player, "e-31", 4, 0);
            return false;
        } else if ((move[0] == 6 && piece_team == 1 && (move[0] - move[2]) == 2 && get_piece_type(board[move[2]][move[3]]) == -1) ||
                   (move[0] == 1 && piece_team == -1 && (move[0] - move[2]) == -2 && get_piece_type(board[move[2]][move[3]]) == -1)) {
            printf("First move\n");
            return true;
        } else {
            send(player, "e-62", 5, 0);
            return false;
        }
    } else if (x_moves == 1) {
        if ((piece_team == 1 && (move[0] - move[2]) == 1 && get_piece_type(board[move[2]][move[3]]) == -1) ||
            (piece_team == -1 && (move[0] - move[2]) == -1 && get_piece_type(board[move[2]][move[3]]) == -1)) {
            if ((piece_team == 1 && move[2] == 0) || (piece_team == -1 && move[2] == 7)) {
                promote_piece(board, move[0], move[1], piece_team);
                send(player, "i-98", 4, 0);
            }
            return true;
        }
    }
    return false;
}

bool check_end_game(wchar_t **board) {
    int game_over = 2;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (get_piece_type(board[i][j]) == 0) {
                game_over--;
            }
        }
    }
    if (game_over == 0) {
        return false;
    } else {
        return true;
    }
}