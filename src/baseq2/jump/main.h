#pragma once

// how many maps the server has in total.
#define JUMP_CONFIG_MAPS_TOTAL      CS_GENERAL

// nextmaps shown in scoreboard
#define JUMP_CONFIG_NEXTMAP1        (JUMP_CONFIG_MAPS_TOTAL+1)
#define JUMP_CONFIG_NEXTMAP2        (JUMP_CONFIG_NEXTMAP1+1)
#define JUMP_CONFIG_NEXTMAP3        (JUMP_CONFIG_NEXTMAP2+1)

#define JUMP_CONFIG_VOTE_REMAINING  (JUMP_CONFIG_NEXTMAP3+1)
#define JUMP_CONFIG_VOTE_CAST       (JUMP_CONFIG_VOTE_REMAINING+1)
#define JUMP_CONFIG_VOTE_TYPE       (JUMP_CONFIG_VOTE_CAST+1)
#define JUMP_CONFIG_VOTE_INITIATED  (JUMP_CONFIG_VOTE_TYPE+1)

#define JUMP_MAX_MAPS               1337


qboolean    Jump_GetTimeleft(float *timeleft);
void        Jump_ExtendTime(int extend_amount, qboolean print);

void        Jump_UpdateHighscores(edict_t *player, double new_time);

extern jump_globals_t jump;
