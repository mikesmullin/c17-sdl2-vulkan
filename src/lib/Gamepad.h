#ifndef GAMEPAD_H
#define GAMEPAD_H

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "Base.h"

#define GAMEPAD__AXES_CAP 2
#define GAMEPAD__BUTTON_CAP 4

typedef struct {
  u8 m_index;
  SDL_Joystick* m_joystick;
  f32 fm_axes[GAMEPAD__AXES_CAP];
  bool bm_buttons[GAMEPAD__BUTTON_CAP];
} Gamepad_t;

void Gamepad__New(Gamepad_t* self, const uint8_t index);
void Gamepad__Shutdown(Gamepad_t* self);
const char* Gamepad__GetControllerName(Gamepad_t* self);
void Gamepad__Open(Gamepad_t* self);
void Gamepad__OnInput(const SDL_Event* event);

#endif