#include "SDL.h"

#include <stdio.h>

#include "Base.h"

const char* ckp_SDL__ERROR_MESSAGES[] = {
    "None\n",
    "SDL_Init() Failed. %s\n",
};

static bool bs_SDL__UseAudio = false;
static bool bs_SDL__UseGamepad = false;
static bool bs_SDL__UseVideo = false;

void SDL__EnableAudio() {
  bs_SDL__UseAudio = true;
}

void SDL__EnableGamepad() {
  bs_SDL__UseGamepad = true;
}

void SDL__EnableVideo() {
  bs_SDL__UseVideo = true;
}

SDL__Error_t SDL__Init() {
  u32 flags = 0;
  if (bs_SDL__UseAudio) {
    flags = flags | SDL_INIT_AUDIO;
  }
  if (bs_SDL__UseGamepad) {
    flags = flags | SDL_INIT_JOYSTICK;
  }
  if (bs_SDL__UseVideo) {
    flags = flags | SDL_INIT_VIDEO;
  }
  if (0 != SDL_Init(flags)) {
    RETURN_ERROR(SDL_ERROR_INIT_FAILED, ckp_SDL__ERROR_MESSAGES, SDL_GetError())
  }
  return SDL_ERROR_NONE;
}

void SDL__Shutdown() {
  SDL_Quit();
}