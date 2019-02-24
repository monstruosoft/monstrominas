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
    assert(field->cells && field->hints && field->state && field->flags);

    int mine_count = 0;
    bool (*cells)[field->cols] = (bool (*)[])field->cells;
    int (*hints)[field->cols] = (int (*)[])field->hints;
    float ratio = MINESWEEPER_MIN_RATIO + ((field->rows * field->cols - 100) / 480.) * (MINESWEEPER_MAX_RATIO - MINESWEEPER_MIN_RATIO);
    ratio = ratio > MINESWEEPER_MAX_RATIO ? MINESWEEPER_MAX_RATIO : ratio;
    int mines = field->rows * field->cols * ratio;

    while (mine_count < mines) {
        int row = rand() % field->rows;
        int column = rand() % field->cols;
        if (cells[row][column]) continue;
        cells[row][column] = true;
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

    field->cell_count = field->rows * field->cols;
    field->mine_count = mines;
    field->cell_size = MINESWEEPER_CELL_SIZE;
    printf("Created minesweeper field: %dx%d cells, %d mines (%f mine ratio)\n", field->rows, field->cols, field->mine_count, ratio);

    return field;
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

    if (row < 0 || row >= field->rows) return;
    if (col < 0 || col >= field->cols) return;
    if (flags[row][col] != 0 || state[row][col]) return true;
    if (cells[row][col]) return false;     // You lose
    state[row][col] = true;
    field->cell_count--;
    if (hints[row][col] == 0)
        minesweeper_field_uncover(field, row, col);
    if (field->cell_count <= field->mine_count)
        field->complete = true;

    return true;
}



/**
 * Toggles the flags in the cell defined by \c row and \c col.
 */
void minesweeper_event_flag(MINESWEEPER_FIELD *field, int row, int col) {
    int (*flags)[field->cols] = (int (*)[])field->flags;
    bool (*state)[field->cols] = (bool (*)[])field->state;
    if (!state[row][col])
        flags[row][col] = (flags[row][col] + 1) % 3;  
}


