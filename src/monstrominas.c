#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "monstrominas.h"



void minesweeper_field_print(MINESWEEPER_FIELD *field) {
    for (int row = 0; row < MINESWEEPER_ROWS; row++) {
        for (int col = 0; col < MINESWEEPER_COLUMNS; col++)
            printf("%d", field->cells[row][col] ? 1 : 0);
        printf("\t");
        for (int col = 0; col < MINESWEEPER_COLUMNS; col++)
            printf("%d", field->hints[row][col]);
        printf("\t");
        for (int col = 0; col < MINESWEEPER_COLUMNS; col++)
            printf("%d", field->state[row][col] ? 0 : 1);
        printf("\n");
    }
    printf("\n");
}



MINESWEEPER_FIELD *minesweeper_field_create() {
    MINESWEEPER_FIELD *field = calloc(sizeof(MINESWEEPER_FIELD), 1);
    assert(field);
    int mine_count = 0;
    while (mine_count < MINESWEEPER_MINES) {
        int row = rand() % MINESWEEPER_ROWS;
        int column = rand() % MINESWEEPER_COLUMNS;
        if (field->cells[row][column]) continue;
        field->cells[row][column] = true;
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
                    if (field->cells[j][i]) field->hints[row][col]++;
                }
            }
        }

    field->cell_count = MINESWEEPER_ROWS * MINESWEEPER_COLUMNS;

    return field;
}



void minesweeper_field_uncover(MINESWEEPER_FIELD *field, int row, int col) {
    for (int j = row - 1; j <= row + 1; j++) {
        if (j < 0 || j >= MINESWEEPER_ROWS) continue;
        for (int i = col - 1; i <= col + 1; i++) {
            if (i < 0 || i >= MINESWEEPER_COLUMNS || field->state[j][i]) continue;
            if (!field->cells[j][i]) {
                field->state[j][i] = true;
                field->cell_count--;
            }
            if (field->hints[j][i] == 0)
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
    if (field->flags[row][col] != 0 || field->state[row][col]) return true;
    if (field->cells[row][col]) return false;     // You lose
    field->state[row][col] = true;
    field->cell_count--;
    if (field->hints[row][col] == 0)
        minesweeper_field_uncover(field, row, col);
    if (field->cell_count <= MINESWEEPER_MINES)
        field->complete = true;

    return true;
}



/**
 * Toggles the flags in the cell defined by \c row and \c col.
 */
void minesweeper_event_flag(MINESWEEPER_FIELD *field, int row, int col) {
    if (!field->state[row][col])
        field->flags[row][col] = (field->flags[row][col] + 1) % 3;  
}


