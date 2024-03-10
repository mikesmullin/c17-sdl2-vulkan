#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

void Audio__Init();
void Audio__Shutdown();
void Audio__LoadAudioFile(const char* path);
void Audio__PlayAudio(const int id, const bool loop, const double gain);
void Audio__ResumeAudio(const int id, const bool loop, const double gain);
void Audio__StopAudio(const int id);

#endif