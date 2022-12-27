#include "../include/chess.h"

///////////////////////////////////////////////////////////////
//  MOVE LOGIC

flag_t newCastleRightsByCapture(board_t* board, piece_t captured, idx_t pos) {
    flag_t cr = board->castle_rights;

    if (board->player == BLACK) {
        if (captured == ROOK) {
            switch (pos) {
                case 0:
                    cr = cr & (~LONGSIDEW);
                    break;
                case 7:
                    cr = cr & (~SHORTSIDEW);
                    break;
                default:
                    break;
            }
        }
    } else {
        if (captured == (ROOK | BLACK)) {
            switch (pos) {
                case 56:
                    cr = cr & (~LONGSIDEB);
                    break;
                case 63:
                    cr = cr & (~SHORTSIDEB);
                    break;
                default:
                    break;
            }
        }
    }

    return cr;
}

flag_t newCastleRightsByMove(board_t* board, piece_t piece, idx_t pos, flag_t oldcr) {
    flag_t cr = oldcr;

    if (board->player == WHITE) {
        // IF KING MOVE
        if (piece == KING) {
            cr = cr & (~(LONGSIDEW | SHORTSIDEW));
        }
        // IF ROOK MOVE
        else if (piece == ROOK) {
            switch (pos) {
                case 0:
                    cr = cr & (~LONGSIDEW);
                    break;
                case 7:
                    cr = cr & (~SHORTSIDEW);
                    break;
                default:
                    break;
            }
        }
    } else {
        // IF KING MOVE
        if (piece == (KING | BLACK)) {
            cr = cr & (~(LONGSIDEB | SHORTSIDEB));
        }

        else if (piece == (ROOK | BLACK)) {
            switch (pos) {
                case 56:
                    cr = cr & (~LONGSIDEB);
                    break;
                case 63:
                    cr = cr & (~SHORTSIDEB);
                    break;
                default:
                    break;
            }
        }
    }

    return cr;
}

void generatePawnMoves(board_t* board, node_t* movelst, idx_t idx) {
    ///////////////////////
    //  if WHITE at turn
    if (board->player == WHITE) {
        // one step foward
        if (board->playing_field[idx + 8] == EMPTY) {
            flag_t new_cr = board->castle_rights;
            // if promotion
            if ((idx + 8) >= 56) {
                add(movelst, create_promotion_move(PAWN, KNIGHT, 0, idx, idx + 8, new_cr, copy_flags(board)));
                add(movelst, create_promotion_move(PAWN, BISHOP, 0, idx, idx + 8, new_cr, copy_flags(board)));
                add(movelst, create_promotion_move(PAWN, ROOK, 0, idx, idx + 8, new_cr, copy_flags(board)));
                add(movelst, create_promotion_move(PAWN, QUEEN, 0, idx, idx + 8, new_cr, copy_flags(board)));
            }
            // if no promotion
            else {
                add(movelst, create_normal_move(PAWN, 0, idx, idx + 8, new_cr, copy_flags(board)));
            }
        }

        // two step foward
        if (idx >= 8 && idx <= 15) {
            if (board->playing_field[idx + 8] == EMPTY && board->playing_field[idx + 16] == EMPTY) {
                add(movelst, create_ep_possible_move(PAWN, idx, idx + 16, idx + 8, copy_flags(board)));
            }
        }

        // if can take to left
        piece_t totakeleft;
        if (idx % 8 != 0 && (totakeleft = board->playing_field[idx + 7]) != EMPTY) {
            if (COLOR(totakeleft) == BLACK) {
                flag_t new_cr = newCastleRightsByCapture(board, totakeleft, idx + 7);
                // if promotion
                if ((idx + 7) >= 56) {
                    add(movelst, create_promotion_move(PAWN, KNIGHT, totakeleft, idx, idx + 7, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN, BISHOP, totakeleft, idx, idx + 7, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN, ROOK, totakeleft, idx, idx + 7, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN, QUEEN, totakeleft, idx, idx + 7, new_cr, copy_flags(board)));
                }
                // if no promotion
                else {
                    add(movelst, create_normal_move(PAWN, totakeleft, idx, idx + 7, new_cr, copy_flags(board)));
                }
            }
        }

        // if can take to right
        piece_t totakeright;
        if (idx % 8 != 7 && (totakeright = board->playing_field[idx + 9]) != EMPTY) {
            if (COLOR(totakeright) == BLACK) {
                flag_t new_cr = newCastleRightsByCapture(board, totakeright, idx + 9);
                // if promotion
                if ((idx + 9) >= 56) {
                    add(movelst, create_promotion_move(PAWN, KNIGHT, totakeright, idx, idx + 9, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN, BISHOP, totakeright, idx, idx + 9, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN, ROOK, totakeright, idx, idx + 9, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN, QUEEN, totakeright, idx, idx + 9, new_cr, copy_flags(board)));
                }
                // if no promotion
                else {
                    add(movelst, create_normal_move(PAWN, totakeright, idx, idx + 9, new_cr, copy_flags(board)));
                }
            }
        }
    }
    ///////////////////////
    //  if BLACK at turn
    else {
        // one step foward
        if (board->playing_field[idx - 8] == EMPTY) {
            flag_t new_cr = board->castle_rights;
            // if promotion
            if ((idx - 8) <= 7) {
                add(movelst, create_promotion_move(PAWN | BLACK, KNIGHT | BLACK, 0, idx, idx - 8, new_cr, copy_flags(board)));
                add(movelst, create_promotion_move(PAWN | BLACK, BISHOP | BLACK, 0, idx, idx - 8, new_cr, copy_flags(board)));
                add(movelst, create_promotion_move(PAWN | BLACK, ROOK | BLACK, 0, idx, idx - 8, new_cr, copy_flags(board)));
                add(movelst, create_promotion_move(PAWN | BLACK, QUEEN | BLACK, 0, idx, idx - 8, new_cr, copy_flags(board)));
            }
            // if no promotion
            else {
                add(movelst, create_normal_move(PAWN | BLACK, 0, idx, idx - 8, new_cr, copy_flags(board)));
            }
        }

        // two step foward
        if (idx >= 48 && idx <= 55) {
            if (board->playing_field[idx - 8] == EMPTY && board->playing_field[idx - 16] == EMPTY) {
                add(movelst, create_ep_possible_move(PAWN | BLACK, idx, idx - 16, idx - 8, copy_flags(board)));
            }
        }

        // if can take to left
        piece_t totakeleft;
        if (idx % 8 != 7 && (totakeleft = board->playing_field[idx - 7]) != EMPTY) {
            if (COLOR(totakeleft) == WHITE) {
                flag_t new_cr = newCastleRightsByCapture(board, totakeleft, idx - 7);
                // if promotion
                if ((idx - 7) <= 7) {
                    add(movelst, create_promotion_move(PAWN | BLACK, KNIGHT | BLACK, totakeleft, idx, idx - 7, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN | BLACK, BISHOP | BLACK, totakeleft, idx, idx - 7, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN | BLACK, ROOK | BLACK, totakeleft, idx, idx - 7, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN | BLACK, QUEEN | BLACK, totakeleft, idx, idx - 7, new_cr, copy_flags(board)));
                }
                // if no promotion
                else {
                    add(movelst, create_normal_move(PAWN | BLACK, totakeleft, idx, idx - 7, new_cr, copy_flags(board)));
                }
            }
        }

        // if can take to right
        piece_t totakeright;
        if (idx % 8 != 0 && (totakeright = board->playing_field[idx - 9]) != EMPTY) {
            if (COLOR(totakeright) == WHITE) {
                flag_t new_cr = newCastleRightsByCapture(board, totakeright, idx - 9);
                // if promotion
                if ((idx - 9) <= 7) {
                    add(movelst, create_promotion_move(PAWN | BLACK, KNIGHT | BLACK, totakeright, idx, idx - 9, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN | BLACK, BISHOP | BLACK, totakeright, idx, idx - 9, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN | BLACK, ROOK | BLACK, totakeright, idx, idx - 9, new_cr, copy_flags(board)));
                    add(movelst, create_promotion_move(PAWN | BLACK, QUEEN | BLACK, totakeright, idx, idx - 9, new_cr, copy_flags(board)));
                }
                // if no promotion
                else {
                    add(movelst, create_normal_move(PAWN | BLACK, totakeright, idx, idx - 9, new_cr, copy_flags(board)));
                }
            }
        }
    }
}

void generateKnightMoves(board_t* board, node_t* movelst, idx_t idx) {
    // TWO UP ONE RIGHT
    if ((idx - 48 < 0) && (idx % 8 != 7)) {
        piece_t captured = board->playing_field[idx + 17];
        piece_t new_cr = newCastleRightsByCapture(board, captured, idx + 17);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KNIGHT | (board->player), 0, idx, idx + 17, board->castle_rights, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KNIGHT | (board->player), captured, idx, idx + 17, new_cr, copy_flags(board)));
        }
    }

    // TWO UP ONE LEFT
    if ((idx - 48 < 0) && (idx % 8 != 0)) {
        piece_t captured = board->playing_field[idx + 15];
        piece_t new_cr = newCastleRightsByCapture(board, captured, idx + 15);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KNIGHT | (board->player), 0, idx, idx + 15, board->castle_rights, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KNIGHT | (board->player), captured, idx, idx + 15, new_cr, copy_flags(board)));
        }
    }

    // ONE UP TWO RIGHT
    if ((idx - 56 < 0) && (idx % 8 != 7) && (idx % 8 != 6)) {
        piece_t captured = board->playing_field[idx + 10];
        piece_t new_cr = newCastleRightsByCapture(board, captured, idx + 10);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KNIGHT | (board->player), 0, idx, idx + 10, board->castle_rights, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KNIGHT | (board->player), captured, idx, idx + 10, new_cr, copy_flags(board)));
        }
    }

    // ONE UP TWO LEFT
    if ((idx - 56 < 0) && (idx % 8 != 0) && (idx % 8 != 1)) {
        piece_t captured = board->playing_field[idx + 6];
        piece_t new_cr = newCastleRightsByCapture(board, captured, idx + 6);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KNIGHT | (board->player), 0, idx, idx + 6, board->castle_rights, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KNIGHT | (board->player), captured, idx, idx + 6, new_cr, copy_flags(board)));
        }
    }

    // TWO DOWN ONE RIGHT
    if ((idx - 16 >= 0) && (idx % 8 != 7)) {
        piece_t captured = board->playing_field[idx - 15];
        piece_t new_cr = newCastleRightsByCapture(board, captured, idx - 15);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KNIGHT | (board->player), 0, idx, idx - 15, board->castle_rights, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KNIGHT | (board->player), captured, idx, idx - 15, new_cr, copy_flags(board)));
        }
    }

    // TWO DOWN ONE LEFT
    if ((idx - 16 >= 0) && (idx % 8 != 0)) {
        piece_t captured = board->playing_field[idx - 17];
        piece_t new_cr = newCastleRightsByCapture(board, captured, idx - 17);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KNIGHT | (board->player), 0, idx, idx - 17, board->castle_rights, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KNIGHT | (board->player), captured, idx, idx - 17, new_cr, copy_flags(board)));
        }
    }

    // ONE DOWN TWO RIGHT
    if ((idx - 8 >= 0) && (idx % 8 != 6) && (idx % 8 != 7)) {
        piece_t captured = board->playing_field[idx - 6];
        piece_t new_cr = newCastleRightsByCapture(board, captured, idx - 6);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KNIGHT | (board->player), 0, idx, idx - 6, board->castle_rights, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KNIGHT | (board->player), captured, idx, idx - 6, new_cr, copy_flags(board)));
        }
    }

    // ONE DOWN TWO LEFT
    if ((idx - 8 >= 0) && (idx % 8 != 0) && (idx % 8 != 1)) {
        piece_t captured = board->playing_field[idx - 10];
        piece_t new_cr = newCastleRightsByCapture(board, captured, idx - 10);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KNIGHT | (board->player), 0, idx, idx - 10, board->castle_rights, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KNIGHT | (board->player), captured, idx, idx - 10, new_cr, copy_flags(board)));
        }
    }
}

void generateRookMoves(board_t* board, node_t* movelst, idx_t idx) {
    idx_t start = idx;

    // NORTH
    while (idx + 8 < 64) {
        piece_t captured = board->playing_field[idx + 8];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 8);
        new_cr = newCastleRightsByMove(board, board->playing_field[start], start, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(ROOK | (board->player), 0, start, idx + 8, new_cr, copy_flags(board)));
            idx += 8;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(ROOK | (board->player), captured, start, idx + 8, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // EAST
    while (((idx + 1) % 8) != 0) {
        piece_t captured = board->playing_field[idx + 1];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 1);
        new_cr = newCastleRightsByMove(board, board->playing_field[start], start, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(ROOK | (board->player), 0, start, idx + 1, new_cr, copy_flags(board)));
            idx += 1;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(ROOK | (board->player), captured, start, idx + 1, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // SOUTH
    while (idx - 8 >= 0) {
        piece_t captured = board->playing_field[idx - 8];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 8);
        new_cr = newCastleRightsByMove(board, board->playing_field[start], start, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(ROOK | (board->player), 0, start, idx - 8, new_cr, copy_flags(board)));
            idx -= 8;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(ROOK | (board->player), captured, start, idx - 8, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // WEST
    while ((idx - 1 >= 0) && (((idx - 1) % 8) != 7)) {
        piece_t captured = board->playing_field[idx - 1];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 1);
        new_cr = newCastleRightsByMove(board, board->playing_field[start], start, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(ROOK | (board->player), 0, start, idx - 1, new_cr, copy_flags(board)));
            idx -= 1;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(ROOK | (board->player), captured, start, idx - 1, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }
}

void generateBishopMoves(board_t* board, node_t* movelst, idx_t idx) {
    idx_t start = idx;

    // NORTH EAST
    while ((idx + 9 < 64) && ((idx) % 8 != 7)) {
        piece_t captured = board->playing_field[idx + 9];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 9);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(BISHOP | (board->player), 0, start, idx + 9, board->castle_rights, copy_flags(board)));
            idx += 9;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(BISHOP | (board->player), captured, start, idx + 9, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // SOUTH EAST
    while ((idx - 7 >= 0) && ((idx) % 8 != 7)) {
        piece_t captured = board->playing_field[idx - 7];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 7);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(BISHOP | (board->player), 0, start, idx - 7, board->castle_rights, copy_flags(board)));
            idx -= 7;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(BISHOP | (board->player), captured, start, idx - 7, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // SOUTH WEST
    while ((idx - 9 >= 0) && ((idx) % 8 != 0)) {
        piece_t captured = board->playing_field[idx - 9];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 9);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(BISHOP | (board->player), 0, start, idx - 9, board->castle_rights, copy_flags(board)));
            idx -= 9;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(BISHOP | (board->player), captured, start, idx - 9, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // NORTH WEST
    while ((idx + 7 < 64) && ((idx) % 8 != 0)) {
        piece_t captured = board->playing_field[idx + 7];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 7);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(BISHOP | (board->player), 0, start, idx + 7, board->castle_rights, copy_flags(board)));
            idx += 7;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(BISHOP | (board->player), captured, start, idx + 7, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }
}

void generateQueenMoves(board_t* board, node_t* movelst, idx_t idx) {
    idx_t start = idx;

    // NORTH
    while (idx + 8 < 64) {
        piece_t captured = board->playing_field[idx + 8];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 8);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(QUEEN | (board->player), 0, start, idx + 8, board->castle_rights, copy_flags(board)));
            idx += 8;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(QUEEN | (board->player), captured, start, idx + 8, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // EAST
    while (((idx + 1) % 8) != 0) {
        piece_t captured = board->playing_field[idx + 1];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 1);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(QUEEN | (board->player), 0, start, idx + 1, board->castle_rights, copy_flags(board)));
            idx += 1;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(QUEEN | (board->player), captured, start, idx + 1, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // SOUTH
    while (idx - 8 >= 0) {
        piece_t captured = board->playing_field[idx - 8];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 8);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(QUEEN | (board->player), 0, start, idx - 8, board->castle_rights, copy_flags(board)));
            idx -= 8;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(QUEEN | (board->player), captured, start, idx - 8, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // WEST
    while ((idx - 1 >= 0) && (((idx - 1) % 8) != 7)) {
        piece_t captured = board->playing_field[idx - 1];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 1);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(QUEEN | (board->player), 0, start, idx - 1, board->castle_rights, copy_flags(board)));
            idx -= 1;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(QUEEN | (board->player), captured, start, idx - 1, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // NORTH EAST
    while ((idx + 9 < 64) && ((idx) % 8 != 7)) {
        piece_t captured = board->playing_field[idx + 9];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 9);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(QUEEN | (board->player), 0, start, idx + 9, board->castle_rights, copy_flags(board)));
            idx += 9;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(QUEEN | (board->player), captured, start, idx + 9, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // SOUTH EAST
    while ((idx - 7 >= 0) && ((idx) % 8 != 7)) {
        piece_t captured = board->playing_field[idx - 7];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 7);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(QUEEN | (board->player), 0, start, idx - 7, board->castle_rights, copy_flags(board)));
            idx -= 7;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(QUEEN | (board->player), captured, start, idx - 7, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // SOUTH WEST
    while ((idx - 9 >= 0) && ((idx) % 8 != 0)) {
        piece_t captured = board->playing_field[idx - 9];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 9);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(QUEEN | (board->player), 0, start, idx - 9, board->castle_rights, copy_flags(board)));
            idx -= 9;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(QUEEN | (board->player), captured, start, idx - 9, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }

    idx = start;

    // NORTH WEST
    while ((idx + 7 < 64) && ((idx) % 8 != 0)) {
        piece_t captured = board->playing_field[idx + 7];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 7);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(QUEEN | (board->player), 0, start, idx + 7, board->castle_rights, copy_flags(board)));
            idx += 7;
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(QUEEN | (board->player), captured, start, idx + 7, new_cr, copy_flags(board)));
            break;
        } else {
            break;
        }
    }
}

void generateKingMoves(board_t* board, node_t* movelst, idx_t idx) {
    // NORTH
    if (idx + 8 < 64) {
        piece_t captured = board->playing_field[idx + 8];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 8);
        new_cr = newCastleRightsByMove(board, board->playing_field[idx], idx, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KING | (board->player), 0, idx, idx + 8, new_cr, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KING | (board->player), captured, idx, idx + 8, new_cr, copy_flags(board)));
        }
    }

    // NORTH EAST
    if ((idx + 9 < 64) && (idx % 8 != 7)) {
        piece_t captured = board->playing_field[idx + 9];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 9);
        new_cr = newCastleRightsByMove(board, board->playing_field[idx], idx, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KING | (board->player), 0, idx, idx + 9, new_cr, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KING | (board->player), captured, idx, idx + 9, new_cr, copy_flags(board)));
        }
    }

    // EAST
    if ((idx % 8 != 7)) {
        piece_t captured = board->playing_field[idx + 1];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 1);
        new_cr = newCastleRightsByMove(board, board->playing_field[idx], idx, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KING | (board->player), 0, idx, idx + 1, new_cr, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KING | (board->player), captured, idx, idx + 1, new_cr, copy_flags(board)));
        }
    }

    // SOUTH EAST
    if ((idx - 7 >= 0) && (idx % 8 != 7)) {
        piece_t captured = board->playing_field[idx - 7];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 7);
        new_cr = newCastleRightsByMove(board, board->playing_field[idx], idx, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KING | (board->player), 0, idx, idx - 7, new_cr, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KING | (board->player), captured, idx, idx - 7, new_cr, copy_flags(board)));
        }
    }

    // SOUTH
    if ((idx - 8 >= 0)) {
        piece_t captured = board->playing_field[idx - 8];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 8);
        new_cr = newCastleRightsByMove(board, board->playing_field[idx], idx, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KING | (board->player), 0, idx, idx - 8, new_cr, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KING | (board->player), captured, idx, idx - 8, new_cr, copy_flags(board)));
        }
    }

    // SOUTH WEST
    if ((idx - 9 >= 0) && (idx % 8 != 0)) {
        piece_t captured = board->playing_field[idx - 9];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 9);
        new_cr = newCastleRightsByMove(board, board->playing_field[idx], idx, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KING | (board->player), 0, idx, idx - 9, new_cr, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KING | (board->player), captured, idx, idx - 9, new_cr, copy_flags(board)));
        }
    }

    // WEST
    if ((idx % 8 != 0)) {
        piece_t captured = board->playing_field[idx - 1];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx - 1);
        new_cr = newCastleRightsByMove(board, board->playing_field[idx], idx, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KING | (board->player), 0, idx, idx - 1, new_cr, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KING | (board->player), captured, idx, idx - 1, new_cr, copy_flags(board)));
        }
    }

    // NORTH WEST
    if ((idx + 7 < 64) && (idx % 8 != 0)) {
        piece_t captured = board->playing_field[idx + 7];
        flag_t new_cr = newCastleRightsByCapture(board, captured, idx + 7);
        new_cr = newCastleRightsByMove(board, board->playing_field[idx], idx, new_cr);
        if (captured == EMPTY) {
            add(movelst, create_normal_move(KING | (board->player), 0, idx, idx + 7, new_cr, copy_flags(board)));
        } else if (COLOR(captured) == OPPONENT(board->player)) {
            add(movelst, create_normal_move(KING | (board->player), captured, idx, idx + 7, new_cr, copy_flags(board)));
        }
    }
}

void generateCastleMoves(board_t* board, node_t* movelst) {
    if (board->player == WHITE) {
        piece_t A1 = board->playing_field[0];
        piece_t B1 = board->playing_field[1];
        piece_t C1 = board->playing_field[2];
        piece_t D1 = board->playing_field[3];
        piece_t E1 = board->playing_field[4];
        piece_t F1 = board->playing_field[5];
        piece_t G1 = board->playing_field[6];
        piece_t H1 = board->playing_field[7];

        // LONGSIDE
        if (A1 == ROOK && B1 == EMPTY && C1 == EMPTY && D1 == EMPTY && E1 == KING && (board->castle_rights & LONGSIDEW) != 0) {
            add(movelst, create_castle_move(4, 2, 0, 3, board->castle_rights & (~(LONGSIDEW | SHORTSIDEW)), copy_flags(board)));
        }
        // SHORTSIDE
        if (H1 == ROOK && G1 == EMPTY && F1 == EMPTY && E1 == KING && (board->castle_rights & SHORTSIDEW) != 0) {
            add(movelst, create_castle_move(4, 6, 7, 5, board->castle_rights & (~(LONGSIDEW | SHORTSIDEW)), copy_flags(board)));
        }
    } else {
        piece_t A8 = board->playing_field[56];
        piece_t B8 = board->playing_field[57];
        piece_t C8 = board->playing_field[58];
        piece_t D8 = board->playing_field[59];
        piece_t E8 = board->playing_field[60];
        piece_t F8 = board->playing_field[61];
        piece_t G8 = board->playing_field[62];
        piece_t H8 = board->playing_field[63];

        // LONGSIDE
        if (A8 == (ROOK | BLACK) && B8 == EMPTY && C8 == EMPTY && D8 == EMPTY && E8 == (KING | BLACK) && (board->castle_rights & LONGSIDEB) != 0) {
            add(movelst, create_castle_move(60, 58, 56, 59, board->castle_rights & (~(LONGSIDEB | SHORTSIDEB)), copy_flags(board)));
        }
        // SHORTSIDE
        if (H8 == (ROOK | BLACK) && G8 == EMPTY && F8 == EMPTY && E8 == (KING | BLACK) && (board->castle_rights & SHORTSIDEB) != 0) {
            add(movelst, create_castle_move(60, 62, 63, 61, board->castle_rights & (~(LONGSIDEB | SHORTSIDEB)), copy_flags(board)));
        }
    }
}

void generateEnpassantMoves(board_t* board, node_t* movelst) {
    if (board->player == WHITE) {
        // FROM THE RIGHT
        if ((board->ep_field != 47) && board->playing_field[board->ep_field - 7] == PAWN) {
            add(movelst, create_ep_move(board->ep_field - 7, board->ep_field, copy_flags(board)));
        }

        // FROM THE LEFT
        if ((board->ep_field != 40) && board->playing_field[board->ep_field - 9] == PAWN) {
            add(movelst, create_ep_move(board->ep_field - 9, board->ep_field, copy_flags(board)));
        }
    } else {
        // FROM THE RIGHT
        if ((board->ep_field != 23) && board->playing_field[board->ep_field + 9] == (PAWN | BLACK)) {
            add(movelst, create_ep_move(board->ep_field + 9, board->ep_field, copy_flags(board)));
        }

        // FROM THE LEFT
        if ((board->ep_field != 16) && board->playing_field[board->ep_field + 7] == (PAWN | BLACK)) {
            add(movelst, create_ep_move(board->ep_field + 7, board->ep_field, copy_flags(board)));
        }
    }
}

int is_legal_move(board_t* board) {
    player_t playerwhomademove = (board->player == BLACK) ? WHITE : BLACK;
    idx_t idx = 0;

    // find idx of king of player who made a (possibly illegal) move

    while (board->playing_field[idx] != (KING | playerwhomademove)) {
        idx++;
    }

    idx_t start = idx;

    // NORTH
    while (idx + 8 < 64) {
        piece_t captured = board->playing_field[idx + 8];
        if (captured == EMPTY) {
            idx += 8;
        } else if (COLOR(captured) == OPPONENT(playerwhomademove)) {
            if (FIGURE(captured) == ROOK || FIGURE(captured) == QUEEN) {
                return 0;
            }
            if (FIGURE(captured) == KING && idx == start) {
                return 0;
            }
            break;
        } else {
            break;
        }
    }

    idx = start;

    // EAST
    while (((idx + 1) % 8) != 0) {
        piece_t captured = board->playing_field[idx + 1];
        if (captured == EMPTY) {
            idx += 1;
        } else if (COLOR(captured) == OPPONENT(playerwhomademove)) {
            if (FIGURE(captured) == ROOK || FIGURE(captured) == QUEEN) {
                return 0;
            }
            if (FIGURE(captured) == KING && idx == start) {
                return 0;
            }
            break;
        } else {
            break;
        }
    }

    idx = start;

    // SOUTH
    while (idx - 8 >= 0) {
        piece_t captured = board->playing_field[idx - 8];
        if (captured == EMPTY) {
            idx -= 8;
        } else if (COLOR(captured) == OPPONENT(playerwhomademove)) {
            if (FIGURE(captured) == ROOK || FIGURE(captured) == QUEEN) {
                return 0;
            }
            if (FIGURE(captured) == KING && idx == start) {
                return 0;
            }
            break;
        } else {
            break;
        }
    }

    idx = start;

    // WEST
    while ((idx - 1 >= 0) && (((idx - 1) % 8) != 7)) {
        piece_t captured = board->playing_field[idx - 1];
        if (captured == EMPTY) {
            idx -= 1;
        } else if (COLOR(captured) == OPPONENT(playerwhomademove)) {
            if (FIGURE(captured) == ROOK || FIGURE(captured) == QUEEN) {
                return 0;
            }
            if (FIGURE(captured) == KING && idx == start) {
                return 0;
            }
            break;
        } else {
            break;
        }
    }

    idx = start;

    // NORTH EAST
    while ((idx + 9 < 64) && ((idx) % 8 != 7)) {
        piece_t captured = board->playing_field[idx + 9];
        if (captured == EMPTY) {
            idx += 9;
        } else if (COLOR(captured) == OPPONENT(playerwhomademove)) {
            if (FIGURE(captured) == BISHOP || FIGURE(captured) == QUEEN) {
                return 0;
            }
            if (FIGURE(captured) == PAWN && playerwhomademove == WHITE && idx == start) {
                return 0;
            }
            if (FIGURE(captured) == KING && idx == start) {
                return 0;
            }
            break;
        } else {
            break;
        }
    }

    idx = start;

    // SOUTH EAST
    while ((idx - 7 >= 0) && ((idx) % 8 != 7)) {
        piece_t captured = board->playing_field[idx - 7];
        if (captured == EMPTY) {
            idx -= 7;
        } else if (COLOR(captured) == OPPONENT(playerwhomademove)) {
            if (FIGURE(captured) == BISHOP || FIGURE(captured) == QUEEN) {
                return 0;
            }
            if (FIGURE(captured) == PAWN && playerwhomademove == BLACK && idx == start) {
                return 0;
            }
            if (FIGURE(captured) == KING && idx == start) {
                return 0;
            }
            break;
        } else {
            break;
        }
    }

    idx = start;

    // SOUTH WEST
    while ((idx - 9 >= 0) && ((idx) % 8 != 0)) {
        piece_t captured = board->playing_field[idx - 9];
        if (captured == EMPTY) {
            idx -= 9;
        } else if (COLOR(captured) == OPPONENT(playerwhomademove)) {
            if (FIGURE(captured) == BISHOP || FIGURE(captured) == QUEEN) {
                return 0;
            }
            if (FIGURE(captured) == PAWN && playerwhomademove == BLACK && idx == start) {
                return 0;
            }
            if (FIGURE(captured) == KING && idx == start) {
                return 0;
            }
            break;
        } else {
            break;
        }
    }

    idx = start;

    // NORTH WEST
    while ((idx + 7 < 64) && ((idx) % 8 != 0)) {
        piece_t captured = board->playing_field[idx + 7];
        if (captured == EMPTY) {
            idx += 7;
        } else if (COLOR(captured) == OPPONENT(playerwhomademove)) {
            if (FIGURE(captured) == BISHOP || FIGURE(captured) == QUEEN) {
                return 0;
            }
            if (FIGURE(captured) == PAWN && playerwhomademove == WHITE && idx == start) {
                return 0;
            }
            if (FIGURE(captured) == KING && idx == start) {
                return 0;
            }
            break;
        } else {
            break;
        }
    }

    idx = start;

    // TWO UP ONE RIGHT
    if ((idx - 48 < 0) && (idx % 8 != 7)) {
        piece_t captured = board->playing_field[idx + 17];
        if (captured == EMPTY) {
        } else if (COLOR(captured) == OPPONENT(playerwhomademove) && FIGURE(captured) == KNIGHT) {
            return 0;
        }
    }

    // TWO UP ONE LEFT
    if ((idx - 48 < 0) && (idx % 8 != 0)) {
        piece_t captured = board->playing_field[idx + 15];
        if (captured == EMPTY) {
        } else if (COLOR(captured) == OPPONENT(playerwhomademove) && FIGURE(captured) == KNIGHT) {
            return 0;
        }
    }

    // ONE UP TWO RIGHT
    if ((idx - 56 < 0) && (idx % 8 != 7) && (idx % 8 != 6)) {
        piece_t captured = board->playing_field[idx + 10];
        if (captured == EMPTY) {
        } else if (COLOR(captured) == OPPONENT(playerwhomademove) && FIGURE(captured) == KNIGHT) {
            return 0;
        }
    }

    // ONE UP TWO LEFT
    if ((idx - 56 < 0) && (idx % 8 != 0) && (idx % 8 != 1)) {
        piece_t captured = board->playing_field[idx + 6];
        if (captured == EMPTY) {
        } else if (COLOR(captured) == OPPONENT(playerwhomademove) && FIGURE(captured) == KNIGHT) {
            return 0;
        }
    }

    // TWO DOWN ONE RIGHT
    if ((idx - 16 >= 0) && (idx % 8 != 7)) {
        piece_t captured = board->playing_field[idx - 15];
        if (captured == EMPTY) {
        } else if (COLOR(captured) == OPPONENT(playerwhomademove) && FIGURE(captured) == KNIGHT) {
            return 0;
        }
    }

    // TWO DOWN ONE LEFT
    if ((idx - 16 >= 0) && (idx % 8 != 0)) {
        piece_t captured = board->playing_field[idx - 17];
        if (captured == EMPTY) {
        } else if (COLOR(captured) == OPPONENT(playerwhomademove) && FIGURE(captured) == KNIGHT) {
            return 0;
        }
    }

    // ONE DOWN TWO RIGHT
    if ((idx - 8 >= 0) && (idx % 8 != 6) && (idx % 8 != 7)) {
        piece_t captured = board->playing_field[idx - 6];
        if (captured == EMPTY) {
        } else if (COLOR(captured) == OPPONENT(playerwhomademove) && FIGURE(captured) == KNIGHT) {
            return 0;
        }
    }

    // ONE DOWN TWO LEFT
    if ((idx - 8 >= 0) && (idx % 8 != 0) && (idx % 8 != 1)) {
        piece_t captured = board->playing_field[idx - 10];
        if (captured == EMPTY) {
        } else if (COLOR(captured) == OPPONENT(playerwhomademove) && FIGURE(captured) == KNIGHT) {
            return 0;
        }
    }

    return 1;
}

node_t* filterNonCaptures(node_t* movelst) {
    node_t* captureMoves = init_list();
    move_t* move;

    while ((move = pop(movelst)) != NULL) {
        if (move->type_of_move == NORMALMOVE || move->type_of_move == PROMOTIONMOVE) {
            if (move->piece_cap != EMPTY) {
                add(captureMoves, move);
            } else {
                free_move(move);
            }
        } else {
            add(captureMoves, move);
        }
    }
    free(movelst);

    return captureMoves;
}

node_t* filterIllegalMoves(board_t* board, node_t* movelst) {
    node_t* legalMoves = init_list();
    player_t playerwhomademove = board->player;
    move_t* move;

    while ((move = pop(movelst)) != NULL) {
        int islegal;
        // if(move->start == 34 && move->type_of_move == ENPASSANTMOVE){
        //     print_board(board);
        // }
        if (move->type_of_move != CASTLEMOVE) {
            play_move(board, move, playerwhomademove);
            // if(move->start == 34 && move->type_of_move == ENPASSANTMOVE){
            //     print_board(board);
            // }
            islegal = is_legal_move(board);
            reverse_move(board, move, playerwhomademove);

            if (islegal) {
                add(legalMoves, move);
            } else {
                free_move(move);
            }
        } else if (move->type_of_move == CASTLEMOVE) {
            // check that we werent in check before
            board->player = OPPONENT(playerwhomademove);
            int islegal1 = is_legal_move(board);
            board->player = playerwhomademove;

            // check if we castle that we arent casteling through check
            move_t* zwmove = NULL;
            switch (move->end) {
                case 2:
                    zwmove = create_normal_move(KING | playerwhomademove, 0, 4, 3, move->new_cr, copy_flags_from_move(move));
                    break;
                case 6:
                    zwmove = create_normal_move(KING | playerwhomademove, 0, 4, 5, move->new_cr, copy_flags_from_move(move));
                    break;
                case 58:
                    zwmove = create_normal_move(KING | playerwhomademove, 0, 60, 59, move->new_cr, copy_flags_from_move(move));
                    break;
                case 62:
                    zwmove = create_normal_move(KING | playerwhomademove, 0, 60, 61, move->new_cr, copy_flags_from_move(move));
                    break;
                default:
                    fprintf(stderr,"%sSomething went wrong%s\n", Color_PURPLE, Color_END);
                    break;
            }

            play_move(board, zwmove, playerwhomademove);
            int islegal2 = is_legal_move(board);

            free_move(zwmove);

            // check that we arent casteling into check
            play_move(board, move, playerwhomademove);
            islegal = is_legal_move(board);
            reverse_move(board, move, playerwhomademove);

            if (islegal && islegal1 && islegal2) {
                add(legalMoves, move);
            } else {
                free_move(move);
            }
        } else {
            fprintf(stderr, "%sWeird move: %d%s", Color_PURPLE, move->type_of_move, Color_END);
        }
    }
    free(movelst);

    return legalMoves;
}

/* Move generation */
node_t* generate_moves(board_t* board) {
    node_t* movelst = init_list();

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            piece_t piece = board->playing_field[pos_to_idx(x, y)];
            if (board->player == WHITE) {
                switch (piece) {
                    case 2:
                        generatePawnMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 4:
                        generateKnightMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 8:
                        generateBishopMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 16:
                        generateRookMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 32:
                        generateQueenMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 64:
                        generateKingMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    default:
                        break;
                }
            } else {
                switch (piece) {
                    case 3:
                        generatePawnMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 5:
                        generateKnightMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 9:
                        generateBishopMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 17:
                        generateRookMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 33:
                        generateQueenMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    case 65:
                        generateKingMoves(board, movelst, pos_to_idx(x, y));
                        break;
                    default:
                        break;
                }
            }
        }
    }

    generateCastleMoves(board, movelst);

    if (board->ep_possible == TRUE) {
        generateEnpassantMoves(board, movelst);
    }

    node_t* legalMoves = filterIllegalMoves(board, movelst);

    return legalMoves;
}

node_t* generate_captures(board_t* board) {
    return (filterNonCaptures(generate_moves(board)));
}