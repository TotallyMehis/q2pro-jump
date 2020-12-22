#pragma once


void Jump_Replay_RecordingThink(edict_t *player, const usercmd_t *ucmd);
void Jump_Replay_WatchThink(edict_t *player, usercmd_t *ucmd);

void Jump_Replay_StartWatching(edict_t *player, int replay_num);

void Jump_Replay_OnRunStart(edict_t *player, const usercmd_t *ucmd);
void Jump_Replay_OnRunEnd(edict_t *player, double new_time);

