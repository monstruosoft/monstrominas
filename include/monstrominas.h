/**
 * @file monstrominas.h
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
 * Typedefs and function prototypes for monstrominas minesweeper.
 */

#define MINESWEEPER_ROWS        10
#define MINESWEEPER_COLUMNS     10
#define MINESWEEPER_MINES       85
#define MINESWEEPER_CELL_SIZE   20
#define MINESWEEPER_DANGER       1
#define MINESWEEPER_WARNING      2
#define MINESWEEPER_MIN_RATIO    0.1
#define MINESWEEPER_MAX_RATIO    0.2



typedef struct MINESWEEPER_FIELD {
// Core fields
    bool *cells;
    int *hints;
    bool *state;
    int *flags;
    int rows;
    int cols;
    int cell_count;
    int cell_size;
    int mine_count;
    int flags_count;
    bool complete;
    int move_count;
} MINESWEEPER_FIELD;



void minesweeper_field_print(MINESWEEPER_FIELD *field);
MINESWEEPER_FIELD *minesweeper_field_create(int rows, int cols);
void minesweeper_field_destroy(MINESWEEPER_FIELD *field);
void minesweeper_field_uncover(MINESWEEPER_FIELD *field, int row, int col);
bool minesweeper_event_uncover(MINESWEEPER_FIELD *field, int row, int col);
void minesweeper_event_flag(MINESWEEPER_FIELD *field, int row, int col);

