#include "Tools.h"
#include "music_bin.h"

#include "Effects.h"

#define CLEAR_COLOR 0x555555FF

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
//     effectNordlichtInit();
    effectGreetsInit();
    
    // Main loop
    int escalate = 0;
    int haveEscalated = 0;
    int curEscalate = escalate;
    while (aptMainLoop()) {
        hidScanInput();

        int64_t currentTick = svcGetSystemTick() - startTick;
        float time = currentTick * 0.000001;
            
        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) {
            break; // break in order to return to hbmenu
        }
        
        if (kDown & KEY_A && haveEscalated == 0) {
            escalate += 1;
            haveEscalated = 1;
        }
        else {
            haveEscalated = 0;
        }
        
//         // Init / Deinit
//         if(escalate == 1 && haveEscalated) {
//             curEscalate = escalate;
//             effectNordlichtExit();
//             effectIcosphereInit();
//         }
//         
//         if(escalate == 3 && haveEscalated) {
//             curEscalate = escalate;
//             effectIcosphereExit();
//             effectMetaballsInit();
//         }
        
        float slider = osGet3DSliderState();
        float iod = slider / 3.0;
        
        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            // effectNordlichtRender(targetLeft, targetRight, iod, time, curEscalate);
//             if(escalate < 1) {
//                 effectNordlichtRender(targetLeft, targetRight, iod, time, escalate - curEscalate);
//             } else if(escalate < 3) {
//                 effectIcosphereRender(targetLeft, targetRight, iod, time, escalate - curEscalate);
//             } else if(escalate < 5) {
//                 effectMetaballsRender(targetLeft, targetRight, iod, time, escalate - curEscalate);
//             }
            effectGreetsRender(targetLeft, targetRight, iod, time, escalate - curEscalate);
        C3D_FrameEnd(0);
    }
    
    // Clean up
    //effectMetaballsExit();
    effectGreetsExit();
    
    // Sound off
    csndExit();
    
    // Deinitialize graphics
    C3D_Fini();
    gfxExit();
    return 0;
}
