#ifndef SDL_H
#define SDL_H

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

typedef enum {
  SDL_ERROR_NONE = 0,
  SDL_ERROR_INIT_FAILED = 1,
} SDL__Error_t;

extern const char* ckp_SDL__ERROR_MESSAGES[];

void SDL__EnableAudio();
void SDL__EnableGamepad();
void SDL__EnableVideo();

SDL__Error_t SDL__Init();
void SDL__Shutdown();

#endif