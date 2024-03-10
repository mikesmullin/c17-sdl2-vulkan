#include "Finger.h"

FingerState_t g_Finger__state = {};

#define FINGER_CALLBACKS_CAP 10
static const void (*CALLBACKS[FINGER_CALLBACKS_CAP])();
static u8 callbackCount = 0;

void Finger__RegisterCallback(void(*cb)) {
  ASSERT(callbackCount < FINGER_CALLBACKS_CAP);
  CALLBACKS[callbackCount] = cb;
  callbackCount++;
}

void Finger__DispatchCallbacks() {
  for (u8 i = 0; i < callbackCount; i++) {
    CALLBACKS[i]();
  }
}

void Finger__OnInput(const SDL_Event* event) {
  g_Finger__state.event = FINGER_NONE;
  g_Finger__state.clicks = 0;
  g_Finger__state.pressure = 0.0f;
  g_Finger__state.finger = 0;
  g_Finger__state.x = 0;
  g_Finger__state.y = 0;
  g_Finger__state.x_rel = 0;
  g_Finger__state.y_rel = 0;
  g_Finger__state.button_l = false;
  g_Finger__state.button_m = false;
  g_Finger__state.button_r = false;
  g_Finger__state.button_x1 = false;
  g_Finger__state.button_x2 = false;
  g_Finger__state.wheel_x = 0.0f;
  g_Finger__state.wheel_y = 0.0f;

  switch (event->type) {
    case SDL_MOUSEMOTION:
      g_Finger__state.event = FINGER_MOVE;
      g_Finger__state.x = event->motion.x;
      g_Finger__state.y = event->motion.y;
      g_Finger__state.x_rel = event->motion.xrel;
      g_Finger__state.y_rel = event->motion.yrel;

      g_Finger__state.button_l = event->motion.state & SDL_BUTTON_LMASK;
      g_Finger__state.button_m = event->motion.state & SDL_BUTTON_MMASK;
      g_Finger__state.button_r = event->motion.state & SDL_BUTTON_RMASK;
      g_Finger__state.button_x1 = event->motion.state & SDL_BUTTON_X1MASK;
      g_Finger__state.button_x2 = event->motion.state & SDL_BUTTON_X2MASK;

      Finger__DispatchCallbacks();
      break;

    case SDL_FINGERMOTION:
      g_Finger__state.event = FINGER_MOVE;
      g_Finger__state.x = event->tfinger.x;
      g_Finger__state.y = event->tfinger.y;
      g_Finger__state.x_rel = event->tfinger.dx;
      g_Finger__state.y_rel = event->tfinger.dy;
      g_Finger__state.finger = event->tfinger.fingerId;

      Finger__DispatchCallbacks();
      break;

    case SDL_MOUSEWHEEL:
      g_Finger__state.event = FINGER_SCROLL;
      // g_Finger__state.wheel_x = event->wheel.x;
      // g_Finger__state.wheel_y = event->wheel.y;
      g_Finger__state.wheel_x = event->wheel.preciseX;
      g_Finger__state.wheel_y = event->wheel.preciseY;

      Finger__DispatchCallbacks();
      break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      g_Finger__state.event = SDL_MOUSEBUTTONDOWN == event->type ? FINGER_DOWN : FINGER_UP;
      g_Finger__state.x = event->button.x;
      g_Finger__state.y = event->button.y;

      g_Finger__state.clicks = event->button.clicks;
      g_Finger__state.button_l = event->button.button == SDL_BUTTON_LEFT;
      g_Finger__state.button_m = event->button.button == SDL_BUTTON_MIDDLE;
      g_Finger__state.button_r = event->button.button == SDL_BUTTON_RIGHT;
      g_Finger__state.button_x1 = event->button.button == SDL_BUTTON_X1;
      g_Finger__state.button_x2 = event->button.button == SDL_BUTTON_X2;

      Finger__DispatchCallbacks();
      break;

    case SDL_FINGERDOWN:
    case SDL_FINGERUP:
      g_Finger__state.event = SDL_FINGERDOWN == event->type ? FINGER_DOWN : FINGER_UP;
      g_Finger__state.x = event->tfinger.x;
      g_Finger__state.y = event->tfinger.y;
      g_Finger__state.pressure = event->tfinger.pressure;
      g_Finger__state.finger = event->tfinger.fingerId;
      g_Finger__state.button_l = SDL_FINGERDOWN == event->type;

      Finger__DispatchCallbacks();
      break;
  }
}
