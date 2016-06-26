#ifndef __DS3D_H__
#define __DS3D_H__

#ifndef ARM9
#error 3D hardware is only available from the ARM9
#endif

#include <nds/ndstypes.h>
#include <nds/arm9/video.h>
#include <nds/dma.h>
#include <nds/memory.h>
#include <nds/arm9/math.h>
#include <nds/arm9/trig_lut.h>
#include <nds/arm9/cache.h>

/*#define DS_LUT_SIZE 512
#define DS_LUT_MASK 0x1FF

static inline int32_t isin(int a) { return SIN[a&DS_LUT_MASK]; }
static inline int32_t icos(int a) { return COS[a&DS_LUT_MASK]; }
static inline int32_t itan(int a) { return TAN[a&DS_LUT_MASK]; }
static inline int32_t iangle(int a) { return a*DS_LUT_SIZE/360.0; }
*/
#define DS_LUT_SIZE 512

static inline int32_t isin(int a) { return sinFixed(a); }
static inline int32_t icos(int a) { return cosFixed(a); }
static inline int32_t itan(int a) { return tanFixed(a); }
static inline int32_t iangle(int a) { return a*DS_LUT_SIZE/360.0; }

#define DSPack16(a,b) ((a&0xffff)|((b)<<16))
#define DSPack10(a,b,c) (((a)&0x3ff)|(((b)&0x3ff)<<10)|(((c)&0x3ff)<<20))
#define DSPackRGB5(r,g,b) ((r)|((g)<<5)|((b)<<10))
#define DSPackRGB8(r,g,b) (((r)>>3)|(((g)>>3)<<5)|(((b)>>3)<<10))

// f32: Matrix-math 20.12 fixed-point format
#define DSintiof32(n) ((n)<<12)
#define DSfloatto32(n) ((int32_t)((n)*(1<<12)))
#define DSf32toint(n) ((n)>>12)
#define DSf32tofloat(n) (((float)(n))/(float)(1<<12))

#define DSf32(n) ((int32_t)((n)*(1<<12))) // shorthand

// v16: Vertex coordinate 4.12 fixed-point format
#define DSf32tov16(n) ((int16_t)(n))
#define DSinttov16(n) ((int16_t)((n)<<12))
#define DSfloattov16(n) ((int16_t)((n)*(1<<12)))
#define DSv16toint(n) ((n)>>12)

// v10: Vertex coordinate 4.6 fixed-point format
#define DSf32tov10(n) ((int16_t)((n)>>6))
#define DSinttov10(n) ((int16_t)((n)<<6))
#define DSfloattov10(n) ((int16_t)((n)*(1<<6)))
#define DSv10toint(n) ((n)>>6)

// t16: Texture coordinate 12.4 fixed-point format
#define DSf32tot16(n) ((int16_t)(n>>8))
#define DSinttot16(n) ((int16_t)((n)<<4))
#define DSfloattot16(n) ((int16_t)((n)*(1 << 4)))
#define DSt16toint(n) ((n)>>4)

// n10: Normal and light direction 1.9 fixed-point format
#define DSf32ton10(n) ((int16_t)((n)>>3))
#define DSintton10(n) ((int16_t)((n)<<9))
#define DSfloatton10(n) (((n)>.998)?0x1FF:((int16_t)((n)*(1<<9))))
#define DSn10toint(n) ((n)>>9)

// d15: Depth buffer 12.3 fixed-point format
#define DSinttod15(n) ((int16_t)((n)<<3))
#define DSfloattod15(n) ((uint16_t)((n)*(1<<3)))
#define DS_MAX_DEPTH 0x7fff

typedef struct { int32_t m[9]; } Matrix3x3;
typedef struct { int32_t m[16]; } Matrix4x4;
typedef struct { int32_t m[12]; } Matrix4x3;
typedef struct { int32_t x,y,z; } Vector;


// http://nocash.emubase.de/gbatek.htm#ds3dpolygondefinitionsbyvertices
#define DS_TRIANGLES 0
#define DS_QUADS 1
#define DS_TRIANGLE_STRIP 2
#define DS_QUAD_STRIP 3

// http://nocash.emubase.de/gbatek.htm#ds3dmatrixloadmultiply
#define DS_PROJECTION 0
#define DS_POSITION 1
#define DS_MODELVIEW 2
#define DS_TEXTURE 3



// http://nocash.emubase.de/gbatek.htm#ds3dpolygonattributes
#define DS_POLY_LIGHT0 0x0001
#define DS_POLY_LIGHT1 0x0002
#define DS_POLY_LIGHT2 0x0004
#define DS_POLY_LIGHT3 0x0008
#define DS_POLY_MODE_MODULATION 0x0000
#define DS_POLY_MODE_DECAL 0x0010
#define DS_POLY_MODE_TOON 0x0020
#define DS_POLY_MODE_SHADOW 0x0030
#define DS_POLY_CULL_ALL 0x0000
#define DS_POLY_CULL_FRONT 0x0040
#define DS_POLY_CULL_BACK 0x0080
#define DS_POLY_CULL_NONE 0x00c0
#define DS_POLY_SET_TRANS_DEPTH 0x0800
#define DS_POLY_CLIP_FAR 0x1000
#define DS_POLY_RENDER_1DOT 0x2000
#define DS_POLY_DEPTH_TEST_LESS 0x0000
#define DS_POLY_DEPTH_TEST_EQUAL 0x4000
#define DS_POLY_FOG 0x8000
#define DS_POLY_ALPHA(n) (((n)&0x1f)<<16)
#define DS_POLY_ID(n) (((n)&0x3f)<<24)

// http://nocash.emubase.de/gbatek.htm#ds3dtextureattributes
#define DS_TEX_ADDRESS(addr) ((((uint32_t)(addr))>>3)&0xffff)
#define DS_TEX_WRAP_S 0x10000
#define DS_TEX_WRAP_T 0x20000
#define DS_TEX_FLIP_S 0x40000
#define DS_TEX_FLIP_T 0x80000
#define DS_TEX_SIZE_S(n) ((n)<<20)
#define DS_TEX_SIZE_S_8 (0<<20)
#define DS_TEX_SIZE_S_16 (1<<20)
#define DS_TEX_SIZE_S_32 (2<<20)
#define DS_TEX_SIZE_S_64 (3<<20)
#define DS_TEX_SIZE_S_128 (4<<20)
#define DS_TEX_SIZE_S_256 (5<<20)
#define DS_TEX_SIZE_S_512 (6<<20)
#define DS_TEX_SIZE_S_1024 (7<<20)
#define DS_TEX_SIZE_T(n) ((n)<<23)
#define DS_TEX_SIZE_T_8 (0<<23)
#define DS_TEX_SIZE_T_16 (1<<23)
#define DS_TEX_SIZE_T_32 (2<<23)
#define DS_TEX_SIZE_T_64 (3<<23)
#define DS_TEX_SIZE_T_128 (4<<23)
#define DS_TEX_SIZE_T_256 (5<<23)
#define DS_TEX_SIZE_T_512 (6<<23)
#define DS_TEX_SIZE_T_1024 (7<<23)
#define DS_TEX_FORMAT_NONE (0<<26)
#define DS_TEX_FORMAT_A3P5 (1<<26)
#define DS_TEX_FORMAT_PAL2 (2<<26)
#define DS_TEX_FORMAT_PAL4 (3<<26)
#define DS_TEX_FORMAT_PAL8 (4<<26)
#define DS_TEX_FORMAT_COMPRESSED (5<<26)
#define DS_TEX_FORMAT_A5P3 (6<<26)
#define DS_TEX_FORMAT_RGB (7<<26)
#define DS_TEX_FORMAT_MASK (7<<26)
#define DS_TEX_COLOR0_TRANS 0x20000000
#define DS_TEX_GEN_OFF 0x00000000
#define DS_TEX_GEN_TEXCOORD 0x40000000
#define DS_TEX_GEN_NORMAL 0x80000000
#define DS_TEX_GEN_POSITION 0xc0000000
#define DS_INVALID_TEXTURE 0xffffffff

//http://nocash.emubase.de/gbatek.htm#ds3ddisplaycontrol
#define DS_TEXTURING 0x01
#define DS_TOON_SHADING 0x02
#define DS_ALPHA_TEST 0x04
#define DS_ALPHA_BLEND 0x08
#define DS_ANTIALIAS 0x10
#define DS_OUTLINE 0x20
#define DS_FOG_ALPHA_ONLY 0x40
#define DS_FOG 0x80
#define DS_FOG_SHIFT(n) ((n)<<8)
#define DS_COLOR_UNDERFLOW 0x1000
#define DS_POLY_OVERFLOW 0x2000
#define DS_CLEAR_BITMAP 0x4000

// http://nocash.emubase.de/gbatek.htm#ds3ddisplaycontrol
#define DS_FLUSH_NO_SORTING 0x01
#define DS_FLUSH_WBUFFERING 0x02

/*#define FIFO_COMMAND_PACK(c1,c2,c3,c4) (((c4)<<24)|((c3)<<16)|((c2)<<8)|(c1))

#define REG2ID(r) (uint8_t)((((uint32_t)(&(r)))-0x04000400)>>2)

#define FIFO_NOP REG2ID(GFX_FIFO)
#define FIFO_STATUS REG2ID(GFX_STATUS)
#define FIFO_COLOR REG2ID(GFX_COLOR)

#define FIFO_VERTEX16 REG2ID(GFX_VERTEX16)
#define FIFO_TEX_COORD REG2ID(GFX_TEX_COORD)
#define FIFO_TEX_FORMAT REG2ID(GFX_TEX_FORMAT)
#define FIFO_PAL_FORMAT REG2ID(GFX_PAL_FORMAT)

#define FIFO_CLEAR_COLOR REG2ID(GFX_CLEAR_COLOR)
#define FIFO_CLEAR_DEPTH REG2ID(GFX_CLEAR_DEPTH)

#define FIFO_LIGHT_VECTOR REG2ID(GFX_LIGHT_VECTOR)
#define FIFO_LIGHT_COLOR REG2ID(GFX_LIGHT_COLOR)
#define FIFO_NORMAL REG2ID(GFX_NORMAL)

#define FIFO_DIFFUSE_AMBIENT REG2ID(GFX_DIFFUSE_AMBIENT)
#define FIFO_SPECULAR_EMISSION REG2ID(GFX_SPECULAR_EMISSION)
#define FIFO_SHININESS REG2ID(GFX_SHININESS)

#define FIFO_POLY_FORMAT REG2ID(GFX_POLY_FORMAT)

#define FIFO_BEGIN REG2ID(GFX_BEGIN)
#define FIFO_END REG2ID(GFX_END)
#define FIFO_FLUSH REG2ID(GFX_FLUSH)
#define FIFO_VIEWPORT REG2ID(GFX_VIEWPORT)*/

#ifdef __cplusplus
extern "C" {
#endif

void DSInit3D();
void DSRotatef32i(int angle,int32_t x,int32_t y,int32_t z);

uint32_t DSTextureSize(uint32_t flags);
uint16_t *DSTextureAddress(uint32_t texture);

uint32_t DSAllocTexture(uint32_t flags);
void DSFreeAllTextures();
void DSCopyTexture(uint32_t texture,void *data);
uint32_t DSAllocAndCopyTexture(uint32_t flags,void *data);

void DSCopyColorTexture(uint32_t texture,uint32_t color);
uint32_t DSMakeColorTexture(uint32_t color);
uint32_t DSMakeWhiteTexture();

void DSSetFogWithCallback(uint8_t r,uint8_t g,uint8_t b,uint8_t a,int32_t start,int32_t end,int32_t near,
int32_t far,int32_t (*callback)(int32_t z,int32_t start,int32_t end));
void DSSetFogLinearf32(uint8_t r,uint8_t g,uint8_t b,uint8_t a,int32_t start,int32_t end,int32_t near,int32_t far);
void DSSetFogLinearf(uint8_t r,uint8_t g,uint8_t b,uint8_t a,float start,float end,float near,float far);

#ifdef __cplusplus
}
#endif



// http://nocash.emubase.de/gbatek.htm#ds3ddisplaycontrol
static inline void DSViewport(uint8_t left, uint8_t bottom, uint8_t right, uint8_t top) { GFX_VIEWPORT=(left)|(bottom<<8)|(right<<16)|(top<<24); }
static inline void DSFlush(uint32_t mode) { GFX_FLUSH=mode; }

static inline void DSSetControl(int control) { GFX_CONTROL=control; }

static inline void DSAlphaThreshold(int alphathreshold) { GFX_ALPHA_TEST=alphathreshold; }
static inline void DSCutoffDepth(uint16_t wval) { GFX_CUTOFF_DEPTH=wval; }

static inline void DSClearParams(uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint8_t id)
{ GFX_CLEAR_COLOR=((id&0x3f)<<24)|((a&0x1f)<<16)|DSPackRGB5(r,g,b); }

//http://nocash.emubase.de/gbatek.htm#ds3drearplane
static inline void DSClearDepth(uint16_t depth) { GFX_CLEAR_DEPTH=depth; }
static inline void DSMaxClearDepth() { GFX_CLEAR_DEPTH=DS_MAX_DEPTH; }

// http://nocash.emubase.de/gbatek.htm#ds3dpolygonattributes
static inline void DSPolyFormat(uint32_t format) { GFX_POLY_FORMAT=format; }

// http://nocash.emubase.de/gbatek.htm#ds3dtextureattributes
static inline void DSSetTexture(uint32_t texture) { while(GFX_STATUS&(1<<27)); GFX_TEX_FORMAT=texture; }
static inline void DSSetPaletteOffset(uint32_t offs,uint32_t format)
{ GFX_PAL_FORMAT=(offs>>((format&DS_TEX_FORMAT_MASK)==DS_TEX_FORMAT_PAL2?3:4))&0x1fff; }
static inline void DSSetPalette(void *ptr,uint32_t format) { DSSetPaletteOffset((uint32_t)ptr,format); }


// http://nocash.emubase.de/gbatek.htm#ds3dpolygondefinitionsbyvertices
static inline void DSBegin(int mode) { GFX_BEGIN=mode; }
static inline void DSEnd() { GFX_END=0; }

static inline void DSVertex(uint32_t xy,uint32_t z) { GFX_VERTEX16=xy; GFX_VERTEX16=z; }
static inline void DSVertex3v16(uint16_t x,uint16_t y,uint16_t z) { DSVertex(DSPack16(x,y),z); }
static inline void DSVertex3f(float x,float y,float z) { DSVertex3v16(DSfloattov16(x),DSfloattov16(y),DSfloattov16(z)); }
static inline void DSVertex3v(const Vector v) { DSVertex3v16(v.x,v.y,v.z); }

static inline void DSVertexXY(uint32_t xy) { GFX_VERTEX_XY=xy; }
static inline void DSVertexXYv16(uint16_t x,uint16_t y) { DSVertexXY(DSPack16(x,y)); }
static inline void DSVertexXYf(float x,float y) { DSVertexXYv16(DSfloattov16(x),DSfloattov16(y)); }
static inline void DSVertex2f(float x,float y) { DSVertexXYf(x,y); }

static inline void DSVertexXZ(uint32_t xz) { GFX_VERTEX_XZ=xz; }
static inline void DSVertexXZv16(uint16_t x,uint16_t z) { DSVertexXZ(DSPack16(x,z)); }
static inline void DSVertexXZf(float x,float z) { DSVertexXZv16(DSfloattov16(x),DSfloattov16(z)); }

static inline void DSVertexYZ(uint32_t yz) { GFX_VERTEX_YZ=yz; }
static inline void DSVertexYZv16(uint16_t y,uint16_t z) { DSVertexYZ(DSPack16(y,z)); }
static inline void DSVertexYZf(float y,float z) { DSVertexYZv16(DSfloattov16(y),DSfloattov16(z)); }

static inline void DSVertex10(uint32_t xyz) { GFX_VERTEX10=xyz; }
static inline void DSVertex103f(float x,float y,float z) { DSVertex10(DSPack10(DSfloattov16(x),DSfloattov16(y),DSfloattov16(z))); }

static inline void DSVertexDiff(uint32_t xyz) { GFX_VERTEX_DIFF=xyz; }
static inline void DSVertexDiff3n10(uint16_t x,uint16_t y,uint16_t z) { DSVertexDiff(DSPack10(x,y,z)); }
static inline void DSVertexDiff3f(float x,float y,float z) { DSVertexDiff3n10(DSfloatton10(x*8),DSfloatton10(y*8),DSfloatton10(z*8)); }

// http://nocash.emubase.de/gbatek.htm#ds3dpolygonattributes
static inline void DSColor(uint16_t color) { GFX_COLOR=color; }
static inline void DSColor3b(uint8_t r,uint8_t g,uint8_t b) { DSColor(DSPackRGB5(r,g,b)); }
static inline void DSColor3f(float r,float g, float b) { DSColor3b(r*31,g*31,b*31); }

// http://nocash.emubase.de/gbatek.htm#ds3dpolygonlightparameters
static inline void DSNormal(uint32_t normal) { GFX_NORMAL=normal; }
static inline void DSNormal3n10(int16_t x,int16_t y,int16_t z) { DSNormal(DSPack10(x,y,z)); }
static inline void DSNormal3f(float x,float y,float z) { DSNormal3n10(DSfloatton10(x),DSfloatton10(y),DSfloatton10(z)); }

// http://nocash.emubase.de/gbatek.htm#ds3dtextureattributes
static inline void DSTexCoord(uint32_t coords) { GFX_TEX_COORD=coords; }
static inline void DSTexCoord2t16(uint16_t u,uint16_t v) { DSTexCoord(DSPack16(u,v)); }
static inline void DSTexCoord2f(float u,float v) { DSTexCoord2t16(DSfloattot16(u),DSfloattot16(v)); }




// http://nocash.emubase.de/gbatek.htm#ds3dmatrixstack
static inline void DSPushMatrix() { MATRIX_PUSH=0; }
static inline void DSPopMatrix(int num) { MATRIX_POP=num; }
static inline void DSRestoreMatrix(int index) { MATRIX_RESTORE=index; }
static inline void DSStoreMatrix(int index) { MATRIX_STORE=index; }

// http://nocash.emubase.de/gbatek.htm#ds3dmatrixloadmultiply
static inline void DSMatrixMode(int mode) { MATRIX_CONTROL=mode; }
static inline void DSLoadIdentity() { MATRIX_IDENTITY=0; }

static inline void DSLoadMatrix4x4(const Matrix4x4 m)
{
	MATRIX_LOAD4x4=m.m[0]; MATRIX_LOAD4x4=m.m[1]; MATRIX_LOAD4x4=m.m[2]; MATRIX_LOAD4x4=m.m[3];
	MATRIX_LOAD4x4=m.m[4]; MATRIX_LOAD4x4=m.m[5]; MATRIX_LOAD4x4=m.m[6]; MATRIX_LOAD4x4=m.m[7];
	MATRIX_LOAD4x4=m.m[8]; MATRIX_LOAD4x4=m.m[9]; MATRIX_LOAD4x4=m.m[10]; MATRIX_LOAD4x4=m.m[11];
	MATRIX_LOAD4x4=m.m[12]; MATRIX_LOAD4x4=m.m[13]; MATRIX_LOAD4x4=m.m[14]; MATRIX_LOAD4x4=m.m[15];
}

static inline void DSLoadMatrix4x3(const Matrix4x3 m)
{
	MATRIX_LOAD4x3=m.m[0]; MATRIX_LOAD4x3=m.m[1]; MATRIX_LOAD4x3=m.m[2];
	MATRIX_LOAD4x3=m.m[3]; MATRIX_LOAD4x3=m.m[4]; MATRIX_LOAD4x3=m.m[5];
	MATRIX_LOAD4x3=m.m[6]; MATRIX_LOAD4x3=m.m[7]; MATRIX_LOAD4x3=m.m[8];
	MATRIX_LOAD4x3=m.m[9]; MATRIX_LOAD4x3=m.m[10]; MATRIX_LOAD4x3=m.m[11];
}

static inline void DSMultMatrix4x4(const Matrix4x4 m)
{
	MATRIX_MULT4x4=m.m[0]; MATRIX_MULT4x4=m.m[1]; MATRIX_MULT4x4=m.m[2]; MATRIX_MULT4x4=m.m[3];
	MATRIX_MULT4x4=m.m[4]; MATRIX_MULT4x4=m.m[5]; MATRIX_MULT4x4=m.m[6]; MATRIX_MULT4x4=m.m[7];
	MATRIX_MULT4x4=m.m[8]; MATRIX_MULT4x4=m.m[9]; MATRIX_MULT4x4=m.m[10]; MATRIX_MULT4x4=m.m[11];
	MATRIX_MULT4x4=m.m[12]; MATRIX_MULT4x4=m.m[13]; MATRIX_MULT4x4=m.m[14]; MATRIX_MULT4x4=m.m[15];
}

static inline void DSMULTMatrix4x3(const Matrix4x3 m)
{
	MATRIX_MULT4x3=m.m[0]; MATRIX_MULT4x3=m.m[1]; MATRIX_MULT4x3=m.m[2];
	MATRIX_MULT4x3=m.m[3]; MATRIX_MULT4x3=m.m[4]; MATRIX_MULT4x3=m.m[5];
	MATRIX_MULT4x3=m.m[6]; MATRIX_MULT4x3=m.m[7]; MATRIX_MULT4x3=m.m[8];
	MATRIX_MULT4x3=m.m[9]; MATRIX_MULT4x3=m.m[10]; MATRIX_MULT4x3=m.m[11];
}

static inline void DSMultMatrix3x3(const Matrix3x3 m)
{
	MATRIX_MULT3x3=m.m[0]; MATRIX_MULT3x3=m.m[1]; MATRIX_MULT3x3=m.m[2];
	MATRIX_MULT3x3=m.m[3]; MATRIX_MULT3x3=m.m[4]; MATRIX_MULT3x3=m.m[5];
	MATRIX_MULT3x3=m.m[6]; MATRIX_MULT3x3=m.m[7]; MATRIX_MULT3x3=m.m[8];
}

static inline void DSScalef32(int32_t x,int32_t y,int32_t z)
{
	MATRIX_SCALE=x;
	MATRIX_SCALE=y;
	MATRIX_SCALE=z;
}
static inline void DSScalev(const Vector v) { DSScalef32(v.x,v.y,v.z); }
static inline void DSScalef(float x,float y,float z) { DSScalef32(DSf32(x),DSf32(y),DSf32(z)); }
static inline void DSScaleUniformf32(int32_t factor) { DSScalef32(factor,factor,factor); }
static inline void DSScaleUniformf(float factor) { DSScaleUniformf32(DSf32(factor)); }

static inline void DSTranslatef32(int32_t x,int32_t y,int32_t z)
{
	MATRIX_TRANSLATE=x;
	MATRIX_TRANSLATE=y;
	MATRIX_TRANSLATE=z;
}
static inline void DSTranslatev(const Vector v) { DSTranslatef32(v.x,v.y,v.z); }
static inline void DSTranslatef(float x,float y,float z) { DSTranslatef32(DSf32(x),DSf32(y),DSf32(z)); }

static inline void DSRotateXi(int angle)
{
	int32_t sine=isin(angle);
	int32_t cosine=icos(angle);
	
	MATRIX_MULT3x3=DSf32(1); MATRIX_MULT3x3=0; MATRIX_MULT3x3=0;
	MATRIX_MULT3x3=0; MATRIX_MULT3x3=cosine; MATRIX_MULT3x3=sine;
	MATRIX_MULT3x3=0; MATRIX_MULT3x3=-sine; MATRIX_MULT3x3=cosine;
}

static inline void DSRotateYi(int angle)
{
	int32_t sine=isin(angle);
	int32_t cosine=icos(angle);
	
	MATRIX_MULT3x3=cosine; MATRIX_MULT3x3=0; MATRIX_MULT3x3=-sine;
	MATRIX_MULT3x3=0; MATRIX_MULT3x3=DSf32(1); MATRIX_MULT3x3=0;
	MATRIX_MULT3x3=sine; MATRIX_MULT3x3=0; MATRIX_MULT3x3=cosine;
}

static inline void DSRotateZi(int angle)
{
	int32_t sine=isin(angle);
	int32_t cosine=icos(angle);
	
	MATRIX_MULT3x3=cosine; MATRIX_MULT3x3=sine; MATRIX_MULT3x3=0;
	MATRIX_MULT3x3=-sine; MATRIX_MULT3x3=cosine; MATRIX_MULT3x3=0;
	MATRIX_MULT3x3=0; MATRIX_MULT3x3=0; MATRIX_MULT3x3=DSf32(1);
}

static inline void DSRotateXf(float angle) { DSRotateXi(iangle(angle)); }
static inline void DSRotateYf(float angle) { DSRotateYi(iangle(angle)); }
static inline void DSRotateZf(float angle) { DSRotateZi(iangle(angle)); }
static inline void DSRotatef32(float angle,int32_t x,int32_t y,int32_t z) { DSRotatef32i(iangle(angle),x,y,z); }
static inline void DSRotatef(float angle,float x,float y,float z) { DSRotatef32(angle,DSf32(x),DSf32(y),DSf32(z)); }

static inline void DSOrthof32(int32_t left,int32_t right,int32_t bottom,int32_t top,int32_t near,int32_t far)
{
	MATRIX_MULT4x4=divf32(DSf32(2),right-left); MATRIX_MULT4x4=0; MATRIX_MULT4x4=0; MATRIX_MULT4x4=0;
	MATRIX_MULT4x4=0; MATRIX_MULT4x4=divf32(DSf32(2),top-bottom); MATRIX_MULT4x4=0; MATRIX_MULT4x4=0;
	MATRIX_MULT4x4=0; MATRIX_MULT4x4=0; MATRIX_MULT4x4=divf32(DSf32(-2),far-near); MATRIX_MULT4x4=0;

	MATRIX_MULT4x4=-divf32(right+left,right-left);  
	MATRIX_MULT4x4=-divf32(top+bottom,top-bottom);  
	MATRIX_MULT4x4=-divf32(far+near,far-near);  
	MATRIX_MULT4x4=DSf32(1);
}
static inline void DSOrtho(float left,float right,float bottom,float top,float near, float far)
{ DSOrthof32(DSf32(left),DSf32(right),DSf32(bottom),DSf32(top),DSf32(near),DSf32(far)); }

static inline void DS2DProjection(int fractionbits)
{
	DSOrthof32(0,(256<<fractionbits),(192<<fractionbits),0,DSf32(-1),DSf32(1));
}

static inline void DSLookAtf32(int32_t eyex,int32_t eyey,int32_t eyez,int32_t lookatx,int32_t lookaty,
int32_t lookatz,int32_t upx,int32_t upy,int32_t upz)
{
	int32_t side[3],forward[3],up[3],eye[3];

	forward[0]=eyex-lookatx; forward[1]=eyey-lookaty; forward[2]=eyez-lookatz;
	up[0]=upx; up[1]=upy; up[2]=upz;
	eye[0]=eyex; eye[1]=eyey; eye[2]=eyez;
	
	normalizef32(forward);
	crossf32(up,forward,side);
	normalizef32(side);
	crossf32(forward,side,up);

	MATRIX_MULT4x3=side[0]; MATRIX_MULT4x3=up[0]; MATRIX_MULT4x3=forward[0];
	MATRIX_MULT4x3=side[1]; MATRIX_MULT4x3=up[1]; MATRIX_MULT4x3=forward[1];
	MATRIX_MULT4x3=side[2]; MATRIX_MULT4x3=up[2]; MATRIX_MULT4x3=forward[2]; 
	MATRIX_MULT4x3=-dotf32(eye,side);
	MATRIX_MULT4x3=-dotf32(eye,up);
	MATRIX_MULT4x3=-dotf32(eye,forward);
}
static inline void DSLookAt(float eyex,float eyey,float eyez,float lookatx,float lookaty,
float lookatz,float upx,float upy,float upz)
{ DSLookAtf32(DSf32(eyex),DSf32(eyey),DSf32(eyez),DSf32(lookatx),
DSf32(lookaty),DSf32(lookatz),DSf32(upx),DSf32(upy),DSf32(upz)); }

static inline void DSFrustumf32(int32_t left,int32_t right,int32_t bottom,int32_t top,int32_t near,int32_t far)
{
	MATRIX_MULT4x4=divf32(2*near,right-left); MATRIX_MULT4x4=0; MATRIX_MULT4x4=0; MATRIX_MULT4x4=0;
 	MATRIX_MULT4x4=0; MATRIX_MULT4x4=divf32(2*near,top-bottom); MATRIX_MULT4x4=0; MATRIX_MULT4x4=0;
	MATRIX_MULT4x4=divf32(right + left, right - left);
	MATRIX_MULT4x4=divf32(top + bottom, top - bottom);
	MATRIX_MULT4x4=-divf32(far + near, far - near);
	MATRIX_MULT4x4=DSf32(-1);
	MATRIX_MULT4x4=0; MATRIX_MULT4x4=0; MATRIX_MULT4x4=-divf32(2*mulf32(far,near),far-near); MATRIX_MULT4x4=0;
}
static inline void DSFrustum(float left,float right,float bottom,float top,float near,float far) { DSFrustumf32(DSf32(left),DSf32(right),DSf32(bottom),DSf32(top),DSf32(near),DSf32(far)); }

static inline void DSPerspectivef32(int fovy,int32_t aspect,int32_t near,int32_t far)
{
	int32_t ymax=mulf32(near,itan(fovy>>1));
	int32_t ymin=-ymax;
	int32_t xmin=mulf32(ymin,aspect);
	int32_t xmax=mulf32(ymax,aspect);

	DSFrustumf32(xmin,xmax,ymin,ymax,near,far);
}
static inline void DSPerspective(float fovy,float aspect,float near,float far) { DSPerspectivef32(iangle(fovy),DSf32(aspect),DSf32(near),DSf32(far)); }

/* Utility function which generates a picking matrix for selection
 x 2D x of center  (touch x normally)
 y 2D y of center  (touch y normally)
 width width in pixels of the window (3 or 4 is a good number)
 height height in pixels of the window (3 or 4 is a good number)
 viewport the current viewport (normaly {0, 0, 255, 191}) */
static inline void DSPickMatrix(int x,int y,int width,int height,const int viewport[4])
{
	MATRIX_MULT4x4=DSf32(viewport[2])/width; MATRIX_MULT4x4=0; MATRIX_MULT4x4=0; MATRIX_MULT4x4=0;
	MATRIX_MULT4x4=0; MATRIX_MULT4x4=DSf32(viewport[3]) / height; MATRIX_MULT4x4=0; MATRIX_MULT4x4=0;
	MATRIX_MULT4x4=0; MATRIX_MULT4x4=0; MATRIX_MULT4x4=DSf32(1); MATRIX_MULT4x4=0;
	MATRIX_MULT4x4=DSf32(viewport[2]+((viewport[0]-x)<<1))/width;
	MATRIX_MULT4x4=DSf32(viewport[3]+((viewport[1]-y)<<1))/height;
	MATRIX_MULT4x4=0;
	MATRIX_MULT4x4=DSf32(1);
}

static inline void DSResetMatrixStack()
{
	DSMatrixMode(DS_TEXTURE);
	DSLoadIdentity();

	// make sure there are no push/pops that haven't executed yet
	while(GFX_STATUS & BIT(14)) GFX_STATUS|=1<<15; // clear push/pop errors or push/pop busy bit never clears

	DSMatrixMode(DS_PROJECTION);
	// pop the projection stack to the top; poping 0 off an empty stack causes an error... weird?
	if((GFX_STATUS&(1<<13))!=0) DSPopMatrix(1);
	DSLoadIdentity();

	// 31 deep modelview matrix; 32nd entry works but sets error flag
	DSMatrixMode(DS_MODELVIEW);
	DSPopMatrix((GFX_STATUS>>8)&0x1F);
	DSLoadIdentity();
}



// http://nocash.emubase.de/gbatek.htm#ds3dpolygonlightparameters
static inline void DSLight(int n,uint16_t color,uint32_t dir)
{
	uint32_t nummask=(n&3)<<30;
	GFX_LIGHT_VECTOR=nummask|dir;
	GFX_LIGHT_COLOR=nummask|color;
}
static inline void DSLight3n10(int n,uint16_t color,int16_t x,int16_t y,int16_t z) { DSLight(n,color,DSPack10(x,y,z)); }
static inline void DSLight3f(int id,uint16_t color,float x,float y,float z) { DSLight3n10(id,color,DSfloatton10(x),DSfloatton10(y),DSfloatton10(z)); }
static inline void DSLight3b3f(int id,uint8_t r,uint8_t g,uint8_t b,float x,float y,float z) { DSLight3f(id,DSPackRGB5(r,g,b),x,y,z); }

static inline void DSMaterialDiffuseAndAmbient(uint16_t diffuse,uint16_t ambient) { GFX_DIFFUSE_AMBIENT=DSPack16(diffuse,ambient); }
static inline void DSMaterialDiffuseAndAmbient6b(uint8_t dr,uint8_t dg,uint8_t db,uint8_t ar,uint8_t ag,uint8_t ab) { DSMaterialDiffuseAndAmbient(DSPackRGB5(dr,dg,db),DSPackRGB5(ar,ag,ab)); }
static inline void DSMaterialDiffuseAndAmbient3b(uint8_t r,uint8_t g,uint8_t b) { DSMaterialDiffuseAndAmbient(DSPackRGB5(r,g,b),DSPackRGB5(r,g,b)); }
static inline void DSMaterialSpecularAndEmission(uint8_t shiny,uint16_t specular,uint16_t emission) { GFX_SPECULAR_EMISSION=DSPack16(specular,emission)|(shiny?0x8000:0); }
static inline void DSMaterialSpecularAndEmission6b(uint8_t shiny,uint8_t sr,uint8_t sg,uint8_t sb,uint8_t er,uint8_t eg,uint8_t eb) { DSMaterialSpecularAndEmission(shiny,DSPackRGB5(sr,sg,sb),DSPackRGB5(er,eg,eb)); }

static inline void DSMaterialShinyness()
{
	for(int i=0;i<32;i++) GFX_SHININESS=(i*8)|((i*8+2)<<8)|((i*8+4)<<8)|((i*8+6)<<8);
}

// http://nocash.emubase.de/gbatek.htm#ds3dtoonedgefog
static inline void DSSetOutlineColor(int id,uint16_t color) { GFX_EDGE_TABLE[id]=color; }
static inline void DSSetOutlineColor3b(int id,uint8_t r,uint8_t g,uint8_t b) { GFX_EDGE_TABLE[id]=DSPackRGB5(r,g,b); }

static inline void DSSetToonTable(const uint16_t *table) { for(int i=0;i<32;i++) GFX_TOON_TABLE[i]=table[i]; }
static inline void DSSetToonTableRange(int start,int end,uint16_t color) { for(int i=start;i<=end;i++) GFX_TOON_TABLE[i]=color; }
static inline void DSSetToonTableRange3b(int start,int end,uint8_t r,uint8_t g,uint8_t b) { DSSetToonTableRange(start,end,DSPackRGB5(r,g,b)); }






// http://nocash.emubase.de/gbatek.htm#ds3dgeometrycommands
static inline void DSCallList(const uint32_t *list)
{
	uint32_t count=*list++;
	
	// flush the area that we are going to DMA
	DC_FlushRange(list,count*4);
	
	// Don't start DMAing while anything else is being DMAed because FIFO DMA is touchy as hell
	while((DMA_CR(0)&DMA_BUSY)||(DMA_CR(1)&DMA_BUSY)||(DMA_CR(2)&DMA_BUSY)||(DMA_CR(3)&DMA_BUSY));
	
	// send the packed list asynchronously via DMA to the FIFO
	DMA_SRC(0)=(uint32_t)list;
	DMA_DEST(0)=0x4000400;
	DMA_CR(0)=DMA_FIFO|count;
	while(DMA_CR(0)&DMA_BUSY);
}







/*! \brief Grabs fixed format of state variables<BR>
OpenGL's modelview matrix is handled on the DS with two matrices. The combination of the DS's position matrix and directional vector matrix hold the data that is in OpenGL's one modelview matrix. (a.k.a. modelview = postion and vector)<BR>
<A HREF="http://nocash.emubase.de/gbatek.htm#ds3diomap">http://nocash.emubase.de/gbatek.htm#ds3diomap</A>
\param param The state variable to retrieve
\param f pointer with room to hold the requested data */
/*static inline void glGetFixed(const GL_GET_ENUM param, int32_t* f) {
	int i;
	switch (param) {
		case GL_GET_MATRIX_VECTOR:
			while(GFX_BUSY); // wait until the graphics engine has stopped to read matrixes
			for(i = 0; i < 9; i++) f[i] = MATRIX_READ_VECTOR[i];
			break;
		case GL_GET_MATRIX_CLIP:
			while(GFX_BUSY); // wait until the graphics engine has stopped to read matrixes
			for(i = 0; i < 16; i++) f[i] = MATRIX_READ_CLIP[i];
			break;
		case GL_GET_MATRIX_PROJECTION:
			glMatrixMode(GL_POSITION);
			glPushMatrix(); // save the current state of the position matrix
			glLoadIdentity(); // load an identity matrix into the position matrix so that the clip matrix = projection matrix
			while(GFX_BUSY); // wait until the graphics engine has stopped to read matrixes
				for(i = 0; i < 16; i++) f[i] = MATRIX_READ_CLIP[i]; // read out the projection matrix
			glPopMatrix(1); // restore the position matrix
			break;
		case GL_GET_MATRIX_POSITION:
			glMatrixMode(GL_PROJECTION);
			glPushMatrix(); // save the current state of the projection matrix
			glLoadIdentity(); // load a identity matrix into the projection matrix so that the clip matrix = position matrix
			while(GFX_BUSY); // wait until the graphics engine has stopped to read matrixes
				for(i = 0; i < 16; i++) f[i] = MATRIX_READ_CLIP[i]; // read out the position matrix
			glPopMatrix(1); // restore the projection matrix
			break;
		default: 
			break;
	}
}*/

/*! \brief Grabs integer state variables from openGL
\param param The state variable to retrieve
\param i pointer with room to hold the requested data */
/*static inline void glGetInt(GL_GET_ENUM param, int* i) {
	switch (param) {
		case GL_GET_POLYGON_RAM_COUNT:
			*i = GFX_POLYGON_RAM_USAGE;
			break;
		case GL_GET_VERTEX_RAM_COUNT:
			*i = GFX_VERTEX_RAM_USAGE;
			break;
		case GL_GET_TEXTURE_WIDTH:
			*i = 8 << (((glGlob->textures[glGlob->activeTexture]) >> 20) & 7);
			break;
		case GL_GET_TEXTURE_HEIGHT:
			*i = 8 << (((glGlob->textures[glGlob->activeTexture]) >> 23) & 7);
			break;
		default:
			break;
	}
}*/

#endif
