#pragma once


void Jump_Database_Init(void);
void Jump_Database_Shutdown(void);


/*
=====================
Fetch single time
=====================
*/
typedef struct {
    int64_t map_id;

    int64_t player_id;

    double time;
    
    char date[64];
} dbload_map_time_t;

/*
=====================
Fetch map
=====================
*/
typedef struct {
    // row id
    int64_t     map_id;

    // the date map was added.
    uint64_t    date_added;

    // number of records this map has.
    int         num_records;

    // how many completions this map has.
    int         num_completions_total;

    // number of maps total on the server.
    int         num_maps_total;

    // was the map record just now added to database?
    qboolean    is_new_map;

    // best times for this map
    int64_t     best_times_ids[MAX_HIGHSCORES];
    double      best_times[MAX_HIGHSCORES];
    char        best_times_date[MAX_HIGHSCORES][64];
    char        best_times_name[MAX_HIGHSCORES][64];
    // how many of the above we have.
    int         num_best_times;
} dbcb_map_t;

void Jump_Database_FetchMapData(const char *mapname, void (*cb)(dbcb_map_t *map_data));

/*
=====================
Fetch user
=====================
*/
typedef struct {
    // player's name we just fetched.
    // to identify the player
    char user_name[64];

    // row id
    int64_t player_id;

    int64_t map_id;

    // the time player had as their best for this map.
    double my_time;

    // number of completions of this map.
    int completions;

    int num_maps_beaten;

    int rank;
} dbcb_user_t;

void Jump_Database_FetchPlayerData(const char *name, void (*cb)(dbcb_user_t *user_data));

/*
=====================
Store time
=====================
*/
typedef struct {
    // player id
    int64_t player_id;


    int new_rank;
} dbcb_storetime_t; 

typedef struct {
    // player id
    int64_t player_id;

    // map id
    int64_t map_id;

    // time to save
    double time;

    // completely new record. won't be in the database.
    qboolean is_new_rec;

    // new personal best. should have previous times.
    qboolean new_pb;

    // callback to call when done.
    void (*cb)(dbcb_storetime_t *data);
} dbstore_time_t;

void Jump_Database_StoreTime(dbcb_storetime_t *data);

/*
=====================
Fetch rank list (for printing)
=====================
*/
typedef struct {
    int count;
    int offset;

    int player_num;
} dbload_ranklist_t;

typedef struct {
    // player to print to.
    int player_num;

    // the name of the players
    char player_name[15][64];

    int num_1[15]; // 1st
    int num_2[15]; // 2nd
    int num_3[15]; // 3rd
    int num_4[15]; // 4th
    int num_5[15]; // 5th

    int total_points[15];

    // number of players retrieved
    int num_retrieved;

    // number we requested.
    int count;

    int offset;
} dbcb_ranklist_t;

void Jump_Database_FetchRankList(dbload_ranklist_t *data, void (*cb)(dbcb_ranklist_t *ranklist));
