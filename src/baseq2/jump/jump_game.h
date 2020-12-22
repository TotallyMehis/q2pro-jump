#pragma once


/*
===============
jump_game.h


Automatically included with g_local.h
===============
*/


#define INVALID_PLAYER_ID       ((int64_t)-1)
#define INVALID_MAP_ID          ((int64_t)-1)
#define INVALID_TIME            (0.0)
#define INVALID_RANK            (0)


#define MAX_HIGHSCORES          15

#define DATE_FORMAT             "%Y/%m/%d"


#define JUMP_GAMEDIR            (gi.cvar("game", "jump", 0)->string)


/*
=====================
Replay structs
=====================
*/

// replay frame
// pretty much a copy of usercmd struct.
typedef struct {
    byte        msec;
    byte        buttons;
    short       angles[2];
    short       forwardmove, sidemove, upmove;
} jump_replay_frame_t;

// this is a snapshot of the player's state.
typedef struct {
    vec3_t      origin;
    vec3_t      velocity;
    vec3_t      angles;

    short       delta_angles[3];
} jump_replay_snapshot_t;

/*
=====================
Player structs
=====================
*/

// our teams
typedef enum {
    JUMP_TEAM_NONE = -1,

    JUMP_TEAM_SPECTATOR,

    JUMP_TEAM_EASY,

    JUMP_TEAM_HARD,
} jump_team_t;

// idle state
typedef enum {
    // not idle.
    JUMP_IDLESTATE_NONE = 0,
    
    // idle due to afk.
    JUMP_IDLESTATE_AUTO,

    // idle by using the 'idle' command.
    JUMP_IDLESTATE_SELF,
} jump_idle_state_t;



// frame specific client data.
// do not use outside of client's Think functions.
typedef struct {
    // current msec the player moved for.
    int                     msec;

    short                   move_fwd;
    short                   move_side;
    short                   move_up;
} jump_client_frame_t;

// recording related data
typedef struct {
    jump_replay_frame_t     *frames;
    size_t                  num_frames_allocated;

    int64_t                 cur_frame;

    qboolean                recording;

    // data on how the player started (angles, possible velocity, etc.)
    jump_replay_snapshot_t  initial_snapshot;
} jump_client_recording_t;

// replay related data
typedef struct {
    qboolean watching;

    int64_t cur_frame;
} jump_client_replay_t;

// data that will be reset on respawn.
// JUMP TODO: Remove this?
typedef struct {
    // the accumulated time
    // incremented every client think
    unsigned int            cur_time_msec;

    // have they moved after spawning?
    // for starting the timer
    qboolean                moved;

    qboolean                finished;
    float                   finished_time;
} jump_client_spawn_t;

typedef struct {
    // the id from database.
    int64_t                 player_id;

    // my best time
    double                  best_time;
    
    int                     completions;

    // my rank
    int                     rank;

    int                     num_maps_beaten;
} jump_client_dbstate_t;

// data that will persist across level changes and respawns.
typedef struct {
    //
    jump_idle_state_t       idle_state;

    // the accumulated time player has been idle for in seconds.
    float                   idle_time;

    jump_team_t             team;
    jump_team_t             prev_team;

    jump_client_dbstate_t   db;

    // current scoreboard state
    int                     scoreboard;

    jump_client_recording_t recording;
    jump_client_replay_t    replay;

    // whether other players should be hidden.
    qboolean hide_others;
} jump_client_persdata_t;

typedef struct {
    // persistent data
    jump_client_persdata_t      pers;

    // things that will be gone next respawn.
    jump_client_spawn_t         sp;

    // things we saved for this frame.
    jump_client_frame_t         frame;
} jump_client_data_t;


/*
=====================
Global structs
=====================
*/

typedef enum {
    JUMP_VOTE_NONE = 0,

    // extend the time we will spend on this map.
    JUMP_VOTE_EXTEND,

    // change the current map right now.
    JUMP_VOTE_CHANGEMAP,

    // nominate a map for the map end vote.
    JUMP_VOTE_NOMINATE,
} jump_votetype_t;

// to store the map's best times.
typedef struct {
    double          time;

    char            player_name[64];
    
    char            date[64];

    int64_t         player_id;

    qboolean        was_beaten_this_session;
} jump_besttimedata_t;

// map specific data
typedef struct {
    // how many times we've extended.
    int                     times_extended;
    // how much we've extended.
    int                     extended_amount;

    //
    // map specific things retrieved from database.
    //

    // current map's row id in the database.
    int64_t                 map_id;
    // when this map was added.
    uint64_t                date_added;
    // how many records this map has.
    int                     num_records;
    // how many times this map has been completed.
    int                     num_completions_total;

    jump_besttimedata_t     best[MAX_HIGHSCORES];
    // how many times we have loaded in the 'best' array.
    int                     num_best_times;

} jump_mapdata_t;

// JUMP TODO: Finish mapcycle stuff
typedef struct {
    char map_name[64];

    uint64_t last_played_date;
} jump_mapcycle_map_t;

typedef struct {
    jump_mapcycle_map_t *maps;
    size_t  num_maps;

} jump_mapcycle_t;

//
// the global data we will share with the entire jump mod.
//
typedef struct {
    // map specific data
    jump_mapdata_t          map;

    // total number of maps we have.
    int                     num_maps;

    jump_mapcycle_t         mapcycle;
} jump_globals_t;
