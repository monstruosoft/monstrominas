#define MINESWEEPER_ROWS        30
#define MINESWEEPER_COLUMNS     16
#define MINESWEEPER_MINES       85
#define MINESWEEPER_CELL_SIZE   20
#define MINESWEEPER_WARNING      1
#define MINESWEEPER_DANGER       2



typedef struct MINESWEEPER_FIELD {
    bool cells[MINESWEEPER_ROWS][MINESWEEPER_COLUMNS];
    int hints[MINESWEEPER_ROWS][MINESWEEPER_COLUMNS];
    bool state[MINESWEEPER_ROWS][MINESWEEPER_COLUMNS];
    int flags[MINESWEEPER_ROWS][MINESWEEPER_COLUMNS];
    int cell_count;
    bool complete;
} MINESWEEPER_FIELD;



void minesweeper_field_print(MINESWEEPER_FIELD *field);
MINESWEEPER_FIELD *minesweeper_field_create();
void minesweeper_field_uncover(MINESWEEPER_FIELD *field, int row, int col);
bool minesweeper_event_uncover(MINESWEEPER_FIELD *field, int row, int col);
void minesweeper_event_flag(MINESWEEPER_FIELD *field, int row, int col);

