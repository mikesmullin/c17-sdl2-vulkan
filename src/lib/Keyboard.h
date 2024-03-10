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
} KeyboardState_t;

extern KeyboardState_t g_Keyboard__state;

void Keyboard__OnInput(const SDL_Event* event);
void Keyboard__RegisterCallback(void(*cb));
void Keyboard__DispatchCallbacks();

#endif