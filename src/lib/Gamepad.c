#include "Gamepad.h"

#include "Base.h"
#include "Math.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define GAMEPAD__REGISTRY_CAP 4
static Gamepad_t* s_registry[GAMEPAD__REGISTRY_CAP];

void Gamepad__New(Gamepad_t* self, const u8 index) {
  self->m_index = index;
  s_registry[index] = self;
}

void Gamepad__Shutdown(Gamepad_t* self) {
  SDL_JoystickClose(self->m_joystick);
}

const char* Gamepad__GetControllerName(Gamepad_t* self) {
  return SDL_JoystickNameForIndex(self->m_index);
}

void Gamepad__Open(Gamepad_t* self) {
  self->m_joystick = SDL_JoystickOpen(self->m_index);
}

#define GAMEPAD_AXIS_MIN -32768
#define GAMEPAD_AXIS_MAX 32767

void Gamepad__OnInput(const SDL_Event* event) {
  Gamepad_t* g;
  switch (event->type) {
    case SDL_JOYAXISMOTION:
      g = s_registry[event->jdevice.which];
      g->fm_axes[event->jaxis.axis] =
          Math__map(event->jaxis.value, GAMEPAD_AXIS_MIN, GAMEPAD_AXIS_MAX, -1.0f, 1.0f);

      // LOG_DEBUGF(
      //     "JOYAXISMOTION device: %i, axis: %i, value: %i, mapped: %5.3f",
      //     event->jdevice.which,
      //     event->jaxis.axis,
      //     event->jaxis.value,
      //     g->fm_axes[event->jaxis.axis]);
      break;

    case SDL_JOYBUTTONDOWN:
      g = s_registry[event->jdevice.which];
      g->bm_buttons[event->jbutton.button] = true;

      // LOG_DEBUGF(
      //     "JOYBUTTONDOWN device: %i, button: %i",
      //     event->jdevice.which,
      //     event->jbutton.button);
      break;

    case SDL_JOYBUTTONUP:
      g = s_registry[event->jdevice.which];
      g->bm_buttons[event->jbutton.button] = false;

      // LOG_DEBUGF(
      //     "JOYBUTTONUP device: %i, button: %i",
      //     event->jdevice.which,
      //     event->jbutton.button);
      break;
  }
}