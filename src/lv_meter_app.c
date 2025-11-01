/*
 * Einfache LVGL-Meter-Demo.
 * Kompiliere mit -DLV_USE_SDL=1 und -lSDL2 um ein Fenster zu erhalten:
 * gcc src/lv_meter_app.c -o lv_meter_app -I./src -I./src/lvgl -DLV_CONF_INCLUDE_SIMPLE -DLV_USE_SDL=1 -lSDL2 -pthread
 *
 * Hinweis: lv_conf.h liegt in ./src und wird dank LV_CONF_INCLUDE_SIMPLE gefunden.
 */

#ifndef LV_CONF_INCLUDE_SIMPLE
#define LV_CONF_INCLUDE_SIMPLE
#endif

/* Falls nicht in lv_conf.h gesetzt, sicherstellen, dass Meter-Widget aktiviert ist */
#ifndef LV_USE_WIDGETS
#define LV_USE_WIDGETS 1
#endif
#ifndef LV_USE_METER
#define LV_USE_METER 1
#endif

#if __has_include(<SDL2/SDL.h>) && defined(LV_USE_SDL)
#include <SDL2/SDL.h>
#endif

#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> /* usleep */

#define DISP_HOR_RES 320
#define DISP_VER_RES 240

static lv_obj_t *meter;
static lv_meter_indicator_t *meter_indic = NULL;

#if defined(LV_USE_SDL) && __has_include(<SDL2/SDL.h>)
/* Minimaler SDL-Backend (für Demo). Geht davon aus, dass LV_COLOR_DEPTH == 32 */
static SDL_Window *sdl_window = NULL;
static SDL_Renderer *sdl_renderer = NULL;
static SDL_Texture *sdl_texture = NULL;
static lv_disp_draw_buf_t disp_buf;
static lv_color_t *buf1 = NULL;
static lv_disp_t *disp;

static void sdl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    /* Wir aktualisieren ganze Textur - einfacher Ansatz für Demo */
    (void)drv;
    SDL_UpdateTexture(sdl_texture, NULL, buf1, DISP_HOR_RES * sizeof(lv_color_t));
    SDL_RenderClear(sdl_renderer);
    SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);
    lv_disp_flush_ready(drv);
}

static void sdl_init(void)
{
    SDL_Init(SDL_INIT_VIDEO);
    sdl_window = SDL_CreateWindow("LVGL Meter Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  DISP_HOR_RES, DISP_VER_RES, 0);
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING, DISP_HOR_RES, DISP_VER_RES);

    /* Draw buffer */
    buf1 = malloc(DISP_HOR_RES * DISP_VER_RES * sizeof(lv_color_t));
    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, DISP_HOR_RES * DISP_VER_RES);

    /* Register display driver */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    disp_drv.flush_cb = sdl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp = lv_disp_drv_register(&disp_drv);
}
#endif /* LV_USE_SDL */

/* Create a simple meter centered on screen */
static void create_meter(void)
{
    meter = lv_meter_create(lv_scr_act());
    lv_obj_center(meter);
    lv_obj_set_size(meter, 200, 200);

    lv_meter_scale_t *scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_range(meter, scale, 0, 100, 270, 90);
    lv_meter_set_scale_ticks(meter, scale, 11, 2, 10, lv_color_hex(0x999999));
    lv_meter_set_scale_major_ticks(meter, scale, 1, 4, 15, lv_color_hex(0x333333));

    /* Indicator */
    meter_indic = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_RED), -10);
}

int main(void)
{
    lv_init();

#if defined(LV_USE_SDL) && __has_include(<SDL2/SDL.h>)
    sdl_init();
#else
    /* Kein Display-Backend registriert -> Hinweis */
    fprintf(stderr, "Kein Display-Backend registriert. Definiere LV_USE_SDL und linke gegen SDL2\n");
    fprintf(stderr, "Die Meter-Widgets werden dennoch angelegt, aber nicht sichtbar.\n");
#endif

    create_meter();

    /* Demo: zyklische Wertänderung */
    int value = 0;
    int dir = 1;

    while (1) {
        /* einfache Animation */
        value += dir * 1;
        if (value >= 100) { value = 100; dir = -1; }
        if (value <= 0)   { value = 0;   dir = 1; }

        if (meter_indic) {
            lv_meter_set_indicator_value(meter, meter_indic, value);
        }

        lv_timer_handler(); /* LVGL-Ticks/Tasks */
        usleep(16 * 1000); /* ~60 FPS */

#if defined(LV_USE_SDL) && __has_include(<SDL2/SDL.h>)
        /* SDL Ereignisse verarbeiten (Fenster schließen erlaubt) */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                goto cleanup;
            }
        }
#endif
    }

cleanup:
#if defined(LV_USE_SDL) && __has_include(<SDL2/SDL.h>)
    if (buf1) free(buf1);
    if (sdl_texture) SDL_DestroyTexture(sdl_texture);
    if (sdl_renderer) SDL_DestroyRenderer(sdl_renderer);
    if (sdl_window) SDL_DestroyWindow(sdl_window);
    SDL_Quit();
#endif

    return 0;
}