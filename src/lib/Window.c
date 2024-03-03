#include "Window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "Vulkan.h"

const char* ckp_Window__ERROR_MESSAGES[] = {
    "None\n",
    "SDL_CreateWindow() failed. %s\n",
    "SDL_Vulkan_GetInstanceExtensions() failed to count. %s\n",
    "SDL_Vulkan_GetInstanceExtensions() failed to write. %s\n",
};

void Window__New(Window_t* self, char* title, u8 width, u8 height, Vulkan_t* vulkan) {
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
      SDL_GetError());

  // list required extensions, according to SDL window manager
  ASSERT_ERROR(
      SDL_TRUE == SDL_Vulkan_GetInstanceExtensions(
                      self->window,
                      &self->vulkan->m_requiredDriverExtensionCount,
                      NULL),
      WINDOW_ERROR_GET_INSTANCE_EXTENSIONS_FAILED_COUNT,
      ckp_Window__ERROR_MESSAGES,
      SDL_GetError());

  ASSERT_ERROR(
      SDL_TRUE == SDL_Vulkan_GetInstanceExtensions(
                      self->window,
                      &self->vulkan->m_requiredDriverExtensionCount,
                      (const char**)&self->vulkan->m_requiredDriverExtensions),
      WINDOW_ERROR_GET_INSTANCE_EXTENSIONS_FAILED_WRITE,
      ckp_Window__ERROR_MESSAGES,
      SDL_GetError());

  return WINDOW_ERROR_NONE;
}

void Window__Bind(Window_t* self) {
}

void Window__KeepAspectRatio(Window_t* self) {
}

void Window__RenderLoop(Window_t* self) {
}

void Window__Shutdown(Window_t* self) {
  SDL_DestroyWindow(self->window);
}