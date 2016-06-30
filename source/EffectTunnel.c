// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include "vshader_shbin.h"
#include "tunnel_bin.h"
#include "tunnel_logo_bin.h"
#include "Perlin.h"

#include "Lighthouse.h"

#define NORDLICHT_MAX_VERTS 20000

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLoc_projection, uLoc_modelView;
static C3D_Mtx projection;

static C3D_Tex sphere_tex;
static C3D_Tex logo_tex;

int32_t vertCount;
static vertex* vboVerts;

#define SEGMENTS 20

// Ohne tunnel geht eben nicht
void effectTunnelInit() {
    // Load default shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);

    C3D_TexInit(&sphere_tex, 256, 256, GPU_RGBA8);
    C3D_TexInit(&logo_tex, SCREEN_TEXTURE_HEIGHT, SCREEN_TEXTURE_WIDTH, GPU_RGBA8);
    
    vboVerts = (vertex*)linearAlloc(sizeof(vertex) * NORDLICHT_MAX_VERTS);
    
    // Load the texture and bind it to the first texture unit
    C3D_TexUpload(&sphere_tex, tunnel_bin);
    C3D_TexSetFilter(&sphere_tex, GPU_LINEAR, GPU_NEAREST);
    
    C3D_TexUpload(&logo_tex, tunnel_logo_bin);
    C3D_TexSetFilter(&logo_tex, GPU_LINEAR, GPU_NEAREST);
}

void effectTunnelUpdate(float time, float escalate) {
    int depth = 50;
    float zscale = 0.2;
    float rscale = 1.8;
    float rscale_inner = 1.3;
    
    vertCount = 0;
    vec3_t ringPos[SEGMENTS];
    vec3_t ringPosPrev[SEGMENTS];
    
    vec3_t ringPosInner[SEGMENTS];
    vec3_t ringPosPrevInner[SEGMENTS];
    
    float distance = -time * 0.01;
    float offset = fmod(distance, 1.0);
    // printf("%f %f\n", distance, offset);
    
    for(int z = depth - 1; z >= 0; z--) {
        float zo = z + offset;
        float xo = sin(zo * 0.05) * (zo / 10.0);
        float yo = cos(zo * 0.05) * (zo / 10.0);
        
        for(int s = 0; s < SEGMENTS; s++) {
            float rn = 0.9 + noise_at(z - distance + offset, s, 0.5) / 4.0;
            float sf = (((float)s) / (float)SEGMENTS) * 2.0 * 3.1415;
            ringPos[s] = vec3((sin(sf) * rscale + xo) * rn, (cos(sf) * rscale + yo) * rn, -zo * zscale);
            ringPosInner[s] = vec3((sin(sf) * rscale_inner + xo) * rn, (cos(sf) * rscale_inner + yo) * rn, -zo * zscale);
            
        }
        
        if(z != depth - 1) {
            for(int s = 0; s < SEGMENTS; s++) {
                int sn = (s + 1) % SEGMENTS;
                vertCount += buildQuad(&(vboVerts[vertCount]), ringPosPrev[s], ringPos[s], ringPos[sn], ringPosPrev[sn], 
                                    vec2(0.5, 0.5), vec2(1, 0.5), vec2(0.5, 1), vec2(1, 1));
                
                vertCount += buildQuad(&(vboVerts[vertCount]), ringPosPrevInner[s], ringPosInner[s], ringPosInner[sn], ringPosPrevInner[sn], 
                                    vec2(0.0, 0.0), vec2(0.5, 0.0), vec2(0.0, 0.5), vec2(0.5, 0.5));
            }
        }
        
        for(int s = 0; s < SEGMENTS; s++) {
            ringPosPrev[s] = ringPos[s];
            ringPosPrevInner[s] = ringPosInner[s];
        }
    }
}

// Draw 
void effectTunnelDraw(float iod, float time, float escalate) {
    C3D_BindProgram(&program);

    // Get the location of the uniforms
    uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
    uLoc_modelView = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

    // Configure attributes for use with the vertex shader
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2=normal

    // Compute the projection matrix
    Mtx_PerspStereoTilt(&projection, 65.0f*M_PI/180.0f, 400.0f/240.0f, 0.01f, 1000.0f, iod, 2.0f);

    // Load the texture and bind it to the first texture unit
    C3D_TexUpload(&sphere_tex, tunnel_bin);
    C3D_TexSetFilter(&sphere_tex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexBind(0, &sphere_tex);
    
    // Calculate the modelView matrix
    C3D_Mtx modelView;
    Mtx_Identity(&modelView);
    Mtx_Translate(&modelView, 0.0, 0.0, 0.0);
//     Mtx_RotateX(&modelView,  0.5, true);
//     Mtx_RotateZ(&modelView, -0.3, true);
    //Mtx_RotateY(&modelView, time * 0.002, true);
    
    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  &modelView);

    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    
    // To heck with culling
    C3D_CullFace(GPU_CULL_NONE);

    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    
    C3D_LightEnvBind(0);
    
    // Draw the VBO
    C3D_TexBind(0, &sphere_tex);    
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vboVerts, sizeof(vertex), 3, 0x210);
    
    if(vertCount > 0) {
        C3D_DrawArrays(GPU_TRIANGLES, 0, vertCount);
    }
    
    // Depth test is back
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    
    C3D_LightEnvBind(0);
}

void effectTunnelRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time, float escalate) {
    effectTunnelUpdate(time, escalate);
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    
    // Actual scene
    effectTunnelDraw(-iod, time, escalate);
    
    // Overlay
    fullscreenQuad(logo_tex, 0.0, 0.0);
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
    
        // Actual scene
        effectTunnelDraw(iod, time, escalate);
        
        // Overlay
        fullscreenQuad(logo_tex, 0.0, 0.0);
    }
}

void effectTunnelExit() {
    // Free the texture
    C3D_TexDelete(&sphere_tex);
    
    // Free the VBOs
    linearFree(vboVerts);
    
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}
