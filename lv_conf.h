/**
 * @file lv_conf.h
 * Configuration file for v8.3.9
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888) */
#define LV_COLOR_DEPTH 16

/* Maximal horizontal and vertical resolution to support by the library */
#define LV_HOR_RES_MAX          320
#define LV_VER_RES_MAX          170

/* Use a custom tick source that tells the elapsed time in milliseconds */
#define LV_TICK_CUSTOM     1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE  "Arduino.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

/* Memory to use for internal graphics buffers */
#define LV_MEM_CUSTOM      1
#if LV_MEM_CUSTOM == 0
    /* Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB) */
    #define LV_MEM_SIZE    (32U * 1024U)
#else
    #define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
    #define LV_MEM_CUSTOM_ALLOC   malloc
    #define LV_MEM_CUSTOM_FREE    free
    #define LV_MEM_CUSTOM_REALLOC realloc
#endif

/* 1: use custom malloc/free, 0: use the built-in `lv_mem_alloc()` and `lv_mem_free()` */
#define LV_MEMCPY_MEMSET_STD    1

/* Required for widgets */
#define LV_USE_LABEL        1
#define LV_LABEL_TEXT_SELECTION 0
#define LV_USE_BAR          1

/* Fonts */
#define LV_FONT_MONTSERRAT_12    0
#define LV_FONT_MONTSERRAT_14    1
#define LV_FONT_MONTSERRAT_16    0
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Enable drawing */
#define LV_USE_DRAW_SW  1
#if LV_USE_DRAW_SW
    #define LV_DRAW_SW_GRADIENT     1
    #define LV_DRAW_SW_COMPLEX      1
#endif

/* HAL settings */
#define LV_USE_GPU_ESP32    1
#define LV_USE_PERF_MONITOR 0

/* Feature usage */
#define LV_USE_ANIMATION        1
#define LV_USE_SHADOW          1
#define LV_USE_GROUP           1
#define LV_USE_FILESYSTEM      0
#define LV_USE_USER_DATA       0
#define LV_USE_THEME_DEFAULT   1

#endif /*LV_CONF_H*/ 