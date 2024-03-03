#ifndef KEYBOARD_H
#define KEYBOARD_H

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "Base.h"

typedef struct {
  bool pressed;
  bool altKey;
  bool ctrlKey;
  bool shiftKey;
  bool metaKey;
  u8 code;
  u8 location;
} KeyboardState;

void Keyboard__OnInput(const SDL_Event* event);

#endif