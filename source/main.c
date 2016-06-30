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
    int64_t startTick = svcGetSystemTick();
    
    // Start up first effect
    effectTunnelInit();
    
    int currentSwitchTick = 0;
    int64_t switchTicks[8];
    
    float fudge = -17570.0;
    float songOrigSamples = 4056096.0;
    float songOrigRate = 48000.0;
    float songSeconds = songOrigSamples / songOrigRate;
    float halfSectionSeconds = songSeconds / 10.0;
    
    float cpuHz = 268123480.0;
    
    // Tunnel
    switchTicks[0] = (halfSectionSeconds * 2.0 + fudge / songOrigRate) * cpuHz;
    
    // Iso
    switchTicks[1] = (halfSectionSeconds * 3.0 + fudge / songOrigRate) * cpuHz;
    switchTicks[2] = (halfSectionSeconds * 4.0 + fudge / songOrigRate) * cpuHz;
    
    // Meta
    switchTicks[3] = (halfSectionSeconds * 5.0 + fudge / songOrigRate) * cpuHz;
    switchTicks[4] = (halfSectionSeconds * 6.0 + fudge / songOrigRate) * cpuHz;
    
    // Greet
    switchTicks[5] = (halfSectionSeconds * 8.0 + fudge / songOrigRate) * cpuHz;
    
    // Nordlicht
    switchTicks[6] = (halfSectionSeconds * 10.0 + fudge / songOrigRate) * cpuHz;
    
    // Nada
    switchTicks[7] = (halfSectionSeconds * 40.0 + fudge / songOrigRate) * cpuHz;
    
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
        
        //if (kDown & KEY_A && haveEscalated == 0) {
        if(currentTick > switchTicks[currentSwitchTick]) {
            currentSwitchTick += 1;
            escalate += 1;
            haveEscalated = 1;
            // printf("%lld\n", currentTick);
        }
        else {
            haveEscalated = 0;
        }
        
        // Init / Deinit
        if(escalate == 1 && haveEscalated) {
            curEscalate = escalate;
            effectTunnelExit();
            effectIcosphereInit();
        }
        
        if(escalate == 3 && haveEscalated) {
            curEscalate = escalate;
            effectIcosphereExit();
            effectMetaballsInit();
        }
        
        if(escalate == 5 && haveEscalated) {
            curEscalate = escalate;
            effectMetaballsExit();
            effectGreetsInit();
        }
        
        if(escalate == 6 && haveEscalated) {
            curEscalate = escalate;
            effectGreetsExit();
            effectNordlichtInit();
        }
        
        float slider = osGet3DSliderState();
        float iod = slider / 3.0;
        
        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            if(escalate < 1) {
                effectTunnelRender(targetLeft, targetRight, iod, time, escalate - curEscalate);
            } else if(escalate < 3) {
                effectIcosphereRender(targetLeft, targetRight, iod, time, escalate - curEscalate);
            } else if(escalate < 5) {
                effectMetaballsRender(targetLeft, targetRight, iod, time, escalate - curEscalate);
            } else if(escalate < 6) {
                effectGreetsRender(targetLeft, targetRight, iod, time, escalate - curEscalate);
            } else if(escalate < 7) {
                effectNordlichtRender(targetLeft, targetRight, iod, time, escalate - curEscalate);
            }
        C3D_FrameEnd(0);
    }
    
    // Clean up
    effectNordlichtExit();
    
    // Sound off
    csndExit();
    
    // Deinitialize graphics
    C3D_Fini();
    gfxExit();
    return 0;
}
