#ifndef SDL_H
#define SDL_H

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <stdbool.h>

extern bool SDL__enableAudio;
extern bool SDL__enableGamepad;
extern bool SDL__enableVideo;

void SDL__Init();
void SDL__Shutdown();

#endif