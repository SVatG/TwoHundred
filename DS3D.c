#include "DS3D.h"

#include <nds/bios.h>

static uint32_t next_texture_block;
static uint32_t next_palette_block;

void DSInit3D()
{
	while(GFX_STATUS&(1<<27)); // wait till gfx engine is not busy

	GFX_STATUS|=(1<<29); // Clear the FIFO
	DSResetMatrixStack(); // Clear overflows from list memory
	DSFlush(0); // prime the vertex/polygon buffers
	GFX_CONTROL=0; // reset the control bits
	DSClearParams(0,0,0,31,0); // reset the rear-plane(a.k.a. clear color) to black, ID=0, and opaque
	DSClearDepth(DS_MAX_DEPTH);// reset the depth to its max

	GFX_TEX_FORMAT=0;
	GFX_POLY_FORMAT=0;

	DSFreeAllTextures();
//	DSFreeAllPalettes();
}



void DSRotatef32i(int angle,int32_t x,int32_t y,int32_t z)
{
	int32_t axis[3];
	int32_t sine=isin(angle);
	int32_t cosine=icos(angle);
	int32_t one_minus_cosine=DSf32(1)-cosine;

	axis[0]=x;
	axis[1]=y;
	axis[2]=z;
	normalizef32(axis);

	MATRIX_MULT3x3=cosine + mulf32(one_minus_cosine, mulf32(axis[0], axis[0]));
	MATRIX_MULT3x3=mulf32(one_minus_cosine, mulf32(axis[0], axis[1])) - mulf32(axis[2], sine);
	MATRIX_MULT3x3=mulf32(mulf32(one_minus_cosine, axis[0]), axis[2]) + mulf32(axis[1], sine);

	MATRIX_MULT3x3=mulf32(mulf32(one_minus_cosine, axis[0]),  axis[1]) + mulf32(axis[2], sine);
	MATRIX_MULT3x3=cosine + mulf32(mulf32(one_minus_cosine, axis[1]), axis[1]);
	MATRIX_MULT3x3=mulf32(mulf32(one_minus_cosine, axis[1]), axis[2]) - mulf32(axis[0], sine);

	MATRIX_MULT3x3=mulf32(mulf32(one_minus_cosine, axis[0]), axis[2]) - mulf32(axis[1], sine);
	MATRIX_MULT3x3=mulf32(mulf32(one_minus_cosine, axis[1]), axis[2]) + mulf32(axis[0], sine);
	MATRIX_MULT3x3=cosine + mulf32(mulf32(one_minus_cosine, axis[2]), axis[2]);
}



/*
//---------------------------------------------------------------------------------
// glColorTable establishes the location of the current palette.
//	Roughly follows glColorTableEXT. Association of palettes with 
//	named textures is left to the application. 
//---------------------------------------------------------------------------------
void glColorTable( uint8_t format, uint32_t addr ) {
//---------------------------------------------------------------------------------
	GFX_PAL_FORMAT = addr>>(4-(format==GL_RGB4));
}
                     

//---------------------------------------------------------------------------------
inline uint32_t alignVal( uint32_t val, uint32_t to ) {
	return (val & (to-1))? (val & ~(to-1)) + to : val;
}

//---------------------------------------------------------------------------------
int getNextPaletteSlot(u16 count, uint8_t format) {
//---------------------------------------------------------------------------------
	// ensure the result aligns on a palette block for this format
	uint32_t result = alignVal(glGlob->nextPBlock, 1<<(4-(format==GL_RGB4)));

	// convert count to bytes and align to next (smallest format) palette block
	count = alignVal( count<<1, 1<<3 ); 

	// ensure that end is within palette video mem
	if( result+count > 0x10000 )   // VRAM_F - VRAM_E
		return -1;

	glGlob->nextPBlock = result+count;
	return (int)result;
} 

//---------------------------------------------------------------------------------
void glTexLoadPal(const u16* pal, u16 count, u32 addr) {
//---------------------------------------------------------------------------------
	vramSetBankE(VRAM_E_LCD);
	swiCopy( pal, &VRAM_E[addr>>1] , count / 2 | COPY_MODE_WORD);
	vramSetBankE(VRAM_E_TEX_PALETTE);
}

//---------------------------------------------------------------------------------
int gluTexLoadPal(const u16* pal, u16 count, uint8_t format) {
//---------------------------------------------------------------------------------
	int addr = getNextPaletteSlot(count, format);
	if( addr>=0 )
		glTexLoadPal(pal, count, (u32) addr);

	return addr;
}


*/



uint32_t DSTextureSize(uint32_t flags)
{
	uint32_t pixels=1<<(((flags>>20)&7)+((flags>>23)&7)+6);

	switch(flags&DS_TEX_FORMAT_MASK)
	{
		case DS_TEX_FORMAT_A3P5:
		case DS_TEX_FORMAT_PAL8:
		case DS_TEX_FORMAT_A5P3:
			return pixels;

		case DS_TEX_FORMAT_NONE:
		case DS_TEX_FORMAT_COMPRESSED:
			return 0;

		case DS_TEX_FORMAT_RGB:
			return pixels*2;

		case DS_TEX_FORMAT_PAL2:
			return pixels/4;

		case DS_TEX_FORMAT_PAL4:
			return pixels/2;
	}
	return 0;
}

uint16_t *DSTextureAddress(uint32_t texture)
{
	return (uint16_t *)(((texture&0xffff)<<3)|0x6800000);
}

static int VRAMAddressIsTextureBank(uint32_t addr)
{
	if((uint16_t *)addr<VRAM_A) return 0;
	else if((uint16_t *)addr<VRAM_B) return (VRAM_A_CR&3)==((VRAM_A_TEXTURE)&3);
	else if((uint16_t *)addr<VRAM_C) return (VRAM_B_CR&3)==((VRAM_B_TEXTURE)&3);
	else if((uint16_t *)addr<VRAM_D) return (VRAM_C_CR&3)==((VRAM_C_TEXTURE)&3);
	else if((uint16_t *)addr<VRAM_E) return (VRAM_D_CR&3)==((VRAM_D_TEXTURE)&3);
	else return 0;
}

uint32_t DSAllocTexture(uint32_t flags)
{
	uint32_t size=DSTextureSize(flags);
	uint32_t addr=next_texture_block;

	next_texture_block+=size;

	// Bug: does not handle non-contiguous texture memory gracefully with large allocations
	while(!VRAMAddressIsTextureBank(next_texture_block-1) && next_texture_block<=(uint32_t)VRAM_E)
	{
		addr=next_texture_block=(next_texture_block&~0x1ffff)+0x20000;
		next_texture_block+=size;
	}

	if(next_texture_block>(uint32_t)VRAM_E) return DS_INVALID_TEXTURE;

	return flags|((addr>>3)&0xffff);
}

void DSFreeAllTextures()
{
	next_texture_block=(uint32_t)VRAM_A;
}

void DSCopyTexture(uint32_t texture,void *data)
{
	uint32_t size=DSTextureSize(texture);
	void *dest=DSTextureAddress(texture);

	uint32_t vramtmp=vramSetMainBanks(VRAM_A_LCD,VRAM_B_LCD,VRAM_C_LCD,VRAM_D_LCD);
	swiCopy(data,dest,size/4|COPY_MODE_WORD);
	vramRestoreMainBanks(vramtmp);
}

uint32_t DSAllocAndCopyTexture(uint32_t flags,void *data)
{
	uint32_t tex=DSAllocTexture(flags);
	if(tex==DS_INVALID_TEXTURE) return DS_INVALID_TEXTURE;
	DSCopyTexture(tex,data);
	return tex;
}

void DSCopyColorTexture(uint32_t texture,uint32_t color)
{
	uint32_t size=DSTextureSize(texture);
	uint16_t *dest=DSTextureAddress(texture);

	uint32_t vramtmp=vramSetMainBanks(VRAM_A_LCD,VRAM_B_LCD,VRAM_C_LCD,VRAM_D_LCD);
	for(int i=0;i<size/2;i++) *dest++=color|0x8000;
	vramRestoreMainBanks(vramtmp);
}

uint32_t DSMakeColorTexture(uint32_t color)
{
	uint32_t texture=DSAllocTexture(DS_TEX_SIZE_T_8|DS_TEX_SIZE_S_8|DS_TEX_WRAP_S|DS_TEX_WRAP_T);
	DSCopyColorTexture(texture,color);
	return texture;
}

uint32_t DSMakeWhiteTexture()
{
	return DSMakeColorTexture(0x7fff);
}



void DSSetFogWithCallback(uint8_t r,uint8_t g,uint8_t b,uint8_t a,int32_t start,int32_t end,int32_t near,
int32_t far,int32_t (*callback)(int32_t z,int32_t start,int32_t end))
{
	uint32_t control=GFX_CONTROL&~0xf00;

	uint32_t startdepth=mulf32(0x7fff,divf32(mulf32(far,near-start),mulf32(start,near-far)));
	uint32_t enddepth=mulf32(0x7fff,divf32(mulf32(far,near-end),mulf32(end,near-far)));

	uint32_t diff=enddepth-startdepth-1;
	int log=0;
	while(diff>>=1) log++;

	int shift=14-log;
	if(shift>10) shift=10;

	GFX_CONTROL=control|DS_FOG_SHIFT(shift);

	GFX_FOG_COLOR=((a&0x1f)<<16)|DSPackRGB5(r,g,b);
	GFX_FOG_OFFSET=startdepth;

	for(int i=0;i<32;i++)
	{
		uint32_t depth=startdepth+i*(0x400>>shift);
		if(i==0) GFX_FOG_TABLE[i]=mulf32(0x7f,callback(start,start,end));
		else if(depth>=enddepth) GFX_FOG_TABLE[i]=mulf32(0x7f,callback(end,start,end));
		else GFX_FOG_TABLE[i]=mulf32(0x7f,callback(divf32(mulf32(far,near),far+mulf32(divf32(depth,0x7fff),near-far)),start,end));
	}
}

static int32_t LinearZCallback(int32_t z,int32_t start,int32_t end)
{
	return divf32(z-start,end-start);
}

void DSSetFogLinearf32(uint8_t r,uint8_t g,uint8_t b,uint8_t a,int32_t start,int32_t end,int32_t near,int32_t far)
{ DSSetFogWithCallback(r,g,b,a,start,end,near,far,LinearZCallback); }

void DSSetFogLinearf(uint8_t r,uint8_t g,uint8_t b,uint8_t a,float start,float end,float near,float far)
{ DSSetFogLinearf32(r,g,b,a,DSf32(start),DSf32(end),DSf32(near),DSf32(far)); }

