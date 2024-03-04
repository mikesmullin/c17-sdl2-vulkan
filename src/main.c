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

static void physicsCallback(const f32 deltaTime);
static void renderCallback(const f32 deltaTime);

int main() {
  printf("begin main.\n");

  // initialize random seed using current time
  srand(time(NULL));

  Vulkan__InitDriver1(&s_Vulkan);

  SDL__EnableAudio();
  SDL__EnableGamepad();
  SDL__EnableVideo();
  Window__New(&s_Window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, &s_Vulkan);
  SDL__Init();
  Audio__Init();

  Gamepad_t gamePad1;
  Gamepad__New(&gamePad1, 0);
  LOG_INFOF(
      "Controller Id: %d, Name: %s\n",
      gamePad1.m_index,
      Gamepad__GetControllerName(&gamePad1));
  Gamepad__Open(&gamePad1);

  Window__Begin(&s_Window);

  Vulkan__AssertDriverValidationLayersSupported(&s_Vulkan);

#if OS_MAC == 1
  // enable MoltenVK support for MacOS cross-platform support
  s_Vulkan.m_requiredDriverExtensions[s_Vulkan.m_requiredDriverExtensionCount++] =
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
  Vulkan__UseLogicalDevice(&s_Vulkan);
  Vulkan__CreateSwapChain(&s_Vulkan);

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
