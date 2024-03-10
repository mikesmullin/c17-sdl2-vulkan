#include <cglm/cglm.h>
#include <stdio.h>

#include "lib/Audio.h"
#include "lib/Gamepad.h"
#include "lib/SDL.h"
#include "lib/Timer.h"
#include "lib/Vulkan.h"
#include "lib/Window.h"

static char* WINDOW_TITLE = "Survival";
static char* ENGINE_NAME = "MS2024";
static u16 WINDOW_WIDTH = 800;
static u16 WINDOW_HEIGHT = 800;

static const u8 PHYSICS_FPS = 50;
static const u8 RENDER_FPS = 60;
static bool isVBODirty = true;
static bool isUBODirty[] = {true, true};

static Vulkan_t s_Vulkan;
static Window_t s_Window;

typedef struct {
  vec2 vertex;
} Mesh_t;

typedef struct {
  vec3 pos;
  vec3 rot;
  vec3 scale;
  u32 texId;
} Instance_t;

#define MAX_INSTANCES 255  // TODO: find out how to exceed this limit
static u16 instanceCount = 1;
static Instance_t instances[MAX_INSTANCES];

typedef struct {
  vec3 cam;
  vec3 look;
  vec2 user1;
  vec2 user2;
  f32 aspect;
} World_t;

static World_t world;

static vec3 VEC3_Y_UP = {0, 1, 0};

typedef struct {
  mat4 proj;
  mat4 view;
  vec2 user1;
  vec2 user2;
} ubo_ProjView_t;

static Mesh_t vertices[] = {
    {{-0.5f, -0.5f}},
    {{0.5f, -0.5f}},
    {{0.5f, 0.5f}},
    {{-0.5f, 0.5f}},
};

static u16 indices[] = {0, 1, 2, 2, 3, 0};

static const char* shaderFiles[] = {
    "../assets/shaders/simple_shader.frag.spv",
    "../assets/shaders/simple_shader.vert.spv",
};

static const char* textureFiles[] = {
    "../assets/textures/roguelikeSheet_transparent.png",
    "../assets/textures/wood-wall.png",
};

static ubo_ProjView_t ubo1;  // projection x view matrices

static void physicsCallback(const f64 deltaTime);
static void renderCallback(const f64 deltaTime);

static const u16 BACKGROUND_WH = 800;
static const u16 PIXELS_PER_UNIT = BACKGROUND_WH;
static f32 PixelsToUnits(u32 pixels) {
  return (f32)pixels / PIXELS_PER_UNIT;
}

int main() {
  printf("begin main.\n");

  Timer__MeasureCycles();

  // initialize random seed using current time
  srand(Timer__NowMilliseconds());

  Vulkan__InitDriver1(&s_Vulkan);

  Window__New(&s_Window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, &s_Vulkan);
  SDL__Init();
  Audio__Init();

  Gamepad_t gamePad1;
  Gamepad__New(&gamePad1, 0);
  LOG_INFOF("Controller Id: %d, Name: %s", gamePad1.m_index, Gamepad__GetControllerName(&gamePad1));
  Gamepad__Open(&gamePad1);

  Window__Begin(&s_Window);

  Vulkan__AssertDriverValidationLayersSupported(&s_Vulkan);

#if OS_MAC == 1
  ASSERT(s_Vulkan.m_requiredDriverExtensionsCount < VULKAN_REQUIRED_DRIVER_EXTENSIONS_CAP)
  // enable MoltenVK support for MacOS cross-platform support
  s_Vulkan.m_requiredDriverExtensions[s_Vulkan.m_requiredDriverExtensionsCount++] =
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
#endif
  Vulkan__AssertDriverExtensionsSupported(&s_Vulkan);

  Vulkan__CreateInstance(&s_Vulkan, WINDOW_TITLE, ENGINE_NAME, 1, 0, 0);
  Vulkan__InitDriver2(&s_Vulkan);

  Vulkan__UsePhysicalDevice(&s_Vulkan, 0);
  Window__Bind(&s_Window);

  DrawableArea_t area = {0, 0};
  Window__GetDrawableAreaExtentBounds(&s_Window, &area);
  Window__KeepAspectRatio(&s_Window, area.width, area.height);

  // establish vulkan scene
  Vulkan__AssertSwapChainSupported(&s_Vulkan);
  Vulkan__CreateLogicalDeviceAndQueues(&s_Vulkan);
  Vulkan__CreateSwapChain(&s_Vulkan, false);
  Vulkan__CreateImageViews(&s_Vulkan);
  Vulkan__CreateRenderPass(&s_Vulkan);
  Vulkan__CreateDescriptorSetLayout(&s_Vulkan);
  Vulkan__CreateGraphicsPipeline(
      &s_Vulkan,
      shaderFiles[0],
      shaderFiles[1],
      sizeof(Mesh_t),
      sizeof(Instance_t),
      5,
      (u32[5]){0, 1, 1, 1, 1},
      (u32[5]){0, 1, 2, 3, 4},
      (u32[5]){
          VK_FORMAT_R32G32_SFLOAT,
          VK_FORMAT_R32G32B32_SFLOAT,
          VK_FORMAT_R32G32B32_SFLOAT,
          VK_FORMAT_R32G32B32_SFLOAT,
          VK_FORMAT_R32_UINT},
      (u32[5]){
          offsetof(Mesh_t, vertex),
          offsetof(Instance_t, pos),
          offsetof(Instance_t, rot),
          offsetof(Instance_t, scale),
          offsetof(Instance_t, texId)});
  Vulkan__CreateFrameBuffers(&s_Vulkan);
  Vulkan__CreateCommandPool(&s_Vulkan);
  Vulkan__CreateTextureImage(&s_Vulkan, textureFiles[0]);
  Vulkan__CreateTextureImageView(&s_Vulkan);
  Vulkan__CreateTextureSampler(&s_Vulkan);
  Vulkan__CreateVertexBuffer(&s_Vulkan, 0, sizeof(vertices), vertices);
  Vulkan__CreateVertexBuffer(&s_Vulkan, 1, sizeof(instances), instances);
  Vulkan__CreateIndexBuffer(&s_Vulkan, sizeof(indices), indices);
  Vulkan__CreateUniformBuffers(&s_Vulkan, sizeof(ubo1));
  Vulkan__CreateDescriptorPool(&s_Vulkan);
  Vulkan__CreateDescriptorSets(&s_Vulkan);
  Vulkan__CreateCommandBuffers(&s_Vulkan);
  Vulkan__CreateSyncObjects(&s_Vulkan);
  s_Vulkan.m_drawIndexCount = ARRAY_COUNT(indices);

  // setup scene
  world.aspect = ASPECT_SQUARE;
  glm_vec3_copy((vec3){0, 0, 1}, world.cam);
  glm_vec3_copy((vec3){0, 0, 0}, world.look);

  instanceCount = 1;
  glm_vec3_copy((vec3){PixelsToUnits(200), PixelsToUnits(200), 0}, instances[0].pos);
  glm_vec3_copy((vec3){0, 0, 0}, instances[0].rot);
  glm_vec3_copy((vec3){PixelsToUnits(200), PixelsToUnits(200), 1}, instances[0].scale);
  instances[0].texId = 0;

  LOG_DEBUGF(
      "instances[0] pos %2.5f %2.5f %2.5f",
      instances[0].pos[0],
      instances[0].pos[1],
      instances[0].pos[2])

  LOG_DEBUGF(
      "instances[0] rot %2.5f %2.5f %2.5f",
      instances[0].rot[0],
      instances[0].rot[1],
      instances[0].rot[2])

  LOG_DEBUGF(
      "instances[0] scale %2.5f %2.5f %2.5f",
      instances[0].scale[0],
      instances[0].scale[1],
      instances[0].scale[2])

  // main loop
  Window__RenderLoop(&s_Window, PHYSICS_FPS, RENDER_FPS, &physicsCallback, &renderCallback);

  // cleanup
  printf("shutdown main.\n");
  Vulkan__DeviceWaitIdle(&s_Vulkan);
  Gamepad__Shutdown(&gamePad1);
  Vulkan__Cleanup(&s_Vulkan);
  Audio__Shutdown();
  Window__Shutdown(&s_Window);
  printf("end main.\n");
  return 0;
}

void physicsCallback(const f64 deltaTime) {
  // OnFixedUpdate(deltaTime);
}

void renderCallback(const f64 deltaTime) {
  // OnUpdate(deltaTime);

  if (isVBODirty) {
    isVBODirty = false;

    s_Vulkan.m_instanceCount = instanceCount;
    Vulkan__UpdateVertexBuffer(&s_Vulkan, 1, instanceCount, instances);
  }

  if (isUBODirty[s_Vulkan.m_currentFrame]) {
    isUBODirty[s_Vulkan.m_currentFrame] = false;

    glm_lookat(
        world.cam,
        world.look,
        VEC3_Y_UP,  // Y-axis points upwards (GLM default)
        ubo1.view);

    s_Vulkan.m_aspectRatio = world.aspect;  // sync viewport
    // glm_perspective(
    //     glm_rad(45.0f),  // half the actual 90deg fov
    //     world.aspect,
    //     0.1f,  // TODO: adjust clipping range for z depth?
    //     10.0f,
    //     ubo1.proj);
    glm_ortho(-0.5f, +0.5f, -0.5f, +0.5f, 0.1f, 10.0f, ubo1.proj);

    ubo1.proj[2][2] = -0.10101;
    ubo1.proj[3][2] = -0.01010;
    glm_vec2_copy(world.user1, ubo1.user1);

    world.user2[0] = 1.0f;
    world.user2[1] = 1.0f;
    glm_vec2_copy(world.user2, ubo1.user2);

    // TODO: not sure i make use of one UBO per frame, really
    Vulkan__UpdateUniformBuffer(&s_Vulkan, s_Vulkan.m_currentFrame, &ubo1);
  }
}
