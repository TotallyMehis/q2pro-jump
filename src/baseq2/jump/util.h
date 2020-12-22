#pragma once

void Jump_Util_PadToRight(char* dst, size_t dst_len, const char* src);
void Jump_Util_Highlight(char* dst);

void Jump_Util_PlaySound(edict_t *src, int sound_index, int channel_in, float volume, int attenuation);
void Jump_Util_PlaySoundLocal(edict_t *src, int sound_index, int channel_in, float volume, int attenuation);

// returns the number of elements in the array
#define ARRAY_SIZE(x)        (sizeof(x) / sizeof(*(x)))
