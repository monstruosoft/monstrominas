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

    for (int row = 0; row < MINESWEEPER_ROWS; row++) {
        for (int col = 0; col < MINESWEEPER_COLUMNS; col++)
            printf("%d", cells[row][col] ? 1 : 0);
        printf("\t");
        for (int col = 0; col < MINESWEEPER_COLUMNS; col++)
            printf("%d", hints[row][col]);
        printf("\t");
        for (int col = 0; col < MINESWEEPER_COLUMNS; col++)
            printf("%d", state[row][col] ? 0 : 1);
        printf("\n");
    }
    printf("\n");
}



MINESWEEPER_FIELD *minesweeper_field_create(int rows, int cols) {
    MINESWEEPER_FIELD *field = calloc(sizeof(MINESWEEPER_FIELD), 1);
    assert(field);
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

    while (mine_count < MINESWEEPER_MINES) {
        int row = rand() % MINESWEEPER_ROWS;
        int column = rand() % MINESWEEPER_COLUMNS;
        if (cells[row][column]) continue;
        cells[row][column] = true;
        mine_count++;
    }

// Four nested for() loops, it's ugly, I know :(
    for (int row = 0; row < MINESWEEPER_ROWS; row++)
        for (int col = 0; col < MINESWEEPER_COLUMNS; col++) {
            int trow = row - 1, tcol = col - 1;
            for (int j = trow; j < trow + 3; j++) {
                if (j < 0 || j >= MINESWEEPER_ROWS) continue;
                for (int i = tcol; i < tcol + 3; i++) {
                    if (i < 0 || i >= MINESWEEPER_COLUMNS) continue;
                    if (cells[j][i]) hints[row][col]++;
                }
            }
        }

    field->cell_count = MINESWEEPER_ROWS * MINESWEEPER_COLUMNS;

    return field;
}



void minesweeper_field_uncover(MINESWEEPER_FIELD *field, int row, int col) {
    bool (*cells)[field->cols] = (bool (*)[])field->cells;
    int (*hints)[field->cols] = (int (*)[])field->hints;
    bool (*state)[field->cols] = (bool (*)[])field->state;

    for (int j = row - 1; j <= row + 1; j++) {
        if (j < 0 || j >= MINESWEEPER_ROWS) continue;
        for (int i = col - 1; i <= col + 1; i++) {
            if (i < 0 || i >= MINESWEEPER_COLUMNS || state[j][i]) continue;
            if (!cells[j][i]) {
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

    if (flags[row][col] != 0 || state[row][col]) return true;
    if (cells[row][col]) return false;     // You lose
    state[row][col] = true;
    field->cell_count--;
    if (hints[row][col] == 0)
        minesweeper_field_uncover(field, row, col);
    if (field->cell_count <= MINESWEEPER_MINES)
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


