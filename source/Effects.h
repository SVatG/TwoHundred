#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#include "Tools.h"

extern void effectEcosphereInit(void);
extern void effectEcosphereRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time, float escalate);
extern void effectIcosphereExit(void);

extern void effectMetaballsInit();
extern void effectMetaballsRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time, float escalate);
extern void effectMetaballsExit(void);

#endif