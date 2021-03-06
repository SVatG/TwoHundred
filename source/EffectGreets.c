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
#include "greets_bin.h"
#include "traj_0_bin.h"
#include "traj_1_bin.h"
#include "traj_2_bin.h"
#include "traj_3_bin.h"
#include "traj_4_bin.h"
#include "traj_5_bin.h"
#include "traj_6_bin.h"
#include "Lighthouse.h"

#define NORDLICHT_MAX_VERTS 20000
#define GREET_BREAKS 30
#define GREET_SPEED 0.17
#define NUM_GREETS 8

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLoc_projection, uLoc_modelView;
static C3D_Mtx projection;

static C3D_Tex sphere_tex;
static C3D_Tex logo_tex;

static Pixel* texPixels;
static Bitmap texture;

static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lut_Phong;
static C3D_LightLut lut_shittyFresnel;

static Pixel* screenPixels;
static Bitmap screen;
static C3D_Tex screen_tex;
static C3D_Tex logo_tex;

int32_t vertCount;
static vertex* vboVerts;

int32_t trajSize[8];
int32_t trajPosition[8];
int32_t trajOffset[8];
int32_t* traj[8];

vec3_t prevA[8];
vec3_t prevB[8];
vec3_t prevC[8];
vec3_t prevD[8];

vec3_t prevCP[8];
int prevEmittedCount[8];

int32_t trajCount = 1;

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

// sicke greets
int32_t curGreetStart = 0;
int32_t nextGreetStart = 0;
int32_t currentGreet = 0;

void effectGreetsInit() {
    // Load default shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);

    C3D_TexInit(&sphere_tex, 256, 256, GPU_RGBA8);
    C3D_TexInit(&logo_tex, SCREEN_TEXTURE_HEIGHT, SCREEN_TEXTURE_WIDTH, GPU_RGBA8);
    
    C3D_TexInit(&screen_tex, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, GPU_RGBA8);
    
    screenPixels = (Pixel*)linearAlloc(SCREEN_TEXTURE_WIDTH * SCREEN_TEXTURE_HEIGHT * sizeof(Pixel));
    InitialiseBitmap(&screen, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, BytesPerRowForWidth(SCREEN_TEXTURE_WIDTH), screenPixels);
    
    vboVerts = (vertex*)linearAlloc(sizeof(vertex) * NORDLICHT_MAX_VERTS);
    
    // Load the texture and bind it to the first texture unit
    C3D_TexSetFilter(&sphere_tex, GPU_LINEAR, GPU_NEAREST);
    
    C3D_TexUpload(&logo_tex, greets_bin);
    C3D_TexSetFilter(&logo_tex, GPU_LINEAR, GPU_NEAREST);
    
    texPixels = (Pixel*)linearAlloc(256 * 256 * sizeof(Pixel));
    InitialiseBitmap(&texture, 256, 256, BytesPerRowForWidth(256), texPixels);
    
    traj[0] = (int32_t*)traj_0_bin;
    trajSize[0] = traj_0_bin_size / (2 * sizeof(int32_t));
    trajOffset[0] = 0;
    trajPosition[0] = 1;
    
    traj[1] = (int32_t*)traj_0_bin;
    trajSize[1] = traj_0_bin_size / (2 * sizeof(int32_t));
    trajOffset[1] = 0;
    trajPosition[1] = 1;
    
    traj[2] = (int32_t*)traj_1_bin;
    trajSize[2] = traj_1_bin_size / (2 * sizeof(int32_t));
    trajOffset[2] = 0;
    trajPosition[2] = 1;

    traj[3] = (int32_t*)traj_2_bin;
    trajSize[3] = traj_2_bin_size / (2 * sizeof(int32_t));
    trajOffset[3] = 0;
    trajPosition[3] = 1;
    
    traj[4] = (int32_t*)traj_3_bin;
    trajSize[4] = traj_3_bin_size / (2 * sizeof(int32_t));
    trajOffset[4] = 0;
    trajPosition[4] = 1;
    
    traj[5] = (int32_t*)traj_4_bin;
    trajSize[5] = traj_4_bin_size / (2 * sizeof(int32_t));
    trajOffset[5] = 0;
    trajPosition[5] = 1;
    
    traj[6] = (int32_t*)traj_5_bin;
    trajSize[6] = traj_5_bin_size / (2 * sizeof(int32_t));
    trajOffset[6] = 0;
    trajPosition[6] = 1;
    
    traj[7] = (int32_t*)traj_6_bin;
    trajSize[7] = traj_6_bin_size / (2 * sizeof(int32_t));
    trajOffset[7] = 0;
    trajPosition[7] = 1;
}

void effectGreetsUpdate(float time, float escalate) {
    float radius = 0.06;
    float scale = 0.01;
    float curPosf = time * GREET_SPEED;
    int curPos = (int32_t)curPosf;
    

    if(nextGreetStart == 0) {
        nextGreetStart = trajSize[currentGreet] + GREET_BREAKS;
        curGreetStart = 0;
    }
    
    if(curPos > nextGreetStart) {
        currentGreet++;        
        curGreetStart = curPos;
        nextGreetStart = curGreetStart + trajSize[currentGreet] + GREET_BREAKS;
        vertCount = 0;
        prevEmittedCount[currentGreet] = 0;
    }
    
    if(currentGreet >= NUM_GREETS) {
        vertCount = 0;
        return;
    }
    
    int i = currentGreet;
    int startPosition = (trajPosition[i] == 1 ? 1 : trajPosition[i] - 1);
    vertCount = (trajPosition[i] == 1 ? vertCount : vertCount - prevEmittedCount[i]); // Four quads worth
    for(int j = startPosition; j < min(trajSize[i], curPos - curGreetStart); j++) {
        float along = ((curPosf - curGreetStart - (float)j + 1.0));
        along = min(along, 1.0);
        
        float prevX = traj[i][j - 1] * scale;
        float prevY = traj[i][(j - 1) + trajSize[i]] * scale;
        float curX = traj[i][j] * scale;
        float curY = traj[i][j + trajSize[i]] * scale;
        
        vec3_t dirVec = vec3(prevX - curX, prevY - curY, 0.0);
        vec3_t sideVec1 = vec3(0, 0, 1);
        vec3_t sideVec2 = vec3norm(vec3cross(sideVec1, vec3norm(dirVec)));
        
        sideVec1 = mat3x3transform(mat3x3rotate(j * 0.3, dirVec), sideVec1);
        sideVec2 = mat3x3transform(mat3x3rotate(j * 0.3, dirVec), sideVec2);
        
        vec3_t nextCP = vec3(curX, -curY, sin(j * 0.8 + i) * 0.05);
        vec3_t cpToCp = vec3sub(nextCP, prevCP[i]);
        vec3_t cp = vec3add(prevCP[i], vec3mul(cpToCp, along));
        vec3_t a = vec3add(vec3add(cp, vec3mul(sideVec1, -radius)), vec3mul(sideVec2, -radius));
        vec3_t b = vec3add(vec3add(cp, vec3mul(sideVec1, radius)), vec3mul(sideVec2, -radius));
        vec3_t c = vec3add(vec3add(cp, vec3mul(sideVec1, radius)), vec3mul(sideVec2, radius));
        vec3_t d = vec3add(vec3add(cp, vec3mul(sideVec1, -radius)), vec3mul(sideVec2, radius));
        
        prevEmittedCount[i] = 0;
        if(prevX == 0 || prevY == 0) {
//                 vertCount += buildQuad(&(vboVerts[vertCount]), a, b, c, d, vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1));
        }
        else {
            if(curX == 0 || curY == 0) {
//                     vertCount += buildQuad(&(vboVerts[vertCount]), prevA[i], prevB[i], prevC[i], prevD[i], vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1));
            }
            else {
                vertCount += buildQuadProjectiveXY(&(vboVerts[vertCount]), a, b, prevB[i], prevA[i], 1.0, 0.0, 0.3);
                vertCount += buildQuadProjectiveXY(&(vboVerts[vertCount]), b, c, prevC[i], prevB[i], 1.0, 0.0, 0.3);
                vertCount += buildQuadProjectiveXY(&(vboVerts[vertCount]), c, d, prevD[i], prevC[i], 1.0, 0.0, 0.3);
                vertCount += buildQuadProjectiveXY(&(vboVerts[vertCount]), d, a, prevA[i], prevD[i], 1.0, 0.0, 0.3);
                prevEmittedCount[i] = 6;
            }
            
        }
        
        prevA[i] = a;
        prevB[i] = b;
        prevC[i] = c;
        prevD[i] = d;
        
        prevCP[i] = nextCP;
    }
    
    trajPosition[i] = curPos - curGreetStart;
    
    // Recompute normals
    for(int f = 0; f < vertCount / 3; f++) {
        vec3_t a = vec3(vboVerts[f * 3 + 0].position[0], vboVerts[f * 3 + 0].position[1], vboVerts[f * 3 + 0].position[2]);
        vec3_t b = vec3(vboVerts[f * 3 + 1].position[0], vboVerts[f * 3 + 1].position[1], vboVerts[f * 3 + 1].position[2]);
        vec3_t c = vec3(vboVerts[f * 3 + 2].position[0], vboVerts[f * 3 + 2].position[1], vboVerts[f * 3 + 2].position[2]);
        vec3_t n = vec3norm(vec3cross(vec3norm(vec3sub(b, a)), vec3norm(vec3sub(c, a))));
        for(int v = 0; v < 3; v++) {
            vboVerts[f * 3 + v].normal[0] = n.x;
            vboVerts[f * 3 + v].normal[1] = n.y;
            vboVerts[f * 3 + v].normal[2] = n.z;
        }
    }
    
}

// Draw 
void effectGreetsDraw(float iod, float time, float escalate) {
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
    C3D_TexSetFilter(&sphere_tex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexBind(0, &sphere_tex);
    
    // Calculate the modelView matrix
    C3D_Mtx modelView;
    Mtx_Identity(&modelView);
    Mtx_Translate(&modelView, -1.2 /*- time * 0.0005*/ + sin(time * 0.001) * 0.1, 1.0, -1.5);
//     Mtx_RotateX(&modelView,  0.5, true);
//     Mtx_RotateZ(&modelView, -0.3, true);
    Mtx_RotateY(&modelView, sin(time * 0.001) * 0.1, true);
    
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
    //C3D_CullFace(GPU_CULL_NONE);

    
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

void effectGreetsRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float time, float escalate) {
    FillBitmap(&texture, RGBAf(0.9, 0.9, 0.9, 1.0));
    int stripeWidth = 15;
    int stripeOffset = 100;
    int stripeOffset2 = 80;
    Pixel primaryColor = RGBAf(0.0, 1.0, 1.0, 1.0);
    Pixel secondaryColor = RGBAf(0.0, 0.0, 0.0, 0.0);
    float stripeProgress = max(0.0, ((time * GREET_SPEED) - nextGreetStart + GREET_BREAKS / 2.0) / (GREET_BREAKS / 2.0));    
    float stripeProgress2 = max(0.0, (stripeProgress - 0.5) * 2.0);    
    for(int x = -stripeOffset; x < 256 + stripeOffset; x += stripeWidth) {
        for(int s = 0.0; s < stripeProgress * stripeWidth; s++) {
            DrawLine(&texture, x + s, 0, x + s + stripeOffset, 256, primaryColor); 
        }
        
        for(int s = 0.0; s < stripeProgress2 * stripeWidth; s++) {
            DrawLine(&texture, x + s, 0, x + s + stripeOffset2, 256, secondaryColor); 
        }
    }
    
//     DrawFilledRectangle(&texture, 0, 0, 10, 256, RGBAf(1.0, 0.0, 0.0, 1.0));
//     DrawFilledRectangle(&texture, 246, 0, 10, 256, RGBAf(0.0, 1.0, 0.0, 1.0));
    
    GSPGPU_FlushDataCache(texPixels, 256 * 256 * sizeof(Pixel));
    GX_DisplayTransfer((u32*)texPixels, GX_BUFFER_DIM(256, 256), (u32*)sphere_tex.data, GX_BUFFER_DIM(256, 256), TEXTURE_TRANSFER_FLAGS);
        
    gspWaitForPPF();
    
    float stripeSize = 200.0;
    for(int x = 0; x < SCREEN_WIDTH + 10; x += 10) {
        for(int y = 0; y < SCREEN_HEIGHT + 10; y += 10) {
            float blend =  ((int)(x + y * 3 + time * 0.2) % (int)stripeSize);
            blend = (sin((blend / stripeSize) * 3.14) + 1.0) / 2.0;
            Pixel colorPrimary = RGBAf(blend / 3.0, blend / 2.0, blend / 3.0, 1.0);
            Pixel colorSecondary = RGBAf(blend / 2.0, blend / 5.0, blend / 2.0, 1.0);
            if((((x / 10) & 1) ^ ((y / 10) & 1)) == 0) {
                Pixel colorTemp = colorPrimary;
                colorPrimary = colorSecondary;
                colorSecondary = colorTemp;
            }
            //DrawFilledRectangle(&screen, x, y, 10, 10, colorPrimary);
            DrawFilledCircle(&screen, x + 5, y + 5, 10, colorSecondary);
        }
    }
    
    GSPGPU_FlushDataCache(screenPixels, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Pixel));
    GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), (u32*)screen_tex.data, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), TEXTURE_TRANSFER_FLAGS);
    
    effectGreetsUpdate(time, escalate);
    
    gspWaitForPPF();    
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    
     // Fullscreen quad
    fullscreenQuad(screen_tex, -iod, 1.0 / 10.0);
    
    // Actual scene
    effectGreetsDraw(-iod, time, escalate);
    
    // Overlay
    fullscreenQuad(logo_tex, 0.0, 0.0);
    
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
    
        // Fullscreen quad
        fullscreenQuad(screen_tex, iod, 1.0 / 10.0);        
        
        // Actual scene
        effectGreetsDraw(iod, time, escalate);
        
        // Overlay
        fullscreenQuad(logo_tex, 0.0, 0.0);
        
        fade();
    }
}

void effectGreetsExit() {
    gspWaitForP3D();
    
    // Free the texture
    C3D_TexDelete(&sphere_tex);
    linearFree(texPixels);
    
    // Free the VBOs
    linearFree(vboVerts);
    
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}
