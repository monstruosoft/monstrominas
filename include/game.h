/**
 * @file game.h
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
 * This is an early implementation of isolated game logic by monstruosoft. 
 * This filedefines basic typedefs and function prototypes.
 */

// Game mode flags to be applied by select_game_mode()
#define GM_DISABLE_KEYBOARD_INPUT   1
#define GM_DISABLE_MOUSE_INPUT      2
#define GM_ENABLE_KEYBOARD_INPUT    4
#define GM_ENABLE_MOUSE_INPUT       8



enum {GM_GAME, GM_WIN};     // Game modes



typedef struct GAME_ACTOR {
    int x;
    int y;
    int age;
    int state;
    bool active;
    bool visible;
    void *data;
    void (*control) ();
    void (*logic) ();
    void (*draw) ();
    void (*print) ();
    void (*destroy)();
} GAME_ACTOR;



void select_game_mode(int mode, int flags);

// void game_logic(ALLEGRO_EVENT *event);
// void game_update();

// void win_logic(ALLEGRO_EVENT *event);
// void win_update();



GAME_ACTOR *game_actor_create();  // Create a dummy game actor instance
void game_actor_print(GAME_ACTOR *actor);
void game_actor_draw(GAME_ACTOR *actor);
void game_actor_logic(GAME_ACTOR *actor, ALLEGRO_EVENT *event);
void game_actor_destroy(GAME_ACTOR *actor);

