#include "Window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <stdio.h>

#include "Base.h"
#include "Finger.h"
#include "Gamepad.h"
#include "Keyboard.h"
#include "Timer.h"
#include "Vulkan.h"

void Window__New(Window_t* self, char* title, u16 width, u16 height, Vulkan_t* vulkan) {
  self->quit = false;
  self->window = NULL;
  self->width = width;
  self->height = height;
  self->title = title;
  self->vulkan = vulkan;
}

void Window__Begin(Window_t* self) {
  self->window = SDL_CreateWindow(
      self->title,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      self->width,
      self->height,
      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE /* | SDL_WINDOW_SHOWN*/);
  ASSERT_CONTEXT(NULL != self->window, "SDL_CreateWindow() failed. SDL Error: %s", SDL_GetError())

  // list required extensions, according to SDL window manager
  ASSERT_CONTEXT(
      SDL_TRUE == SDL_Vulkan_GetInstanceExtensions(
                      self->window,
                      &self->vulkan->m_requiredDriverExtensionsCount,
                      NULL),
      "SDL_Vulkan_GetInstanceExtensions() failed to count. SDL Error: %s",
      SDL_GetError())

  ASSERT_CONTEXT(
      SDL_TRUE == SDL_Vulkan_GetInstanceExtensions(
                      self->window,
                      &self->vulkan->m_requiredDriverExtensionsCount,
                      //(const char**)&self->vulkan->m_requiredDriverExtensions),
                      self->vulkan->m_requiredDriverExtensions),
      "SDL_Vulkan_GetInstanceExtensions() failed to write. Count: %u, SDL Error: %s",
      self->vulkan->m_requiredDriverExtensionsCount,
      SDL_GetError())
}

void Window__Shutdown(Window_t* self) {
  SDL_DestroyWindow(self->window);
}

void Window__Bind(Window_t* self) {
  // ask SDL to bind our Vulkan surface to the window surface
  SDL_Vulkan_CreateSurface(self->window, self->vulkan->m_instance, &self->vulkan->m_surface);
  ASSERT_CONTEXT(
      self->vulkan->m_surface,
      "SDL_Vulkan_CreateSurface() failed. SDL Error: %s",
      SDL_GetError())
}

/**
 * for use when telling Vulkan what its drawable area (extent bounds) are, according to SDL
 * window. this may differ from what we requested, and must be less than the physical device
 * capability.
 */
void Window__GetDrawableAreaExtentBounds(Window_t* self, DrawableArea_t* area) {
  int tmpWidth, tmpHeight = 0;
  SDL_Vulkan_GetDrawableSize(self->window, &tmpWidth, &tmpHeight);
  area->width = tmpWidth;
  area->height = tmpHeight;
}

void Window__KeepAspectRatio(Window_t* self, const u32 width, const u32 height) {
  if (width != self->width || height != self->height) {
    self->vulkan->m_framebufferResized = true;
  }

  // use the smaller of the original vs. aspect dimension
  const u32 targetWidth = MATH_MIN((f32)width, height * self->vulkan->m_aspectRatio);
  const u32 targetHeight = MATH_MIN((f32)height, width / self->vulkan->m_aspectRatio);

  // and then center it to provide the illusion of aspect ratio
  const u32 left = (width - targetWidth) / 2;
  const u32 top = (height - targetHeight) / 2;

  self->width = width;
  self->height = height;
  self->vulkan->m_windowWidth = width;
  self->vulkan->m_windowHeight = height;
  self->vulkan->m_viewportX = left;
  self->vulkan->m_viewportY = top;
  self->vulkan->m_viewportWidth = targetWidth;
  self->vulkan->m_viewportHeight = targetHeight;
  self->vulkan->m_bufferWidth = width;
  self->vulkan->m_bufferHeight = height;
}

void Window__RenderLoop(
    Window_t* self,
    const int physicsFps,
    const int renderFps,
    void (*physicsCallback)(const f64),
    void (*renderCallback)(const f64)) {
  const u8 physicsInterval = 1000 / physicsFps;
  const u8 renderInterval = 1000 / renderFps;
  u64 currentTime = Timer__NowMilliseconds();
  u64 lastPhysics = currentTime - physicsInterval;
  u64 lastRender = currentTime - renderInterval;
  u16 elapsedPhysics = 0;
  u16 elapsedRender = 0;
  f64 deltaTime = 0.0f;
  u8 frameCount = 0;
  u8 fpsAvg = 0;
  char title[100];
  SDL_Event e;
  while (!self->quit) {
    // input handling
    while (SDL_PollEvent(&e) > 0) {
      switch (e.type) {
        case SDL_WINDOWEVENT:
          switch (e.window.event) {
            case SDL_WINDOWEVENT_MINIMIZED:
              self->vulkan->m_minimized = true;
              break;

            case SDL_WINDOWEVENT_RESTORED:
              self->vulkan->m_minimized = false;
              self->vulkan->m_maximized = false;
              break;

            case SDL_WINDOWEVENT_MAXIMIZED:
              self->vulkan->m_maximized = true;
              break;

            // case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED:
              self->vulkan->m_minimized = false;
              Window__KeepAspectRatio(self, e.window.data1, e.window.data2);
              break;
          }
          break;

        case SDL_QUIT:
          self->quit = true;
          break;
      }

      Gamepad__OnInput(&e);
      Keyboard__OnInput(&e);
      Finger__OnInput(&e);

      // SDL_UpdateWindowSurface(window);
    }

    if (!self->vulkan->m_minimized) {
      // Physics update
      currentTime = Timer__NowMilliseconds();
      elapsedPhysics = currentTime - lastPhysics;
      if (elapsedPhysics > physicsInterval) {
        deltaTime = 1.0f / MATH_MAX(1, (currentTime - lastPhysics));
        lastPhysics = currentTime;

        physicsCallback(deltaTime);
      }

      // Render update
      currentTime = Timer__NowMilliseconds();
      elapsedRender = currentTime - lastRender;
      if (elapsedRender > renderInterval) {
        Vulkan__AwaitNextFrame(self->vulkan);

        currentTime = Timer__NowMilliseconds();
        deltaTime = 1.0f / MATH_MAX(1, (currentTime - lastRender));
        lastRender = currentTime;

        renderCallback(deltaTime);
        Vulkan__DrawFrame(self->vulkan);

        frameCount++;
        if (frameCount >= renderFps) {
          fpsAvg = 1 / (deltaTime / frameCount);
          // if titlebar updates are tracking with the wall clock seconds hand, then loop is on-time
          // the value shown is potential frames (ie. accounts for spare cycles)
          sprintf(title, "%s | pFPS: %u", self->title, fpsAvg);
          SDL_SetWindowTitle(self->window, title);
          frameCount = 0;
        }
      }
    }

    // sleep to control the frame rate
    SLEEP(1);
  }
}