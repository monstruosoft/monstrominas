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
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#ifdef __ANDROID__
#include <allegro5/allegro_android.h>
#endif
#include "game.h"
#include "monstrominas.h"



#define SCR_WIDTH           800
#define SCR_HEIGHT          600
#define MAX_ROWS             36
#define MAX_COLS             50
#define MAX_BACKGROUNDS      10



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
ALLEGRO_PATH *bg[MAX_BACKGROUNDS] = {0};
ALLEGRO_BITMAP *background = NULL;



void minesweeper_field_draw(MINESWEEPER_FIELD *field) {
    ALLEGRO_COLOR color = al_color_name("lightgray");
    ALLEGRO_COLOR black = al_color_name("black");
    ALLEGRO_COLOR white = al_color_name("white");
    int font_height = al_get_font_line_height(font);

    for (int row = 0; row < field->rows; row++)
        for (int col = 0; col < field->cols; col++) {
            int x1 = field->x_offset + col * field->cell_size, y1 = field->y_offset + row * field->cell_size;
            int x2 = x1 + field->cell_size, y2 = y1 + field->cell_size;
            if (!((bool (*)[field->cols])field->state)[row][col]) {
                al_draw_filled_rectangle(x1, y1, x2, y2, al_color_name("darkgray"));
                al_draw_rectangle(x1, y1, x2 - 1, y2 - 1, black, 1);
                al_draw_line(x1, y1, x2, y1, white, 1);
                al_draw_line(x1, y1, x1, y2, white, 1);
            }
            else {
                // al_draw_filled_rectangle(x1, y1, x2, y2, al_color_name("lightgray"));
                al_draw_rectangle(x1, y1, x2, y2, al_map_rgba(64, 64, 64, 128), 1);
            }
            if (((int (*)[field->cols])field->hints)[row][col] != 0 && ((bool (*)[field->cols])field->state)[row][col])
                al_draw_textf(font, black, x1 + field->cols / 2, y1 + (field->cell_size - font_height), ALLEGRO_ALIGN_CENTER, "%d", ((int (*)[field->cols])field->hints)[row][col]);

            if (((int (*)[field->cols])field->flags)[row][col] != 0)
                al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgba(255, 92 * ((int (*)[field->cols])field->flags)[row][col], 0, 255));
            if (field->complete)
                al_draw_text(font, black, 50, 50, 0, "WIN!!!!1");
        }
}



void minesweeper_field_logic(MINESWEEPER_FIELD *field, ALLEGRO_EVENT *event) {
    if (event->any.source == al_get_mouse_event_source()) {
        int row = (event->mouse.y - field->y_offset) / field->cell_size;
        int col = (event->mouse.x - field->x_offset) / field->cell_size;
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == 1) {
            game_over = !minesweeper_event_uncover(field, row, col);
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event->mouse.button == 2) {
            minesweeper_event_flag(field, row, col);
        }
    }
}



GAME_ACTOR *minesweeper_field_actor(int rows, int cols) {
    GAME_ACTOR *actor = game_actor_create();
    actor->data = minesweeper_field_create(rows, cols);
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
    al_clear_to_color(al_map_rgb(0, 128, 128));
    MINESWEEPER_FIELD *field = actor->data;
    int w = al_get_bitmap_width(background), 
        h = al_get_bitmap_height(background);
    float scalex = SCR_WIDTH * 1.0 / w,
          scaley = SCR_HEIGHT * 1.0 / h;
    int w2 = field->cell_size * field->cols, 
        h2 = field->cell_size * field->rows,
        sx = w / 2 - w2 / scalex / 2,                        // Bitmap region
        sy = h / 2 - h2 / scaley / 2;
    al_draw_tinted_scaled_rotated_bitmap_region(background, 0, 0, w, h, al_map_rgba(32, 32, 32, 32), 0, 0, 0, 0, scalex, scaley, 0, 0);
    al_draw_filled_rectangle(field->x_offset, field->y_offset, field->x_offset + w2, field->y_offset + h2, al_map_rgb(255, 255, 255));
    // al_draw_tinted_scaled_bitmap(background, al_map_rgba(32, 32, 32, 32), 0, 0, w, h, field->x_offset, field->y_offset, w, h, 0);
    al_draw_tinted_scaled_rotated_bitmap_region(background, sx, sy, w2 / scalex, h2 / scaley, al_map_rgba(128, 128, 128, 128), 0, 0, SCR_WIDTH / 2 - w2 / 2, SCR_HEIGHT / 2 - h2 / 2, scalex, scaley, 0, 0);
    game_actor_draw(actor);
}


/*
 * Game initialization.
 */
void initialization(int rows, int cols) {
// Allegro initialization
    assert(al_init());
    assert(al_install_keyboard());
    assert(al_install_mouse());
    al_set_new_display_flags(ALLEGRO_WINDOWED);
    al_set_new_window_title("Monstrominas by monstruosoft");
    display = al_create_display(SCR_WIDTH, SCR_HEIGHT);
    assert(display);
    assert(al_init_primitives_addon());
    assert(al_init_image_addon());
    assert(al_init_font_addon());
    assert(al_init_ttf_addon());

#ifdef __ANDROID__
    printf("Android version: %s", al_android_get_os_version());
    al_android_set_apk_file_interface();
#endif
    font = al_load_ttf_font("data/ZillaSlab-Bold.ttf", MINESWEEPER_CELL_SIZE, 0);
    assert(font);
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
    rows = rows <= MAX_ROWS ? rows : MAX_ROWS;
    cols = cols <= MAX_COLS ? cols : MAX_COLS;
    actor = minesweeper_field_actor(rows, cols);
    game_actor_print(actor);
    MINESWEEPER_FIELD *field = actor->data;
    int x_size = field->cols * field->cell_size;
    int y_size = field->rows * field->cell_size;
    field->x_offset = SCR_WIDTH / 2 - x_size / 2;
    field->y_offset = SCR_HEIGHT / 2 - y_size / 2;

// Find potential background images
    int count = 0;
    ALLEGRO_FS_ENTRY *dir = al_create_fs_entry("data");
    assert(al_fs_entry_exists(dir));
    assert(al_open_directory(dir));
    ALLEGRO_FS_ENTRY *file = al_read_directory(dir);
    printf("Available background images: \n");
    while (file != NULL && count < MAX_BACKGROUNDS) {
        ALLEGRO_PATH *path = al_create_path(al_get_fs_entry_name(file));
        assert(path);
        if (strcmp(al_get_path_extension(path), ".jpg") == 0) {
            printf("file: %s\n", al_get_fs_entry_name(file));
            bg[count++] = path;
        }
        else al_destroy_path(path);
        file = al_read_directory(dir);
    }
    assert(al_close_directory(dir));

// Randomly choose background images from those available
    if (count > 0) {
        int choice = rand() % count;
        printf("Choosing background %d: %s\n", choice, al_path_cstr(bg[choice], '/'));
        background = al_load_bitmap(al_path_cstr(bg[choice], '/'));
        assert(background);
    }
}



/*
 * Game loop.
 */
int main(int argc, char **argv) {
// Parse command line arguments
    int rows = MINESWEEPER_ROWS, cols = MINESWEEPER_COLUMNS;
    if (argc > 2) {
        rows = strtol(argv[2], NULL, 10);
        cols = strtol(argv[1], NULL, 10);
    }
    initialization(rows, cols);
    
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

