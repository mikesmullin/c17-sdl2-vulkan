#include "Window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "Base.h"
#include "Gamepad.h"
#include "Keyboard.h"
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
  // use the smaller of the original vs. aspect dimension
  const u32 targetWidth = MATH_MIN((f32)width, height * self->vulkan->m_aspectRatio);
  const u32 targetHeight = MATH_MIN((f32)height, width / self->vulkan->m_aspectRatio);

  // and then center it to provide the illusion of aspect ratio
  const u32 left = (width - targetWidth) / 2;
  const u32 top = (height - targetHeight) / 2;

  self->vulkan->m_windowWidth = width;
  self->vulkan->m_windowHeight = height;
  self->vulkan->m_viewportX = left;
  self->vulkan->m_viewportY = top;
  self->vulkan->m_viewportWidth = targetWidth;
  self->vulkan->m_viewportHeight = targetHeight;
  self->vulkan->m_bufferWidth = width;
  self->vulkan->m_bufferHeight = height;
  self->vulkan->m_framebufferResized = true;
}

void Window__RenderLoop(
    Window_t* self,
    const int physicsFps,
    const int renderFps,
    void (*physicsCallback)(const float),
    void (*renderCallback)(const float)) {
  // const std::chrono::duration<double, std::milli> physicsInterval(1000.0f / physicsFps);
  // const std::chrono::duration<double, std::milli> renderInterval(1000.0f / renderFps);
  // auto lastPhysics = std::chrono::high_resolution_clock::now();
  // auto lastRender = std::chrono::high_resolution_clock::now();
  // auto currentTime = std::chrono::high_resolution_clock::now();
  // std::chrono::duration<double, std::milli> elapsedPhysics = currentTime - lastPhysics;
  // std::chrono::duration<double, std::milli> elapsedRender = currentTime - lastRender;

  float deltaTime = 0;
  // u8 frameCount = 0;
  // u8 fpsAvg = 0;
  // char title[255];
  SDL_Event e;
  while (!self->quit) {
    // Render update
    // currentTime = std::chrono::high_resolution_clock::now();
    // elapsedRender = currentTime - lastRender;
    // if (elapsedRender > renderInterval) {
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

      // SDL_UpdateWindowSurface(window);
    }

    if (!self->vulkan->m_minimized) {
      // Physics update
      // currentTime = std::chrono::high_resolution_clock::now();
      // elapsedPhysics = currentTime - lastPhysics;
      // if (elapsedPhysics > physicsInterval) {
      // deltaTime =
      //     std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastPhysics)
      //         .count();
      // lastPhysics = currentTime;

      physicsCallback(deltaTime);
      // }

      // render
      Vulkan__AwaitNextFrame(self->vulkan);

      // currentTime = std::chrono::high_resolution_clock::now();
      // deltaTime =
      //     std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastRender)
      //         .count();
      // lastRender = currentTime;

      renderCallback(deltaTime);
      Vulkan__DrawFrame(self->vulkan);

      // frameCount++;
      // if (frameCount >= renderFps) {
      //   fpsAvg = 1 / (deltaTime / frameCount);
      //   // if titlebar updates are tracking with the wall clock seconds hand, then loop is
      //   on-time
      //   // the value shown is potential frames (ie. accounts for spare cycles)
      //   sprintf(title, "%s | pFPS: %u", this->title, fpsAvg);
      //   SDL_SetWindowTitle(window, title);
      //   frameCount = 0;
      // }
      // }
    }

    // sleep to control the frame rate
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}