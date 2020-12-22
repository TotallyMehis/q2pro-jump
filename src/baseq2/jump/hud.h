#pragma once


typedef enum {
    JUMP_STAT_TIMER = 0, // Seconds
    JUMP_STAT_TIMER2, // Deciseconds

    JUMP_STAT_SPEED,

    JUMP_STAT_KEY_LEFTRIGHT,
    JUMP_STAT_KEY_UPDOWN,
    JUMP_STAT_KEY_JUMPCROUCH,
    JUMP_STAT_KEY_ATTACK,

    JUMP_STAT_FPS,

    JUMP_STAT_TIMELEFT,

    JUMP_STAT_MAP_COUNT,

    JUMP_STAT_ADDED_TIME,

    JUMP_STAT_SELECTED_WEAPON,

    // THIS IS HARDCODED ON THE CLIENT
    // DON'T CHANGE
    JUMP_STAT_LAYOUTS = 13,

    JUMP_STAT_NEXTMAP1,
    JUMP_STAT_NEXTMAP2,
    JUMP_STAT_NEXTMAP3,

    JUMP_STAT_VOTE_TYPE,
    JUMP_STAT_VOTE_INITIATED,
    JUMP_STAT_VOTE_CAST,
    JUMP_STAT_VOTE_REMAINING,

    // DON'T GO OVER 32
    JUMP_STAT_LAST,
} jump_stats_t;


void Jump_HudPrecache(void);

void Jump_WriteStatusBar(void);

void Jump_SetStats(edict_t *player);
void Jump_SetSpectatorStats(edict_t *player);
