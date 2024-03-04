#include "Window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "Base.h"
#include "Vulkan.h"

const char* ckp_Window__ERROR_MESSAGES[] = {
    "None\n",
    "SDL_CreateWindow() failed. %s\n",
    "SDL_Vulkan_GetInstanceExtensions() failed to count. %s\n",
    "SDL_Vulkan_GetInstanceExtensions() failed to write. %s\n",
    "SDL_Vulkan_CreateSurface() failed to bind Window Surface to Vulkan. %s\n",
};

void Window__New(Window_t* self, char* title, u16 width, u16 height, Vulkan_t* vulkan) {
  self->quit = false;
  self->window = NULL;
  self->width = width;
  self->height = height;
  self->title = title;
  self->vulkan = vulkan;
}

Window__Error_t Window__Begin(Window_t* self) {
  self->window = SDL_CreateWindow(
      self->title,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      self->width,
      self->height,
      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE /* | SDL_WINDOW_SHOWN*/);
  ASSERT_ERROR(
      NULL != self->window,
      WINDOW_ERROR_CREATE_WINDOW_FAILED,
      ckp_Window__ERROR_MESSAGES,
      SDL_GetError())

  // list required extensions, according to SDL window manager
  ASSERT_ERROR(
      SDL_TRUE == SDL_Vulkan_GetInstanceExtensions(
                      self->window,
                      &self->vulkan->m_requiredDriverExtensionCount,
                      NULL),
      WINDOW_ERROR_GET_INSTANCE_EXTENSIONS_FAILED_COUNT,
      ckp_Window__ERROR_MESSAGES,
      SDL_GetError())

  ASSERT_ERROR(
      SDL_TRUE == SDL_Vulkan_GetInstanceExtensions(
                      self->window,
                      &self->vulkan->m_requiredDriverExtensionCount,
                      //(const char**)&self->vulkan->m_requiredDriverExtensions),
                      self->vulkan->m_requiredDriverExtensions),
      WINDOW_ERROR_GET_INSTANCE_EXTENSIONS_FAILED_WRITE,
      ckp_Window__ERROR_MESSAGES,
      SDL_GetError())

  return WINDOW_ERROR_NONE;
}

void Window__Bind(Window_t* self) {
  // ask SDL to bind our Vulkan surface to the window surface
  SDL_Vulkan_CreateSurface(self->window, self->vulkan->m_instance, &self->vulkan->m_surface);
  ASSERT_ERROR(
      self->vulkan->m_surface,
      WINDOW_ERROR_BIND_FAILED,
      ckp_Window__ERROR_MESSAGES,
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

void Window__RenderLoop(Window_t* self) {
}

void Window__Shutdown(Window_t* self) {
  SDL_DestroyWindow(self->window);
}