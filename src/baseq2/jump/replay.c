#include "g_local.h"


/*
===============
replay.c

Handles all recording / replay related stuff

JUMP TODO:

Currently records EVERY client frame.
This has the benefit of being 100% accurate to the original movement (with one big exception). Think of it as a demo.


Problems:

1.

The way watching a replay currently works depends on the watcher's frame rate.
Every client frame, advance the replay. If the watcher's frame rate differs, it will not look right.

Few possible fixes:
- Only update the replay every SERVER frame (10 tickrate) and advance the replay however many frames it needs.
    But the entire point of this new change is lost.
- We could simulate a fake player that the watcher would then spectate. That's a lot of work.
- Just back down and use 10 tickrate recordings again, but also save the velocity for a bit of accuracy.

2.

Any outside factors that can affect the player (moving platforms, WEAPONS, etc.) will desync the replay.

Few possible fixes:
- Also save a "traditional" version of a replay. Switch to this version on maps that use weapons or platforms.
- For weapons, they could be simulated every client frame instead of every server frame.
===============
*/


// with 125 frames per second this is 30 minutes
#define REPLAY_DEFAULT_ALLOCATED_FRAMES         225000

/*
===============
Jump_Replay_Record

Copy a usercmd to a frame.
===============
*/
void Jump_Replay_Record(const edict_t *player, const usercmd_t *ucmd, jump_replay_frame_t *out_frame)
{
    int i;
    for (i = 0; i < 2; i++) {
        out_frame->angles[i] = player->client->ps.pmove.delta_angles[i] + ucmd->angles[i];
    }

    out_frame->forwardmove = ucmd->forwardmove;
    out_frame->sidemove = ucmd->sidemove;
    out_frame->upmove = ucmd->upmove;

    out_frame->buttons = ucmd->buttons;

    out_frame->msec = ucmd->msec;
}

/*
===============
Jump_Replay_Copy

Copy a saved frame to a usercmd.
===============
*/
void Jump_Replay_Copy(const jump_replay_frame_t *frame, usercmd_t *ucmd)
{
    //ucmd->angles[0] = frame->angles[0];
    //ucmd->angles[1] = frame->angles[1];
    //ucmd->angles[2] = 0;

    ucmd->forwardmove = frame->forwardmove;
    ucmd->sidemove = frame->sidemove;
    ucmd->upmove = frame->upmove;

    ucmd->buttons = frame->buttons;

    ucmd->msec = frame->msec;
}

void Jump_Replay_EndWatching(edict_t *player)
{
    gclient_t *client = player->client;


    client->jump.pers.replay.watching = false;

    client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
    client->ps.pmove.pm_type = PM_NORMAL;
}

/*
===============
Jump_Replay_WatchThink

Called every ClientThink when the player should be watching a replay.
===============
*/
void Jump_Replay_WatchThink(edict_t *player, usercmd_t *ucmd)
{
    int i;
    short old_ang[3];
    short new_ang[3];
    jump_replay_frame_t *frame;

    gclient_t *client = player->client;

    // not watching
    if (!client->jump.pers.replay.watching) {
        return;
    }

    // wants to quit watching?
    const qboolean quit = ucmd->buttons & BUTTON_ATTACK ? true : false;


    int64_t *frame_num = &(client->jump.pers.replay.cur_frame);

    ++*frame_num;

    if (!quit && *frame_num >= 0 && *frame_num < client->jump.pers.recording.cur_frame) {
        
        frame = &(client->jump.pers.recording.frames[*frame_num]);

        old_ang[0] = ucmd->angles[0];
        old_ang[1] = ucmd->angles[1];
        old_ang[2] = ucmd->angles[2];

        new_ang[0] = frame->angles[0];
        new_ang[1] = frame->angles[1];
        new_ang[2] = 0;

        Jump_Replay_Copy(frame, ucmd);

        for (i = 0; i < 3; i++) {
            client->ps.pmove.delta_angles[i] = new_ang[i] - old_ang[i];
        }

        VectorCopy(new_ang, client->v_angle);
        VectorCopy(new_ang, client->ps.viewangles);

        client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
    } else {
        Jump_Replay_EndWatching(player);
    }
}

/*
===============
Jump_Replay_RecordingThink

Called every ClientThink when the player should be recorded.
===============
*/
void Jump_Replay_RecordingThink(edict_t *player, const usercmd_t *ucmd)
{
    gclient_t *client = player->client;


    const qboolean is_recording = client->jump.pers.recording.recording && client->jump.sp.moved && client->jump.pers.team >= JUMP_TEAM_EASY && !client->pers.spectator;

    if (!is_recording) {
        return;
    }


    int64_t *frame_num = &(client->jump.pers.recording.cur_frame);

    ++*frame_num;

    if (*frame_num >= 0 && *frame_num < client->jump.pers.recording.num_frames_allocated) {
        Jump_Replay_Record(player, ucmd, &(client->jump.pers.recording.frames[*frame_num]));
    }
}

void Jump_Replay_StartWatching(edict_t *player, int replay_num)
{
#ifdef _DEBUG
    gi.dprintf("Jump_Replay_StartWatching(%i, %i)\n", player - g_edicts, replay_num);
#endif

    int i;
    vec3_t new_ang;
    gclient_t *client = player->client;


    VectorCopy(client->jump.pers.recording.initial_snapshot.angles, new_ang);

    client->jump.pers.replay.watching = true;
    client->jump.pers.replay.cur_frame = -1;

    client->jump.pers.recording.recording = false;

    
    for (i = 0; i < 3; i++) {
        client->ps.pmove.delta_angles[i] = ANGLE2SHORT(new_ang[i] - client->resp.cmd_angles[i]);
    }

    VectorCopy(new_ang, client->v_angle);
    VectorCopy(new_ang, client->ps.viewangles);
    VectorCopy(client->jump.pers.recording.initial_snapshot.velocity, player->velocity);
    VectorCopy(client->jump.pers.recording.initial_snapshot.origin, player->s.origin);

    //client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
}

/*
===============
Jump_Replay_OnRunStart

When the player has started their run.
Does not save a frame yet.
===============
*/
void Jump_Replay_OnRunStart(edict_t *player, const usercmd_t *ucmd)
{
#ifdef _DEBUG
    gi.dprintf("Jump_Replay_OnRunStart(%i)\n", player - g_edicts);
#endif

    gclient_t *client = player->client;

    if (client->jump.pers.replay.watching) {
        return;
    }

    //if (client->jump.pers.recording.frames) {
    //    gi.TagFree(client->jump.pers.recording.frames);
    //    client->jump.pers.recording.frames = NULL;
    //    client->jump.pers.recording.num_frames_allocated = 0;
    //}


    if (!client->jump.pers.recording.frames) {
        client->jump.pers.recording.num_frames_allocated = REPLAY_DEFAULT_ALLOCATED_FRAMES;
        client->jump.pers.recording.frames = gi.TagMalloc(sizeof(jump_replay_frame_t) * REPLAY_DEFAULT_ALLOCATED_FRAMES, TAG_GAME);
    }

    client->jump.pers.recording.recording = true;
    client->jump.pers.recording.cur_frame = -1;

    VectorCopy(client->v_angle, client->jump.pers.recording.initial_snapshot.angles);
    VectorCopy(player->velocity, client->jump.pers.recording.initial_snapshot.velocity);
    VectorCopy(player->s.origin, client->jump.pers.recording.initial_snapshot.origin);
}

/*
===============
Jump_Replay_OnRunEnd

When the player has touched end.
===============
*/
void Jump_Replay_OnRunEnd(edict_t *player, double new_time)
{
#ifdef _DEBUG
    gi.dprintf("Jump_Replay_OnRunEnd(%i)\n", player - g_edicts);
#endif

    gclient_t *client = player->client;

    client->jump.pers.recording.recording = false;
}
