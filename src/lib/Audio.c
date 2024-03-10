#define SDL_MAIN_HANDLED

#include "Audio.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <cmixer.h>
#include <string.h>

#include "Base.h"

static SDL_mutex* audio_mutex;

static void _lock_handler(cm_Event* e) {
  if (e->type == CM_EVENT_LOCK) {
    SDL_LockMutex(audio_mutex);
  }
  if (e->type == CM_EVENT_UNLOCK) {
    SDL_UnlockMutex(audio_mutex);
  }
}

static void _audio_callback(void* udata, Uint8* stream, int size) {
  cm_process((cm_Int16*)stream, size / 2);
}

#define MAX_AUDIO_SOURCES 25
static u8 s_AudioSourcesSize = 0;
static cm_Source* s_AudioSources[MAX_AUDIO_SOURCES];
static u8 s_Device;

void Audio__Init() {
  SDL_AudioSpec fmt, got;
  cm_Source* src;

  // init SDL
  // SDL_Init(SDL_INIT_AUDIO);
  audio_mutex = SDL_CreateMutex();

  // init SDL audio
  memset(&fmt, 0, sizeof(fmt));
  fmt.freq = 44100;
  fmt.format = AUDIO_S16;
  fmt.channels = 2;
  fmt.samples = 1024;
  fmt.callback = _audio_callback;

  ASSERT_CONTEXT(
      !!(s_Device = SDL_OpenAudioDevice(NULL, 0, &fmt, &got, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE)),
      "SDL Error: %s",
      SDL_GetError());

  // init library
  cm_init(got.freq);
  cm_set_lock(_lock_handler);
  cm_set_master_gain(0.5);

  // start audio
  SDL_PauseAudioDevice(s_Device, 0);
}

void Audio__Shutdown() {
  for (u8 i = 0; i < s_AudioSourcesSize; i++) {
    cm_destroy_source(s_AudioSources[i]);
  }
  SDL_CloseAudioDevice(s_Device);
}

void Audio__LoadAudioFile(const char* path) {
  ASSERT_CONTEXT(
      s_AudioSourcesSize <= MAX_AUDIO_SOURCES,
      "Failed to load audio file %s. Raise MAX_AUDIO_SOURCES.",
      path)

  cm_Source* src = cm_new_source_from_file(path);
  ASSERT_CONTEXT(src, "Failed to load audio file %s", cm_get_error())

  s_AudioSources[s_AudioSourcesSize] = src;
  LOG_INFOF("Audio file loaded. idx: %u, path: %s", s_AudioSourcesSize, path);
  s_AudioSourcesSize++;
}

void Audio__PlayAudio(const int id, const bool loop, const double gain) {
  if (cm_get_state(s_AudioSources[id]) == CM_STATE_PLAYING) {
    cm_stop(s_AudioSources[id]);
  }
  cm_set_gain(s_AudioSources[id], gain);
  cm_set_loop(s_AudioSources[id], loop ? 1 : 0);
  cm_play(s_AudioSources[id]);
}

void Audio__ResumeAudio(const int id, const bool loop, const double gain) {
  if (cm_get_state(s_AudioSources[id]) == CM_STATE_PLAYING) {
    return;
  }
  cm_set_gain(s_AudioSources[id], gain);
  cm_set_loop(s_AudioSources[id], loop ? 1 : 0);
  cm_play(s_AudioSources[id]);
}

void Audio__StopAudio(const int id) {
  if (cm_get_state(s_AudioSources[id]) == CM_STATE_PLAYING) {
    cm_stop(s_AudioSources[id]);
  }
}