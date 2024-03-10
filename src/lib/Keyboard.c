#include "Keyboard.h"

KeyboardState_t g_Keyboard__state = {
    .pressed = false,
    .altKey = false,
    .ctrlKey = false,
    .shiftKey = false,
    .metaKey = false,
    .code = 0,
    .location = 0,
};

#define KEYBOARD_CALLBACKS_CAP 10
static const void (*CALLBACKS[KEYBOARD_CALLBACKS_CAP])();
static u8 callbackCount = 0;

void Keyboard__RegisterCallback(void(*cb)) {
  ASSERT(callbackCount < KEYBOARD_CALLBACKS_CAP);
  CALLBACKS[callbackCount] = cb;
  callbackCount++;
}

void Keyboard__DispatchCallbacks() {
  for (u8 i = 0; i < callbackCount; i++) {
    CALLBACKS[i]();
  }
}

void Keyboard__OnInput(const SDL_Event* event) {
  switch (event->type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      switch (event->key.state) {
        case SDL_PRESSED:
          g_Keyboard__state.pressed = true;
          break;
        case SDL_RELEASED:
        default:
          g_Keyboard__state.pressed = false;
          break;
      }
      g_Keyboard__state.altKey = event->key.keysym.mod & (KMOD_LALT | KMOD_RALT);
      g_Keyboard__state.ctrlKey = event->key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL);
      g_Keyboard__state.shiftKey = event->key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT);
      g_Keyboard__state.metaKey = event->key.keysym.mod & (KMOD_LGUI | KMOD_RGUI);
      // event->key.repeat;
      g_Keyboard__state.code = event->key.keysym.scancode;  // for WASD, arrow keys
      g_Keyboard__state.location = event->key.keysym.sym;   // for everything else

      // LOG_DEBUGF(
      //     "SDL_KEY{UP,DOWN} state "
      //     "code %u location %u pressed %u alt %u "
      //     "ctrl %u shift %u meta %u",
      //     g_Keyboard__state.code,
      //     g_Keyboard__state.location,
      //     g_Keyboard__state.pressed,
      //     g_Keyboard__state.altKey,
      //     g_Keyboard__state.ctrlKey,
      //     g_Keyboard__state.shiftKey,
      //     g_Keyboard__state.metaKey);

      Keyboard__DispatchCallbacks();

      break;
  }
}