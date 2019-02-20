/**
 * @file support.c
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
 * Supporting bitmap functions; namely, functions to blur an Allegro 5 bitmap.
 */

#include <allegro5/allegro.h>
#include <math.h>



static inline int utils_clampi(int x, int a, int b) {
    return x < a ? a : (x > b ? b : x);
}



/**
 *  Blurs and transposes a bitmap
 *
 *  Blurs and transposes the bitmap passed as a parameter. This filter is 
 *  useful since calling this function twice produces a box blur effect.
 */
ALLEGRO_BITMAP *bmputils_transpose_blur(ALLEGRO_BITMAP *bmp, int radius) {
    ALLEGRO_BITMAP *target = NULL, *blur = NULL;
    int *ioffset, *ooffset;
    int kr, kg, kb, ka;
    int i, x, y, w, h, t;
    int irgb, orgb;
    int in, out;
    int factor = radius * 2  + 1;
    ALLEGRO_LOCKED_REGION *ls, *ld;

    if (!bmp) return NULL;

    w = al_get_bitmap_width(bmp);
    h = al_get_bitmap_height(bmp);
    blur = al_create_bitmap(h, w);              // bitmap creation
    target = al_get_target_bitmap();            // save the current target bitmap
    al_set_target_bitmap(blur);                 // set the target bitmap to the bitmap that will hold the blurred output image
    al_clear_to_color(al_map_rgba(0, 0, 0, 0)); // clear to full transparency

    ls = al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_RGBA_8888, ALLEGRO_LOCK_READONLY);
    ld = al_lock_bitmap(blur, ALLEGRO_PIXEL_FORMAT_RGBA_8888, ALLEGRO_LOCK_WRITEONLY);

// Do a 1D horizontal blur and transpose the resulting image
    in = 0;
    for (y = 0; y < h; y++) {
        out = y;
        kr = 0; kg = 0; kb = 0; ka = 0;

    // Get the initial horizontal kernel sum
        ioffset = (int *)(ls->data + y * ls->pitch);
        ooffset = (int *)(ld->data);
        for (i = -radius; i <= radius; i++) {
            irgb = *(ioffset + utils_clampi(i, 0, w - 1));
            kr += (irgb >> 24) & 0xFF;
            kg += (irgb >> 16) & 0xFF;
            kb += (irgb >> 8) & 0xFF;
            ka += irgb & 0xFF;
        }
        
        t = 0;
        for (x = 0; x < w; x++) {
            int i1, i2, rgb1, rgb2;
            
            orgb = ((kr / factor) << 24) | ((kg / factor) << 16) | ((kb / factor) << 8) | (ka / factor);
            *(ooffset + out) = orgb;
            
            i1 = (x + radius + 1 > w - 1) ? w - 1 : x + radius + 1;
            i2 = (x - radius < 0) ? 0 : x - radius;
            rgb1 = *(ioffset + i1);
            rgb2 = *(ioffset + i2);
            kr += ((rgb1 >> 24) & 0xff) - ((rgb2 >> 24) & 0xff);
            kg += ((rgb1 & 0xff0000) - (rgb2 & 0xff0000)) >> 16;
            kb += ((rgb1 & 0xff00) - (rgb2 & 0xff00)) >> 8;
            ka += (rgb1 & 0xff) - (rgb2 & 0xff);
            
            ooffset = (int *)(ld->data + ++t * ld->pitch);
        }
    }

    al_unlock_bitmap(blur);
    al_unlock_bitmap(bmp);
    al_set_target_bitmap(target);               // restore the target bitmap

    return blur;
}



ALLEGRO_BITMAP *bmputils_box_blur(ALLEGRO_BITMAP *bmp, int radius) {
    ALLEGRO_BITMAP *blur = NULL, *temp = NULL;

    if (!bmp) return NULL;
    temp = bmputils_transpose_blur(bmp, radius);
    if (!temp) return NULL;
    blur = bmputils_transpose_blur(temp, radius);
    al_destroy_bitmap(temp);
    return blur;
}

