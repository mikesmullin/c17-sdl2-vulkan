#include "SDL.h"

#include "Base.h"

void SDL__Init() {
  u32 flags;
  ASSERT_CONTEXT(
      (0 == SDL_Init(flags = SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_VIDEO)),
      "SDL Error: %s",
      SDL_GetError())
}

void SDL__Shutdown() {
  SDL_Quit();
}