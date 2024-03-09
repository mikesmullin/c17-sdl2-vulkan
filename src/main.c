#include <cglm/cglm.h>
#include <stdio.h>
#include <time.h>

#include "lib/Audio.h"
#include "lib/Gamepad.h"
#include "lib/SDL.h"
#include "lib/Vulkan.h"
#include "lib/Window.h"

static char* WINDOW_TITLE = "Survival";
static char* ENGINE_NAME = "MS2024";
static u16 WINDOW_WIDTH = 800;
static u16 WINDOW_HEIGHT = 800;

static const u8 PHYSICS_FPS = 50;
static const u8 RENDER_FPS = 60;
static bool isVBODirty = true;
static bool isUBODirty = true;

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
Instance_t instances[MAX_INSTANCES];

typedef struct {
  vec3 cam;
  vec3 look;
  vec2 user1;
  vec2 user2;
  f32 aspect;
} World_t;

typedef struct {
  mat4 proj;
  mat4 view;
  vec2 user1;
  vec2 user2;
} ubo_ProjView_t;

Mesh_t vertices[] = {
    {{-0.5f, -0.5f}},
    {{0.5f, -0.5f}},
    {{0.5f, 0.5f}},
    {{-0.5f, 0.5f}},
};

// TODO: may have to use u16 here
u8 indices[] = {0, 1, 2, 2, 3, 0};

const char* shaderFiles[] = {
    "../assets/shaders/simple_shader.frag.spv",
    "../assets/shaders/simple_shader.vert.spv",
};

const char* textureFiles[] = {
    "../assets/textures/roguelikeSheet_transparent.png",
    "../assets/textures/wood-wall.png",
};

static void physicsCallback(const f32 deltaTime);
static void renderCallback(const f32 deltaTime);

int main() {
  printf("begin main.\n");

  // initialize random seed using current time
  srand(time(NULL));

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
  Vulkan__CreateSwapChain(&s_Vulkan, NULL);
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
  ubo_ProjView_t ubo1;  // projection x view matrices
  Vulkan__CreateUniformBuffers(&s_Vulkan, sizeof(ubo1));
  Vulkan__CreateDescriptorPool(&s_Vulkan);
  Vulkan__CreateDescriptorSets(&s_Vulkan);
  // Vulkan__CreateCommandBuffers(&s_Vulkan);
  // Vulkan__CreateSyncObjects(&s_Vulkan);

  // main loop
  Window__RenderLoop(&s_Window, PHYSICS_FPS, RENDER_FPS, &physicsCallback, &renderCallback);

  // cleanup
  printf("end main.\n");
  Audio__Shutdown();
  Window__Shutdown(&s_Window);
  printf("shutdown complete.\n");
  return 0;
}

void physicsCallback(const f32 deltaTime) {
  // OnFixedUpdate(deltaTime);
}

void renderCallback(const f32 deltaTime) {
  // OnUpdate(deltaTime);

  if (isVBODirty) {
    isVBODirty = false;
    // w.v.instanceCount = instances.size();
    // w.v.UpdateVertexBuffer(1, VectorSize(instances), instances.data());
  }

  if (isUBODirty) {
    isUBODirty = false;

    // ubo1.view = glm::lookAt(
    //     glm::vec3(world.cam.x, world.cam.y, world.cam.z),
    //     glm::vec3(world.look.x, world.look.y, world.look.z),
    //     glm::vec3(0.0f, 1.0f, 0.0f));  // Y-axis points upwards (GLM default)
    // w.v.aspectRatio = world.aspect;    // sync viewport
    // // ubo1.proj = glm::perspective(
    // //     glm::radians(45.0f),  // half the actual 90deg fov
    // //     world.aspect,
    // //     0.1f,  // TODO: adjust clipping range for z depth?
    // //     10.0f);
    // ubo1.proj = glm::ortho(-0.5f, +0.5f, -0.5f, +0.5f, 0.1f, 10.0f);
    // ubo1.user1 = world.user1;
    // ubo1.user2 = world.user2;

    // w.v.UpdateUniformBuffer(&ubo1);
  }
}
