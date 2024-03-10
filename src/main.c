#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/cglm.h>
#include <stdio.h>

#include "lib/Audio.h"
#include "lib/Finger.h"
#include "lib/Gamepad.h"
#include "lib/Keyboard.h"
#include "lib/Math.h"
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

static const f32 PLAYER_WALK_SPEED = 1.0f / 3;  // per-second
static const f32 PLAYER_ZOOM_SPEED = 1.0f / 8;  // per-second

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

enum INSTANCES {
  INSTANCE_FLOOR_0 = 0,
  INSTANCE_PLAYER_1 = 1,
};

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
    "../assets/textures/atlas.png",
};

static const char* audioFiles[] = {
    "../assets/audio/music/grassland.wav",
    "../assets/audio/sfx/grassland_footsteps.wav",
    "../assets/audio/sfx/set_wood_wall.wav",
};

enum AUDIO_FILES {
  AUDIO_AMBIENCE = 0,
  AUDIO_FOOTSTEPS = 1,
  AUDIO_SET_WOOD_WALL = 2,
};

static ubo_ProjView_t ubo1;  // projection x view matrices

static void physicsCallback(const f64 deltaTime);
static void renderCallback(const f64 deltaTime);
static void keyboardCallback();
static void fingerCallback();

static const u16 CANVAS_WH = 800;
static const u16 PIXELS_PER_UNIT = CANVAS_WH;
static f32 PixelsToUnits(u32 pixels) {
  return (f32)pixels / PIXELS_PER_UNIT;
}

typedef enum {
  FRONT = 0,
  LEFT = 1,
  RIGHT = 2,
  BACK = 4,
} PlayerFacing_t;

typedef enum {
  IDLE = 0,
  WALK = 1,
} PlayerAnimState_t;

typedef struct {
  f32 duration;
  u8 frameCount;
  u8 frames[];
} Animation_t;

static Animation_t ANIM_VIKING_IDLE_FRONT = {
    .duration = 3.0f,
    .frameCount = 2,
    .frames = {3, 4},
};

static Animation_t ANIM_VIKING_IDLE_LEFT = {
    .duration = 1.0f,
    .frameCount = 1,
    .frames = {5},
};

static Animation_t ANIM_VIKING_WALK_LEFT = {
    .duration = (1.0f / 3.75) * 7,
    .frameCount = 7,
    //.frames = {6, 7, 8, 9, 10, 9, 8, 7},
    .frames = {6, 7, 8, 9, 8, 7, 10},
};

static Animation_t ANIM_VIKING_WALK_FRONT = {
    .duration = (1.0f / 3.75) * 8,
    .frameCount = 8,
    .frames = {11, 12, 13, 14, 15, 14, 13, 12},
};

typedef struct {
  PlayerFacing_t facing;
  PlayerAnimState_t state;
  u8 frame;
  f64 seek;
  Animation_t* anim;
} AnimationState_t;

static AnimationState_t playerAnimationState = {
    .facing = FRONT,
    .state = IDLE,
    .anim = &ANIM_VIKING_IDLE_FRONT,

    // .facing = LEFT,
    // .state = WALK,
    // .anim = &ANIM_VIKING_WALK_LEFT,

    // .facing = FRONT,
    // .state = WALK,
    // .anim = &ANIM_VIKING_WALK_FRONT,

    .frame = 0,
    .seek = 0.0f,
};

u8 Animate(AnimationState_t* state, f64 deltaTime) {
  state->seek += deltaTime;
  state->seek = Math__mod(state->seek, state->anim->duration);
  state->frame = Math__map(state->seek, 0.0f, state->anim->duration, 0, state->anim->frameCount);
  u8 texId = state->anim->frames[state->frame];
  return texId;
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

  Audio__LoadAudioFile(audioFiles[AUDIO_AMBIENCE]);
  Audio__PlayAudio(AUDIO_AMBIENCE, true, 6.0f);
  Audio__LoadAudioFile(audioFiles[AUDIO_FOOTSTEPS]);
  Audio__LoadAudioFile(audioFiles[AUDIO_SET_WOOD_WALL]);

  Keyboard__RegisterCallback(keyboardCallback);
  Finger__RegisterCallback(fingerCallback);

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

  glm_vec3_copy((vec3){0, 0, 0}, instances[INSTANCE_FLOOR_0].pos);
  glm_vec3_copy((vec3){0, 0, 0}, instances[INSTANCE_FLOOR_0].rot);
  glm_vec3_copy(
      (vec3){PixelsToUnits(2632), PixelsToUnits(1721), 1},
      instances[INSTANCE_FLOOR_0].scale);
  instances[INSTANCE_FLOOR_0].texId = 0;
  instanceCount = 1;

  glm_vec3_copy((vec3){0, 0, 0}, instances[INSTANCE_PLAYER_1].pos);
  glm_vec3_copy((vec3){0, 0, 0}, instances[INSTANCE_PLAYER_1].rot);
  glm_vec3_copy(
      (vec3){PixelsToUnits(300), PixelsToUnits(450), 1},
      instances[INSTANCE_PLAYER_1].scale);
  instances[INSTANCE_PLAYER_1].texId = 4;
  instanceCount++;

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

static void keyboardCallback() {
  // LOG_DEBUGF(
  //     "SDL_KEY{UP,DOWN} state "
  //     "code %u location %u pressed %u alt %u "
  //     "ctrl %u shift %u meta %u",
  //     g_Keyboard__state.code,
  //     g_Keyboard__state.location,
  //     g_Keyboard__state.pressed,
  //     g_Keyboard__state.altKey,
  //     g_Keyboard__state.ctrlKey,
  //     g_Keyboard__state.shiftKey,
  //     g_Keyboard__state.metaKey);

  // character locomotion controls
  if (119 == g_Keyboard__state.location) {  // W
    playerAnimationState.facing = BACK;
    playerAnimationState.state = g_Keyboard__state.pressed ? WALK : IDLE;
    // playerAnimationState.anim = &ANIM_VIKING_WALK_BACK;
    playerAnimationState.anim = &ANIM_VIKING_WALK_FRONT;
  } else if (97 == g_Keyboard__state.location) {  // A
    playerAnimationState.facing = LEFT;
    playerAnimationState.state = g_Keyboard__state.pressed ? WALK : IDLE;
    playerAnimationState.anim = &ANIM_VIKING_WALK_LEFT;
    // instances[INSTANCE_PLAYER_1].scale[0] = +instances[INSTANCE_PLAYER_1].scale[0];
  } else if (115 == g_Keyboard__state.location) {  // S
    playerAnimationState.facing = FRONT;
    playerAnimationState.state = g_Keyboard__state.pressed ? WALK : IDLE;
    playerAnimationState.anim = &ANIM_VIKING_WALK_FRONT;
  } else if (100 == g_Keyboard__state.location) {  // D
    playerAnimationState.facing = RIGHT;
    playerAnimationState.state = g_Keyboard__state.pressed ? WALK : IDLE;
    playerAnimationState.anim = &ANIM_VIKING_WALK_LEFT;
    // instances[INSTANCE_PLAYER_1].scale[0] = -instances[INSTANCE_PLAYER_1].scale[0];
  }
  if (WALK == playerAnimationState.state) {
    Audio__ResumeAudio(AUDIO_FOOTSTEPS, false, 10.0f);
  } else if (IDLE == playerAnimationState.state) {
    Audio__StopAudio(AUDIO_FOOTSTEPS);

    if (BACK == playerAnimationState.facing) {
      // playerAnimationState.anim = &ANIM_VIKING_IDLE_BACK;
      playerAnimationState.anim = &ANIM_VIKING_IDLE_FRONT;
    } else if (LEFT == playerAnimationState.facing) {
      playerAnimationState.anim = &ANIM_VIKING_IDLE_LEFT;
    } else if (FRONT == playerAnimationState.facing) {
      playerAnimationState.anim = &ANIM_VIKING_IDLE_FRONT;
    } else if (RIGHT == playerAnimationState.facing) {
      // playerAnimationState.anim = &ANIM_VIKING_IDLE_RIGHT;
      playerAnimationState.anim = &ANIM_VIKING_IDLE_LEFT;
    }
  }
}

static void fingerCallback() {
  // LOG_DEBUGF(
  //     "SDL_FINGER state "
  //     "event %s "
  //     "clicks %u pressure %2.5f finger %u "
  //     "x %u y %u x_rel %d y_rel %d wheel_x %2.5f wheel_y %2.5f "
  //     "button_l %d button_m %d button_r %d button_x1 %d button_x2 %d ",
  //     (g_Finger__state.event == FINGER_UP       ? "UP"
  //      : g_Finger__state.event == FINGER_DOWN   ? "DOWN"
  //      : g_Finger__state.event == FINGER_MOVE   ? "MOVE"
  //      : g_Finger__state.event == FINGER_SCROLL ? "SCROLL"
  //                                        : ""),
  //     g_Finger__state.clicks,
  //     g_Finger__state.pressure,
  //     g_Finger__state.finger,
  //     g_Finger__state.x,
  //     g_Finger__state.y,
  //     g_Finger__state.x_rel,
  //     g_Finger__state.y_rel,
  //     g_Finger__state.wheel_x,
  //     g_Finger__state.wheel_y,
  //     g_Finger__state.button_l,
  //     g_Finger__state.button_m,
  //     g_Finger__state.button_r,
  //     g_Finger__state.button_x1,
  //     g_Finger__state.button_x2);

  if (FINGER_SCROLL == g_Finger__state.event) {
    // TODO: how to animate camera zoom with spring damping/smoothing?
    // TODO: how to move this into physics callback? or is it better not to?
    world.cam[2] += -g_Finger__state.wheel_y * PLAYER_ZOOM_SPEED /* deltaTime*/;
    isUBODirty[0] = true;
    isUBODirty[1] = true;
  }

  else if (FINGER_DOWN == g_Finger__state.event) {
    // TODO: how to move this into physics callback? or is it better not to?
    // TODO: animate player walk-to, before placing-down
    // TODO: convert window x,y to world x,y

    vec3 pos = (vec3){g_Finger__state.x, g_Finger__state.y, 0.0f};
    mat4 pvMatrix;
    glm_mat4_mul(ubo1.proj, ubo1.view, pvMatrix);
    vec4 viewport = (vec4){0, 0, s_Window.width, s_Window.height};
    vec3 dest;
    glm_unproject(pos, pvMatrix, viewport, dest);

    instances[instanceCount].pos[0] = dest[0];
    instances[instanceCount].pos[1] = dest[1];
    instances[instanceCount].pos[2] = 0.0f;  // dest[2];

    instances[instanceCount].scale[0] = PixelsToUnits(350 / 2);
    instances[instanceCount].scale[1] = PixelsToUnits(420 / 2);
    instances[instanceCount].scale[2] = 1.0f;
    instances[instanceCount].texId = 2;  // wood-wall 1
    instanceCount++;
    isVBODirty = true;

    Audio__PlayAudio(AUDIO_SET_WOOD_WALL, false, 1.0f);
  }
}

void physicsCallback(const f64 deltaTime) {
  // OnFixedUpdate(deltaTime);
  if (WALK == playerAnimationState.state) {
    if (LEFT == playerAnimationState.facing) {
      instances[INSTANCE_PLAYER_1].pos[0] -= PLAYER_WALK_SPEED * deltaTime;
    } else if (RIGHT == playerAnimationState.facing) {
      instances[INSTANCE_PLAYER_1].pos[0] += PLAYER_WALK_SPEED * deltaTime;
    } else if (BACK == playerAnimationState.facing) {
      instances[INSTANCE_PLAYER_1].pos[1] -= PLAYER_WALK_SPEED * deltaTime;
    } else if (FRONT == playerAnimationState.facing) {
      instances[INSTANCE_PLAYER_1].pos[1] += PLAYER_WALK_SPEED * deltaTime;
    }
    isVBODirty = true;

    world.cam[0] = instances[INSTANCE_PLAYER_1].pos[0];
    world.cam[1] = instances[INSTANCE_PLAYER_1].pos[1];
    world.look[0] = instances[INSTANCE_PLAYER_1].pos[0];
    world.look[1] = instances[INSTANCE_PLAYER_1].pos[1];
    isUBODirty[0] = true;
    isUBODirty[1] = true;
  }
}

static u8 newTexId;
static void renderCallback(const f64 deltaTime) {
  // OnUpdate(deltaTime);

  // character frame animation
  newTexId = Animate(&playerAnimationState, deltaTime);
  if (instances[1].texId != newTexId) {
    instances[1].texId = newTexId;
    isVBODirty = true;
  }

  if (isVBODirty) {
    isVBODirty = false;

    s_Vulkan.m_instanceCount = instanceCount;
    Vulkan__UpdateVertexBuffer(&s_Vulkan, 1, sizeof(instances), instances);
  }

  if (isUBODirty[s_Vulkan.m_currentFrame]) {
    isUBODirty[s_Vulkan.m_currentFrame] = false;

    glm_lookat(
        world.cam,
        world.look,
        VEC3_Y_UP,  // Y-axis points upwards (GLM default)
        ubo1.view);

    s_Vulkan.m_aspectRatio = world.aspect;  // sync viewport

    glm_perspective(
        glm_rad(45.0f),  // half the actual 90deg fov
        world.aspect,
        0.1f,  // TODO: adjust clipping range for z depth?
        10.0f,
        ubo1.proj);

    // glm_ortho(-0.5f, +0.5f, -0.5f, +0.5f, 0.1f, 10.0f, ubo1.proj);
    glm_vec2_copy(world.user1, ubo1.user1);
    glm_vec2_copy(world.user2, ubo1.user2);

    // TODO: not sure i make use of one UBO per frame, really
    Vulkan__UpdateUniformBuffer(&s_Vulkan, s_Vulkan.m_currentFrame, &ubo1);
  }
}
