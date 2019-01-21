/**
 * @file main.c
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
 * Game loop.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include "game.h"



#define MINESWEEPER_ROWS        16
#define MINESWEEPER_COLUMNS     10
#define MINESWEEPER_MINES       10
#define MINESWEEPER_CELL_SIZE   32
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



// Allegro global variables
ALLEGRO_EVENT_QUEUE *events = NULL;
ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_FONT *font = NULL;
ALLEGRO_EVENT event;

bool game_over = false;
bool redraw = true;

// Game global variables
GAME_ACTOR *actor = NULL;



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



void minesweeper_field_draw(MINESWEEPER_FIELD *field) {
    ALLEGRO_COLOR color = al_color_name("lightgray");
    ALLEGRO_COLOR black = al_color_name("black");

    for (int row = 0; row < MINESWEEPER_ROWS; row++)
        for (int col = 0; col < MINESWEEPER_COLUMNS; col++) {
            int x1 = col * MINESWEEPER_CELL_SIZE, y1 = row * MINESWEEPER_CELL_SIZE;
            int x2 = x1 + MINESWEEPER_CELL_SIZE, y2 = y1 + MINESWEEPER_CELL_SIZE;
            al_draw_filled_rectangle(x1, y1, x2, y2, al_color_name("lightgray"));
            if (!field->state[row][col]) {
                al_draw_filled_rectangle(x1, y1, x2, y2, al_color_name("darkgray"));
                al_draw_rectangle(x1, y1, x2, y2, black, 2);
            }
            if (field->hints[row][col] != 0 && field->state[row][col])
                al_draw_textf(font, black, x1, y1, 0, "%d", field->hints[row][col]);

            if (field->flags[row][col] != 0)
                al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgba(255, 92 * field->flags[row][col], 0, 255));
            if (field->complete)
                al_draw_text(font, black, 50, 50, 0, "WIN!!!!1");
        }
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



void minesweeper_field_logic(MINESWEEPER_FIELD *field, ALLEGRO_EVENT *event) {
    if (event->any.source == al_get_mouse_event_source()) {
        int row = event->mouse.y / MINESWEEPER_CELL_SIZE;
        int col = event->mouse.x / MINESWEEPER_CELL_SIZE;
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == 1) {
            if (field->flags[row][col] != 0 || field->state[row][col]) return;
            if (field->cells[row][col]) game_over = true;
            field->state[row][col] = true;
            field->cell_count--;
            if (field->hints[row][col] == 0)
                minesweeper_field_uncover(field, row, col);
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == 2) {
            if (!field->state[row][col])
                field->flags[row][col] = (field->flags[row][col] + 1) % 3;
        }
    }
    if (field->cell_count <= MINESWEEPER_MINES)
        field->complete = true;
}



GAME_ACTOR *minesweeper_field_actor() {
    GAME_ACTOR *actor = game_actor_create();
    actor->data = minesweeper_field_create();
    actor->print = minesweeper_field_print;
    actor->draw = minesweeper_field_draw;
    actor->logic = minesweeper_field_logic;
    return actor;
}



GAME_ACTOR *game_actor_create() {
    GAME_ACTOR *actor = calloc(sizeof(GAME_ACTOR), 1);
    assert(actor);
    actor->active = true;
    actor->visible = true;
    // All other fields are set to NULL by calloc()
}

void game_actor_print(GAME_ACTOR *actor) {
    actor->print(actor->data);
}

void game_actor_draw(GAME_ACTOR *actor) {
    actor->draw(actor->data);
}

void game_actor_logic(GAME_ACTOR *actor, ALLEGRO_EVENT *event) {
    actor->logic(actor->data, event);
}



/*
 * Game logic.
 */
void logic(ALLEGRO_EVENT *event) {
    if (event->type == ALLEGRO_EVENT_TIMER) {
        redraw = true;
    }
    else {
        game_actor_logic(actor, event);
        if (event->any.source == al_get_mouse_event_source() && event->mouse.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
            game_actor_print(actor);
    }
//    else if (event->any.source == al_get_keyboard_event_source()) {
//        if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
//            if (event->keyboard.keycode == ALLEGRO_KEY_DOWN)
//                game->inputs |= MONSTRO_TINPUT_DOWN;
//            if (event->keyboard.keycode == ALLEGRO_KEY_LEFT)
//                game->inputs |= MONSTRO_TINPUT_LEFT;
//            if (event->keyboard.keycode == ALLEGRO_KEY_RIGHT)
//                game->inputs |= MONSTRO_TINPUT_RIGHT;
//            if (event->keyboard.keycode == ALLEGRO_KEY_Z || event->keyboard.keycode == ALLEGRO_KEY_SPACE)
//                game->inputs |= MONSTRO_TINPUT_ROTATE_LEFT;
//            if (event->keyboard.keycode == ALLEGRO_KEY_X)
//                game->inputs |= MONSTRO_TINPUT_ROTATE_RIGHT;
//        }
//        if (event->type == ALLEGRO_EVENT_KEY_UP) {
//            if (event->keyboard.keycode == ALLEGRO_KEY_DOWN)
//                game->inputs &= ~MONSTRO_TINPUT_DOWN;
//            if (event->keyboard.keycode == ALLEGRO_KEY_LEFT)
//                game->inputs &= ~MONSTRO_TINPUT_LEFT;
//            if (event->keyboard.keycode == ALLEGRO_KEY_RIGHT)
//                game->inputs &= ~MONSTRO_TINPUT_RIGHT;
//            if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
//                game_over = true;
//        }
//    }
}



/*
 * Screen update.
 */
void update() {
    al_clear_to_color(al_map_rgb(64, 64, 128));
    game_actor_draw(actor);
}


/*
 * Game initialization.
 */
void initialization() {
// Allegro initialization
    assert(al_init());
    assert(al_install_keyboard());
    assert(al_install_mouse());
    al_set_new_display_flags(ALLEGRO_WINDOWED);
    al_set_new_window_title("my monstruosoft game");
    display = al_create_display(800, 600);
    assert(display);
    assert(al_init_primitives_addon());
    assert(al_init_font_addon());
    assert(al_init_ttf_addon());

    font = al_load_ttf_font("data/ZillaSlab-Regular.otf", 32, 0);
    events = al_create_event_queue();
    assert(events);
    timer = al_create_timer(ALLEGRO_BPS_TO_SECS(30));
    assert(timer);
    al_start_timer(timer);
    al_register_event_source(events, al_get_keyboard_event_source());
    al_register_event_source(events, al_get_mouse_event_source());
    al_register_event_source(events, al_get_timer_event_source(timer));
    
    srand(time(NULL));

// Game initialization
    actor = minesweeper_field_actor();
    game_actor_print(actor);
}



/*
 * Game loop.
 */
int main() {
    initialization();
    
    while (!game_over) {
        al_wait_for_event(events, &event);
        logic(&event);
        if (redraw && al_is_event_queue_empty(events)) {
            update();
            al_flip_display();
            redraw = false;
        }  
    }
}

