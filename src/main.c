#include <stdio.h>
#include <time.h>

#include "lib/Audio.h"
#include "lib/Gamepad.h"
#include "lib/Keyboard.h"
#include "lib/SDL.h"
#include "lib/Vulkan.h"
#include "lib/Window.h"

static char* WINDOW_TITLE = "main";
static u16 WINDOW_WIDTH = 800;
static u16 WINDOW_HEIGHT = 800;

static Vulkan_t s_Vulkan;
static Window_t s_Window;

int main() {
  printf("begin main.\n");

  // initialize random seed using current time
  srand(time(NULL));

  Vulkan__InitDriver(&s_Vulkan);

  SDL__EnableAudio();
  SDL__EnableGamepad();
  SDL__EnableVideo();
  Window__New(&s_Window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, &s_Vulkan);
  SDL__Init();
  Audio__Init();

  Gamepad_t gamePad1;
  Gamepad__New(&gamePad1, 0);
  LOG_INFOF(
      "Controller Id: %d, Name: %s\n",
      gamePad1.m_index,
      Gamepad__GetControllerName(&gamePad1));
  Gamepad__Open(&gamePad1);

  Window__Begin(&s_Window);

  Vulkan__AssertDriverValidationLayersSupported(&s_Vulkan);

#if OS_MAC == 1
  // enable MoltenVK support for MacOS cross-platform support
  s_Vulkan.m_requiredDriverExtensions[s_Vulkan.m_requiredDriverExtensionCount++] =
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
#endif
  Vulkan__AssertDriverExtensionsSupported(&s_Vulkan);

  // Vulkan__Error_t e2 = Vulkan__Init1();
  // ASSERT_ERROR(e2 == 0, e2, ckp_Vulkan__ERROR_MESSAGES, NULL)

  bool quit = false;
  SDL_Event e;
  while (!quit) {
    while (SDL_PollEvent(&e) > 0) {
      switch (e.type) {
        case SDL_QUIT:
          quit = true;
          break;
      }

      Gamepad__OnInput(&e);
      Keyboard__OnInput(&e);
    }
  }

  Audio__Shutdown();
  Window__Shutdown(&s_Window);

  printf("done.\n");
  return 0;
}