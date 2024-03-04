#ifndef WINDOW_H
#define WINDOW_H

#include "Base.h"
typedef struct SDL_Window SDL_Window;
#include "Vulkan.h"

typedef struct {
  u32 width;
  u32 height;
} DrawableArea_t;

typedef enum {
  WINDOW_ERROR_NONE = 0,
  WINDOW_ERROR_CREATE_WINDOW_FAILED = 1,
  WINDOW_ERROR_GET_INSTANCE_EXTENSIONS_FAILED_COUNT = 2,
  WINDOW_ERROR_GET_INSTANCE_EXTENSIONS_FAILED_WRITE = 3,
  WINDOW_ERROR_BIND_FAILED = 4,
} Window__Error_t;

extern const char* ckp_Window__ERROR_MESSAGES[];

typedef struct {
  bool quit;
  SDL_Window* window;
  char* title;
  u16 width;
  u16 height;
  Vulkan_t* vulkan;
} Window_t;

void Window__New(Window_t* self, char* title, u16 width, u16 height, Vulkan_t* vulkan);
Window__Error_t Window__Begin(Window_t* self);
void Window__Bind(Window_t* self);
void Window__GetDrawableAreaExtentBounds(Window_t* self, DrawableArea_t* area);
void Window__KeepAspectRatio(Window_t* self, const u32 width, const u32 height);
void Window__RenderLoop(Window_t* self);
void Window__Shutdown(Window_t* self);

#endif