#ifndef AUDIO_H
#define AUDIO_H

// TODO: could implement Unity-compatible interface?
// AudioListener & AudioMixer --> AudioSource --> AudioClip
// see: https://docs.unity3d.com/Manual/class-AudioSource.html

#include <stdbool.h>

typedef enum {
  AUDIO_ERROR_NONE = 0,
  AUDIO_ERROR_SDL_OPEN_AUDIO_DEVICE_FAILED = 1,
  AUDIO_ERROR_FAILED_TO_LOAD_FILE = 2,
  AUDIO_ERROR_MAX_SOURCES = 3,
} Audio__Error_t;

extern const char* ckp_Audio__ERROR_MESSAGES[];

Audio__Error_t Audio__Init();
void Audio__Shutdown();
Audio__Error_t Audio__LoadAudioFile(const char* path);
void Audio__PlayAudio(const int id, const bool loop, const double gain);

#endif