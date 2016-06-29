// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include "vshader_shbin.h"
#include "svatg3_bin.h"
#include "logo_bin.h"

#include "Lighthouse.h"

#define NORDLICHT_MAX_VERTS 20000

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLoc_projection, uLoc_modelView;
static C3D_Mtx projection;

static C3D_Tex sphere_tex;
static C3D_Tex logo_tex;

static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lut_Phong;
static C3D_LightLut lut_shittyFresnel;

int32_t vertCount;
static vertex* vboVerts;

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
    C3D_TexUpload(&sphere_tex, svatg3_bin);
    C3D_TexSetFilter(&sphere_tex, GPU_LINEAR, GPU_NEAREST);
    
    C3D_TexUpload(&logo_tex, logo_bin);
    C3D_TexSetFilter(&logo_tex, GPU_LINEAR, GPU_NEAREST);
}

void effectTunnelUpdate(float time, float escalate) {
    // TODO
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
    C3D_TexUpload(&sphere_tex, svatg3_bin);
    C3D_TexSetFilter(&sphere_tex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexBind(0, &sphere_tex);
    
    // Calculate the modelView matrix
    C3D_Mtx modelView;
    Mtx_Identity(&modelView);
    Mtx_Translate(&modelView, 0.0, 0.0, -1.5);
    Mtx_RotateX(&modelView,  0.5, true);
    Mtx_RotateZ(&modelView, -0.3, true);
    //Mtx_RotateY(&modelView, time * 0.002, true);
    
    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  &modelView);

    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
    
    C3D_TexEnv* env2 = C3D_GetTexEnv(1);
    C3D_TexEnvSrc(env2, C3D_Alpha, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvOp(env2, C3D_Alpha, 0, 0, 0);
    C3D_TexEnvFunc(env2, C3D_Alpha, GPU_REPLACE);
    
    C3D_TexEnv* env3 = C3D_GetTexEnv(2);
    C3D_TexEnvSrc(env3, C3D_RGB, GPU_FRAGMENT_SECONDARY_COLOR, GPU_PREVIOUS, 0);
    C3D_TexEnvOp(env3, C3D_RGB, GPU_TEVOP_RGB_SRC_ALPHA , 0, 0);
    C3D_TexEnvFunc(env3, C3D_RGB, GPU_ADD);
    
    static const C3D_Material lightMaterial =
    {
        { 0.6f, 0.6f, 0.6f }, //ambient
        { 0.4f, 0.4f, 0.4f }, //diffuse
        { 0.8f, 0.8f, 0.8f }, //specular0
        { 0.0f, 0.0f, 0.0f }, //specular1
        { 0.0f, 0.0f, 0.0f }, //emission
    };

    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);

    LightLut_Phong(&lut_Phong, 3.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Phong);
    
    LightLut_FromFunc(&lut_shittyFresnel, badFresnel, 0.6, false);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lut_shittyFresnel);
    C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
    C3D_FVec lightVec = { { 2.0, 2.0, -2.0, 0.0 } };

    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, 1.0, 1.0, 1.0);
    C3D_LightPosition(&light, &lightVec);
    
    // Depth test is back
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    
    // To heck with culling
    C3D_CullFace(GPU_CULL_NONE);

    
    // Draw the VBO
    C3D_TexBind(0, &sphere_tex);    
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vboVerts, sizeof(vertex), 3, 0x210);
    
    if(vertCount > 0) {
        C3D_DrawArrays(GPU_TRIANGLES, 0, vertCount);
    }
    
    C3D_LightEnvBind(0);
}

void effectTunnelRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time, float escalate) {
    effectTunnelUpdate(time, escalate);
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    
    // Actual scene
    effectTunnelDraw(-iod, time, escalate);
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
    
        // Actual scene
        effectTunnelDraw(iod, time, escalate);
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
