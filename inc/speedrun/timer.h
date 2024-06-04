#pragma once

#ifndef TIMER_H
#define TIMER_H

#include "shared/shared.h"

#define SPEEDRUN_TIME_LENGTH 13

void SpeedrunResetTimer(void);
void SpeedrunUpdateTimer(void);
void SpeedrunTimerAddMilliseconds(int msec);
void SpeedrunUnpauseTimer(void);
int SpeedrunPauseTimer(void);
void SpeedrunLevelFinished(void);
void SpeedrunGetTotalTimeString(int accuracy,
                                char time_string[static SPEEDRUN_TIME_LENGTH]);
void SpeedrunGetLevelTimeString(int accuracy,
                                char time_string[static SPEEDRUN_TIME_LENGTH]);

#endif // TIMER_H
