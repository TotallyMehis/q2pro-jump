#include "g_local.h"

#include "stubs.h"
#include "main.h"
#include "util.h"
#include "voting.h"
#include "hud.h"

#include <assert.h>

/*
===============
The mod's hud.

See hud.h for the indices.
===============
*/
static const char jump_hud[] =
    // speed
    "if 2 "
        "yb -32 "
        "xv 200 "
        "num 4 2 "
        "xv 226 "
        "yb -8 "
        "string2 \"Speed\" "
    "endif "

    // selected weapon
    "if 11 "
        "yb	-32 "
        "xv	280 "
        "pic 11 "
    "endif "

    // timer
    "yb -16 "
    "xr -24 "
    "string \".\" "
    "yb -32 "
    "xr -94 "
    "num 4 0 "
    "xr -18 "
    "num 1 1 "
    "yb -8 "
    "xr -34 "
    "string2 \"Time\" "

    // keystrokes
    "xl 2 "
    "yb -42 "

    // left/right key
    "if 3 "
        "pic 3 "
    "endif "

    // forward and back key
    "if 4 "
        "pic 4 "
    "endif "

    // jump and crouch key
    "if 5 "
        "pic 5 "
    "endif "

    // attack key
    "if 6 "
        "pic 6 "
    "endif "

    // FPS
    "xl 0 "
    "yb -76 "
    "num 3 7 "
    "xl 54 "
    "yb -60 "
    "string2 \"FPS\" "

    // voting
    "if 18 "
        "xl 2 "
        "yb -136 "
        "stat_string 18 "
        "yb -128 "
        "stat_string 17 "
        "yb -112 "
        "stat_string 19 "
        "yb -104 "
        "stat_string 20 "
    "endif "

    // map count
    "if 9 "
        "xr -34 "
        "yt 42 "
        "stat_string 9 "
        "yt 50 "
        "string \"Maps\" "
    "endif "


    "xr -250 "
    "yt 2 "
    // current map
    "string \"%s\" "
    "yt 10 "
    // last map 1
    "string \"%s\" "
    "yt 18 "
    // last map 2
    "string \"%s\" "
    "yt 26 "
    // last map 3
    "string \"%s\" "


    "if 8 "
        // added time
        //"xr -32 "
        //"yt 100 "
        //"stat_string 10 "

        // time left
        "yt 64 "
        "xr	-34 "
        "string2 \"Time\" "
        "yt 74 "
        "xr	-50 "
        "num 3 8 "
    "endif "
;

/*
===============
Jump_WriteStatusBar

Writes the status bar into a config string.
===============
*/
void Jump_WriteStatusBar(void)
{
    char            temp[sizeof(jump_hud) + 256];
    //char            temp2[sizeof(jump_hud) + 256];
    char            cur_mapname[32];
    const char      **prev_maps;
    int             num_prev_maps = 0;
    char            formatted_maps[3][32];
    int             i;

    //Q_strlcpy(cur_mapname, level.mapname, sizeof(cur_mapname));
    num_prev_maps = Jump_GetPrevMaps(&prev_maps);

    Jump_Util_PadToRight(cur_mapname, sizeof(cur_mapname), level.mapname);
    Jump_Util_Highlight(cur_mapname);

    for (i = 0; i < min(3, num_prev_maps); i++) {
        Jump_Util_PadToRight(formatted_maps[i], sizeof(formatted_maps[i]), prev_maps[i]);
    }

    Q_snprintf(temp, sizeof(temp), jump_hud,
        cur_mapname,
        formatted_maps[0],
        formatted_maps[1],
        formatted_maps[2]);

    gi.configstring(CS_STATUSBAR, temp);


    gi.configstring(JUMP_CONFIG_MAPS_TOTAL, va("%i", 0));

    // JUMP TODO
    gi.configstring(JUMP_CONFIG_NEXTMAP1, "nextmap1");
    gi.configstring(JUMP_CONFIG_NEXTMAP2, "nextmap2");
    gi.configstring(JUMP_CONFIG_NEXTMAP3, "nextmap3");
}

/*
===============
Jump_HudPrecache

Precache all hud related stuff.
===============
*/
void Jump_HudPrecache(void)
{
    // jump icons
    gi.imageindex("forward");
    gi.imageindex("back");
    gi.imageindex("left");
    gi.imageindex("right");
    gi.imageindex("duck");
    gi.imageindex("jump");
    gi.imageindex("attack");
}


/*
===============
Jump_SetStats

Our implementation of G_SetStats.
===============
*/
void Jump_SetStats(edict_t *player)
{
    gclient_t       *client = player->client;
    player_state_t  *state = &(client->ps);
    float           vel[3];
    double          timer;
    float           timeleft;


    assert(JUMP_STAT_LAST < ARRAY_SIZE(state->stats));
    
    if (client->jump.sp.finished) {
        timer = client->jump.sp.finished_time;
    } else {
        timer = client->jump.sp.cur_time_msec / 1000.0;
    }

    VectorCopy(player->velocity, vel);
    vel[2] = 0;

    timeleft = 0.0f;
    Jump_GetTimeleft(&timeleft);
    if (timeleft >= 60.0f) {
        timeleft = (float)floor(timeleft / 60.0f);
    }


    state->stats[JUMP_STAT_TIMER] = (int)timer;
    state->stats[JUMP_STAT_TIMER2] = (int)(timer * 10) % 10;

    state->stats[JUMP_STAT_SPEED] = (short)VectorLength(vel);

    //
    // keys
    //
    if (client->jump.frame.move_side > 0) {
        state->stats[JUMP_STAT_KEY_LEFTRIGHT] = gi.imageindex("right");
    } else if (client->jump.frame.move_side < 0) {
        state->stats[JUMP_STAT_KEY_LEFTRIGHT] = gi.imageindex("left");
    } else {
        state->stats[JUMP_STAT_KEY_LEFTRIGHT] = 0;
    }

    if (client->jump.frame.move_fwd > 0) {
        state->stats[JUMP_STAT_KEY_UPDOWN] = gi.imageindex("forward");
    } else if (client->jump.frame.move_fwd < 0) {
        state->stats[JUMP_STAT_KEY_UPDOWN] = gi.imageindex("back");
    } else {
        state->stats[JUMP_STAT_KEY_UPDOWN] = 0;
    }

    if (client->jump.frame.move_up > 0) {
        state->stats[JUMP_STAT_KEY_JUMPCROUCH] = gi.imageindex("jump");
    } else if (client->jump.frame.move_up < 0) {
        state->stats[JUMP_STAT_KEY_JUMPCROUCH] = gi.imageindex("duck");
    } else {
        state->stats[JUMP_STAT_KEY_JUMPCROUCH] = 0;
    }

    if (client->pers.weapon && client->pers.weapon->ammo && client->pers.weapon->icon)
        client->ps.stats[JUMP_STAT_SELECTED_WEAPON] = gi.imageindex (client->pers.weapon->icon);
    else
        client->ps.stats[JUMP_STAT_SELECTED_WEAPON] = 0;

    state->stats[JUMP_STAT_KEY_ATTACK] = (client->buttons & BUTTON_ATTACK) ? gi.imageindex("attack") : 0;

    state->stats[JUMP_STAT_TIMELEFT] = (short)timeleft;

    state->stats[JUMP_STAT_MAP_COUNT] = JUMP_CONFIG_MAPS_TOTAL;

    state->stats[JUMP_STAT_FPS] = (short)(1000.0f / client->jump.frame.msec);

    // config strings
    state->stats[JUMP_STAT_ADDED_TIME] = 0;


    state->stats[JUMP_STAT_LAYOUTS] = 0;

    // 1 = draw
    // 2 = capture mouse
    if (client->jump.pers.scoreboard != 0) {
        state->stats[JUMP_STAT_LAYOUTS] |= 1;
    }
    

    state->stats[JUMP_STAT_NEXTMAP1] = JUMP_CONFIG_NEXTMAP1;
    state->stats[JUMP_STAT_NEXTMAP2] = JUMP_CONFIG_NEXTMAP2;
    state->stats[JUMP_STAT_NEXTMAP3] = JUMP_CONFIG_NEXTMAP3;

    //
    // voting
    //
	state->stats[JUMP_STAT_VOTE_REMAINING] = JUMP_CONFIG_VOTE_REMAINING;
	state->stats[JUMP_STAT_VOTE_CAST] = JUMP_CONFIG_VOTE_CAST;
	state->stats[JUMP_STAT_VOTE_TYPE] = JUMP_CONFIG_VOTE_TYPE;

	if (Jump_Vote_IsVoting()) {
		state->stats[JUMP_STAT_VOTE_INITIATED] = JUMP_CONFIG_VOTE_INITIATED;
	}
	else {
		state->stats[JUMP_STAT_VOTE_INITIATED] = 0;
    }
}

/*
===============
Jump_SetSpectatorStats

Our implementation of G_SetSpectatorStats.
===============
*/
void Jump_SetSpectatorStats(edict_t *player)
{
    //assert(player && player->client);

    gclient_t       *client = player->client;

    if (!client->chase_target)
        Jump_SetStats(player);
}

