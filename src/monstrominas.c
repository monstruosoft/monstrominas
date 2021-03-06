/**
 * @file monstrominas.c
 * 
 * @section LICENSE License
 * 
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <https://unlicense.org>
 * 
 * @section DESCRIPTION Description
 * 
 * Minesweeper logic.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "monstrominas.h"



void minesweeper_field_print(MINESWEEPER_FIELD *field) {
    bool (*cells)[field->cols] = (bool (*)[])field->cells;
    int (*hints)[field->cols] = (int (*)[])field->hints;
    bool (*state)[field->cols] = (bool (*)[])field->state;

    for (int row = 0; row < field->rows; row++) {
        for (int col = 0; col < field->cols; col++)
            printf("%d", cells[row][col] ? 1 : 0);
        printf("\t");
        for (int col = 0; col < field->cols; col++)
            printf("%d", hints[row][col]);
        printf("\t");
        for (int col = 0; col < field->cols; col++)
            printf("%d", state[row][col] ? 0 : 1);
        printf("\n");
    }
    printf("\n");
}



MINESWEEPER_FIELD *minesweeper_field_create(int rows, int cols) {
    MINESWEEPER_FIELD *field = calloc(sizeof(MINESWEEPER_FIELD), 1);
    assert(field);
    rows = rows < 10 ? 10 : rows;
    cols = cols < 10 ? 10 : cols;
    field->rows = rows;
    field->cols = cols;
    field->cells = calloc(rows * cols, sizeof(bool));
    field->hints = calloc(rows * cols, sizeof(int));
    field->state = calloc(rows * cols, sizeof(bool));
    field->flags = calloc(rows * cols, sizeof(int));
    field->cell_count = field->rows * field->cols;
    field->cell_size = MINESWEEPER_CELL_SIZE;
    field->move_count = 0;
    assert(field->cells && field->hints && field->state && field->flags);

// minesweeper_field_reset() is called here in case the calling function 
// doesn't reset the field after the player's first move.
    minesweeper_field_reset(field, rand() % rows, rand() % cols, true);
    printf("Created minesweeper field: %dx%d cells, %d mines\n", field->rows, field->cols, field->mine_count);

    return field;
}



/**
 * Resets a game field taking care not to place a mine at (row, col). 
 * This is useful for a new game's first move.
 */
void minesweeper_field_reset(MINESWEEPER_FIELD *field, int row, int col, bool reset_flags) {
    memset(field->cells, 0, field->rows * field->cols * sizeof(bool));
    memset(field->hints, 0, field->rows * field->cols * sizeof(int));
    memset(field->state, 0, field->rows * field->cols * sizeof(bool));
    if (reset_flags)
        memset(field->flags, 0, field->rows * field->cols * sizeof(int));

    int mine_count = 0;
    bool (*cells)[field->cols] = (bool (*)[])field->cells;
    int (*hints)[field->cols] = (int (*)[])field->hints;
    float ratio = MINESWEEPER_MIN_RATIO + ((field->rows * field->cols - 100) / 480.) * (MINESWEEPER_MAX_RATIO - MINESWEEPER_MIN_RATIO);
    ratio = ratio > MINESWEEPER_MAX_RATIO ? MINESWEEPER_MAX_RATIO : ratio;
    int mines = field->rows * field->cols * ratio;
    field->mine_count = mines;
    printf("Field reset: %dx%d cells, %d mines (%f mine ratio)", field->rows, field->cols, field->mine_count, ratio);

    while (mine_count < mines) {
        int candidate_row = rand() % field->rows;
        int candidate_column = rand() % field->cols;
        if (cells[candidate_row][candidate_column] || abs(row - candidate_row) < 1 || abs(col - candidate_column) < 1) continue;
        cells[candidate_row][candidate_column] = true;
        mine_count++;
    }

// Four nested for() loops, it's ugly, I know :(
    for (int row = 0; row < field->rows; row++)
        for (int col = 0; col < field->cols; col++) {
            int trow = row - 1, tcol = col - 1;
            for (int j = trow; j < trow + 3; j++) {
                if (j < 0 || j >= field->rows) continue;
                for (int i = tcol; i < tcol + 3; i++) {
                    if (i < 0 || i >= field->cols) continue;
                    if (cells[j][i]) hints[row][col]++;
                }
            }
        }
}



void minesweeper_field_destroy(MINESWEEPER_FIELD *field) {
    free(field->cells);
    free(field->hints);
    free(field->state);
    free(field->flags);
    free(field);
}



void minesweeper_field_uncover(MINESWEEPER_FIELD *field, int row, int col) {
    bool (*cells)[field->cols] = (bool (*)[])field->cells;
    int (*hints)[field->cols] = (int (*)[])field->hints;
    bool (*state)[field->cols] = (bool (*)[])field->state;
    int (*flags)[field->cols] = (int (*)[])field->flags;

    if (row < 0 || row >= field->rows) return;
    if (col < 0 || col >= field->cols) return;

    for (int j = row - 1; j <= row + 1; j++) {
        if (j < 0 || j >= field->rows) continue;
        for (int i = col - 1; i <= col + 1; i++) {
            if (i < 0 || i >= field->cols || state[j][i] || flags[j][i]) continue;
            if (!cells[j][i] && !flags[j][i]) {
                state[j][i] = true;
                field->cell_count--;
            }
            if (hints[j][i] == 0)
                minesweeper_field_uncover(field, j, i);
        }
    }
}



/**
 * Uncovers the cell defined by \c row and \c col and its neighboring cells, if applicable.
 *
 * @return \c true on success, \c false if a mine was found
 */
bool minesweeper_event_uncover(MINESWEEPER_FIELD *field, int row, int col) {
    bool (*cells)[field->cols] = (bool (*)[])field->cells;
    int (*hints)[field->cols] = (int (*)[])field->hints;
    bool (*state)[field->cols] = (bool (*)[])field->state;
    int (*flags)[field->cols] = (int (*)[])field->flags;

    if (row < 0 || row >= field->rows) return true;
    if (col < 0 || col >= field->cols) return true;
    if (flags[row][col] != 0 || state[row][col]) return true;
    if (cells[row][col]) return false;     // You lose
    state[row][col] = true;
    field->cell_count--;
    if (hints[row][col] == 0)
        minesweeper_field_uncover(field, row, col);
    if (field->cell_count <= field->mine_count)
        field->complete = true;
    field->move_count++;

    return true;
}



/**
 * Toggles the flags in the cell defined by \c row and \c col.
 */
void minesweeper_event_flag(MINESWEEPER_FIELD *field, int row, int col) {
    int (*flags)[field->cols] = (int (*)[])field->flags;
    bool (*state)[field->cols] = (bool (*)[])field->state;

    if (row < 0 || row >= field->rows) return;
    if (col < 0 || col >= field->cols) return;
    if (!state[row][col]) {
        flags[row][col] = (flags[row][col] + 1) % 3;
        if (flags[row][col] == MINESWEEPER_DANGER)
            field->flags_count++;
        else if (flags[row][col] == MINESWEEPER_WARNING)
            field->flags_count--;
    }
}


