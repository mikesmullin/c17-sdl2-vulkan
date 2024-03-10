#ifndef WINDOW_H
#define WINDOW_H

#include "Base.h"
typedef struct SDL_Window SDL_Window;
#include "Vulkan.h"

typedef struct {
  u32 width;
  u32 height;
} DrawableArea_t;

typedef struct {
  bool quit;
  SDL_Window* window;
  char* title;
  u16 width;
  u16 height;
  Vulkan_t* vulkan;
} Window_t;

void Window__New(Window_t* self, char* title, u16 width, u16 height, Vulkan_t* vulkan);
void Window__Begin(Window_t* self);
void Window__Bind(Window_t* self);
void Window__Shutdown(Window_t* self);
void Window__GetDrawableAreaExtentBounds(Window_t* self, DrawableArea_t* area);
void Window__KeepAspectRatio(Window_t* self, const u32 width, const u32 height);
void Window__RenderLoop(
    Window_t* self,
    const int physicsFps,
    const int renderFps,
    void (*physicsCallback)(const f64),
    void (*renderCallback)(const f64));

#endif