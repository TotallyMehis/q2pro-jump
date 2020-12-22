#include "g_local.h"

#include "database.h"
#include "threading.h"
#include "hud.h"
#include "msets.h"
#include "main.h"


jump_globals_t jump = {0};


static void ReadMapCycle(void);

void Jump_Vote_InitCvars(void);
void Jump_Vote_Init(void);
void Jump_Mset_Init(void);
void Jump_Mset_WorldSpawn(const char *arg);

qboolean Jump_Vote_CheckRules(void);


/*
===============
Jump_InitGame_Post

When game dll is starting. Only called ONCE.
===============
*/
void Jump_InitGame_Post(void)
{
#ifdef _DEBUG
    gi.dprintf("Jump_InitGame_Post()\n");
#endif

    // Well we must at least have one... :)
    jump.num_maps = 1;


    Jump_Database_Init();

    gi.cvar_set("run_pitch", "0");
    gi.cvar_set("run_roll", "0");
    gi.cvar_set("bob_up", "0");
    gi.cvar_set("bob_pitch", "0");
    gi.cvar_set("bob_roll", "0");

    // Deathmatch has to be set to 1!!!
    gi.cvar_set("deathmatch", "1");

    gi.cvar_set("dmflags", va("%i", DF_INFINITE_AMMO | DF_NO_FALLING));

    gi.cvar_set("gamename", "jump");

    Jump_Vote_InitCvars();
}

/*
===============
Jump_ShutdownGame_Post

When game dll is shutting down.
===============
*/
void Jump_ShutdownGame_Post(void)
{
    Jump_Thread_CloseAll();
}

/*
===============
Jump_WorldSpawn_Spawn

Called when worldspawn is spawning.
Using temp fields is safe.
===============
*/
void Jump_WorldSpawn_Spawn(edict_t *ent)
{
    Jump_HudPrecache();


    if (st.mset && st.mset[0]) {
        Jump_Mset_WorldSpawn(st.mset);
    }

    ReadMapCycle();
}

/*
===============
Jump_SpawnEntities_Pre

Before map entities are spawned.
===============
*/
static void LoadMapData(dbcb_map_t *map_data)
{
    int i;


    jump.map.map_id = map_data->map_id;
    jump.map.date_added = map_data->date_added;
    jump.num_maps = map_data->num_maps_total;
    jump.map.num_records = map_data->num_records;
    jump.map.num_completions_total = map_data->num_completions_total;


    for (i = 0; i < MAX_HIGHSCORES; i++) {
        Q_strlcpy(jump.map.best[i].date, map_data->best_times_date[i], sizeof(jump.map.best[i].date));
        Q_strlcpy(jump.map.best[i].player_name, map_data->best_times_name[i], sizeof(jump.map.best[i].player_name));
        jump.map.best[i].player_id = map_data->best_times_ids[i];
        jump.map.best[i].time = map_data->best_times[i];
    }

    jump.map.num_best_times = map_data->num_best_times;


    gi.dprintf("Successfully queried map! (id: %i)\n", jump.map.map_id);


    gi.configstring(JUMP_CONFIG_MAPS_TOTAL, va("%i", jump.num_maps));
}

void Jump_SpawnEntities_Pre(void)
{
#ifdef _DEBUG
    gi.dprintf("Jump_SpawnEntities_Pre()\n");
#endif

    jump.map.times_extended = 0;
    jump.map.extended_amount = 0;
    jump.map.map_id = INVALID_MAP_ID; // we don't know the id yet!
    jump.map.date_added = 0;

    jump.map.num_best_times = 0;
    memset(jump.map.best, 0, sizeof(jump.map.best));


    Jump_Vote_Init();
    Jump_Mset_Init();

    Jump_Database_FetchMapData(level.mapname, LoadMapData);
}

/*
===============
Jump_SpawnEntities_Post

After map entities have been spawned.
===============
*/

void Jump_SpawnEntities_Post(void)
{
#ifdef _DEBUG
    gi.dprintf("Jump_SpawnEntities_Post()\n");
#endif
}

/*
===============
Jump_RunFrame

Every server frame.
===============
*/
void Jump_RunFrame(void)
{
    Jump_Thread_Update();

    Jump_Vote_CheckRules();
}

/*
===============
Jump_CheckGameRules

Our game rules check.
===============
*/
void Jump_CheckGameRules(void)
{
    float timeleft;
    if (Jump_GetTimeleft(&timeleft) && timeleft <= 0.0f) {
        level.exitintermission = 1;
        //Q_strlcpy(level.nextmap, "", sizeof(level.nextmap));
        level.changemap = level.nextmap;
    }
}

/*
===============
Jump_GetTimeleft

Returns true if there is time left and timeleft argument is changed to seconds left.
If no timelimit is set, returns false.
===============
*/
qboolean Jump_GetTimeleft(float *timeleft)
{
    if (timelimit->value <= 0)
        return false;

    *timeleft = (timelimit->value + jump.map.extended_amount) * 60 - level.time;
    return true;
}

/*
===============
Jump_ExtendTime
===============
*/
void Jump_ExtendTime(int extend_amount, qboolean print)
{
    if (!extend_amount)
        return;

    jump.map.extended_amount += extend_amount;
    ++jump.map.times_extended;

    if (print) {
        if (extend_amount > 0) {
            gi.bprintf(PRINT_HIGH, "Extended map time by %i minutes!", extend_amount);
        } else {
            gi.bprintf(PRINT_HIGH, "Removed %i minutes from map time!", extend_amount);
        }
        
    }
}

/*
===============
Jump_UpdateHighscores

A new run has finished. Checks if this record should go on the highscores.
===============
*/
void Jump_UpdateHighscores(edict_t *player, double new_time)
{
    int         i;
    gclient_t   *client = player->client;
    int64_t     my_id = client->jump.pers.db.player_id;
    int         my_pos = -1;
    time_t      time_;
    struct tm   *tm_;


    jump.map.num_completions_total++;

    // new record?
    if (client->jump.pers.db.best_time <= 0) {
        jump.map.num_records++;
    }
    

    //
    // update highscore table
    //

    // find their position and see if it should be updated.
    for (i = 0; i < jump.map.num_best_times; i++) {
        if (jump.map.best[i].player_id == my_id) {
            my_pos = i;

            // nothing to update
            if (jump.map.best[i].time <= new_time) {
                return;
            }

            break;
        }
    }

    // find new spot on high score.
    for (i = 0; i < jump.map.num_best_times; i++) {
        if (new_time < jump.map.best[i].time) {
            break;
        }
    }

    if (i >= MAX_HIGHSCORES) {
        return;
    }

    // remove their previous time.
    if (my_pos != -1 && my_pos < (MAX_HIGHSCORES-1)) {
        memmove(
            &(jump.map.best[my_pos]),
            &(jump.map.best[my_pos + 1]),
            sizeof(jump_besttimedata_t) * (MAX_HIGHSCORES - (my_pos + 1))
        );
    }

    // shift all elements up by one.
    if (i < (MAX_HIGHSCORES-1)) {
        memmove(
            &(jump.map.best[i + 1]),
            &(jump.map.best[i]),
            sizeof(jump_besttimedata_t) * (MAX_HIGHSCORES - (i + 1))
        );
    }

    // increase number of records if
    // they didn't have a previous record on the list.
    if (jump.map.num_best_times < MAX_HIGHSCORES && my_pos == -1) {
        jump.map.num_best_times++;
    }


    jump.map.best[i].was_beaten_this_session = true;
    jump.map.best[i].time = new_time;
    jump.map.best[i].player_id = my_id;
    Q_strlcpy(jump.map.best[i].player_name, client->pers.netname, sizeof(jump.map.best[i].player_name));

    time(&time_);
    tm_ = localtime(&time_);
    strftime(jump.map.best[i].date, sizeof(jump.map.best[i].date), DATE_FORMAT, tm_);
}

/*
===============
ReadMapCycle

Read the map list the server will use from file.

JUMP TODO: Don't use this? Just use database? Fix trailing whitespaces.
===============
*/
static void ReadMapCycle(void)
{
    // JUMP TODO: Finish this
    FILE *fp;
    size_t file_size;
    char *p;
    char *newline;
    char *comment;
    char *buffer;
    int i;

    if (jump.mapcycle.maps) {
        gi.TagFree(jump.mapcycle.maps);
        jump.mapcycle.maps = NULL;
        jump.mapcycle.num_maps = 0;
    }


    fp = fopen(va("%s/jump_mapcycle.txt", JUMP_GAMEDIR), "rb");
    if (!fp) {
        gi.dprintf("No mapcycle found!\n");
        return;
    }

    // get file size.
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (!file_size) {
        fclose(fp);
        return;
    }


    buffer = (char*)gi.TagMalloc(file_size+1, TAG_GAME);
    fread(buffer, 1, file_size, fp);
    buffer[file_size] = '\0';

    fclose(fp);

    jump.mapcycle.maps = (jump_mapcycle_map_t*)gi.TagMalloc(sizeof(jump_mapcycle_map_t) * JUMP_MAX_MAPS, TAG_GAME);


    // now assign them
    p = buffer;
    i = 0;
    while (p && *p && i < JUMP_MAX_MAPS) {
        // remove line endings to mark them as the end of map name.
        newline = strchr(p, '\n');
        if (newline) {
            *newline = '\0';
            // remove carriage return
            // if we're not at the start of the string
            if (newline != p && *(newline-1) == '\r') {
                *(newline-1) = '\0';
            }
        }

        // remove comments.
        comment = strstr(p, "//");
        if (comment) {
            *comment = '\0';
        }

        if (*p) {
            Q_strlcpy(jump.mapcycle.maps[i].map_name, p, sizeof(jump.mapcycle.maps[i].map_name));
            i++;
        }

        // no more new lines, just break.
        if (!newline) {
            break;
        }

        p = newline + 1;
    } 

    jump.mapcycle.num_maps = i;

    gi.TagFree(buffer);
}
