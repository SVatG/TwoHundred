#include "Tools.h"
#include "music_bin.h"

#include "Effects.h"

int main() {
    // Initialize graphics
    gfxInit(GSP_RGBA8_OES, GSP_BGR8_OES, false);
    gfxSet3D(true);
    consoleInit(GFX_BOTTOM, NULL);
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // Initialize the render target
    C3D_RenderTarget* targetLeft = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTarget* targetRight = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(targetLeft, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
    C3D_RenderTargetSetClear(targetRight, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
    C3D_RenderTargetSetOutput(targetLeft, GFX_TOP, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTargetSetOutput(targetRight, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

    // Sound on
    csndInit();

    // Play music
    csndPlaySound(0x8, SOUND_ONE_SHOT | SOUND_FORMAT_16BIT, 32000, 1.0, 0.0, (u32*)music_bin, NULL, music_bin_size);
    uint64_t startTick = svcGetSystemTick();
    
    // Start up first effect
    //effectEcosphereInit();
    effectMetaballsInit();
    
    // Main loop
    float escalate = 0.0;
    int haveEscalated = 0;
    while (aptMainLoop()) {
        hidScanInput();

        int64_t currentTick = svcGetSystemTick() - startTick;
        float time = currentTick * 0.000001;
        printf("\x1b[28;14HMystery: %lud", linearSpaceFree());
        printf("\x1b[29;15HMystery: %lld", currentTick);
            
        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) {
            break; // break in order to return to hbmenu
        }
        
        if (kDown & KEY_A && haveEscalated == 0) {
            escalate += 1.0;
            haveEscalated = 1;
        }
        else {
            haveEscalated = 0;
        }
        float slider = osGet3DSliderState();
        float iod = slider/3;
        
        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            effectMetaballsRender(targetLeft, targetRight, iod, time, escalate);
            //effectEcosphereRender(targetLeft, targetRight, iod, time, escalate);
        C3D_FrameEnd(0);
    }
    
    // Clean up
    //effectIcosphereExit();
    effectMetaballsExit();
    
    // Sound off
    csndExit();
    
    // Deinitialize graphics
    C3D_Fini();
    gfxExit();
    return 0;
}
