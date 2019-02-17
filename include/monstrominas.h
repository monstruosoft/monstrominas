#define MINESWEEPER_ROWS        30
#define MINESWEEPER_COLUMNS     16
#define MINESWEEPER_MINES       85
#define MINESWEEPER_CELL_SIZE   20
#define MINESWEEPER_WARNING      1
#define MINESWEEPER_DANGER       2
#define MINESWEEPER_MIN_RATIO    0.1
#define MINESWEEPER_MAX_RATIO    0.2



typedef struct MINESWEEPER_FIELD {
    bool *cells;
    int *hints;
    bool *state;
    int *flags;
    int rows;
    int cols;
    int cell_count;
    int mine_count;
    bool complete;
} MINESWEEPER_FIELD;



void minesweeper_field_print(MINESWEEPER_FIELD *field);
MINESWEEPER_FIELD *minesweeper_field_create(int rows, int cols);
void minesweeper_field_uncover(MINESWEEPER_FIELD *field, int row, int col);
bool minesweeper_event_uncover(MINESWEEPER_FIELD *field, int row, int col);
void minesweeper_event_flag(MINESWEEPER_FIELD *field, int row, int col);

