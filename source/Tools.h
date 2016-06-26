#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <3ds.h>
#include <citro3d.h>
#include <string.h>
#include <stdio.h>

#include "Pixels.h"
#include "Bitmap.h"
#include "Drawing.h"
#include "Perlin.h"
#include "VectorLibrary/Vector.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#define SCREEN_TEXTURE_WIDTH 512
#define SCREEN_TEXTURE_HEIGHT 512

#define CLEAR_COLOR 0x68B0D8FF

#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define TEXTURE_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define DISPLAY_TOBOT_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GSP_RGBA8_OES) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
    
extern int RandomInteger();
extern int nnnoise(int x, int y, int oct);
extern float badFresnel(float input, float expo);
extern void fullscreenQuad(C3D_Tex texture, float iod, float iodmult);

extern int32_t mulf32(int32_t a, int32_t b);
extern int32_t divf32(int32_t a, int32_t b);

typedef struct { float position[3]; float texcoord[2]; float normal[3]; } vertex;

inline void setVert(vertex* vert, vec3_t p, vec2_t t) {
    vert->position[0] = p.x;
    vert->position[1] = p.y;
    vert->position[2] = p.z;
    vert->texcoord[0] = t.x;
    vert->texcoord[1] = t.y;
}

#endif