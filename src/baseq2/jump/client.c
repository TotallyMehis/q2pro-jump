#include "g_local.h"

#include "util.h"
#include "database.h"
#include "client.h"
#include "replay.h"
#include "main.h"


void Jump_Vote_Disconnected(edict_t *player);


static void LoadPlayerData(dbcb_user_t *user_data)
{
    int                 i;
    edict_t             *player;
    gclient_t           *client;
    const char          *name = user_data->user_name;
    char                msg_buffer[1024];

    // JUMP TODO: Handle errors properly.
    if (user_data->map_id != jump.map.map_id) {
        //gi.dprintf("ERROR:");
        return;
    }


    for (i = 0; i <= game.maxclients; i++) {
        player = &g_edicts[i];
        if (!player->inuse)
            continue;
        if (!player->client)
            continue;

        client = player->client;

        if (!Q_strcasecmp(client->pers.netname, name)) {
            // found our match.
            if (user_data->player_id == INVALID_PLAYER_ID) {
                gi.cprintf(player, PRINT_HIGH,
                    "\x1D\x1E\x1E\x1E\x1E\x1E\x1E\x1E\x1E\x1F"
                    "\nSomething went wrong when retrieving your data! :(\n"
                    "\x1D\x1E\x1E\x1E\x1E\x1E\x1E\x1E\x1E\x1F\n");
                return;
            }


            client->jump.pers.db.player_id = user_data->player_id;
            client->jump.pers.db.best_time = user_data->my_time;
            client->jump.pers.db.completions = user_data->completions;
            client->jump.pers.db.num_maps_beaten = user_data->num_maps_beaten;
            client->jump.pers.db.rank = user_data->rank;

            // Loading bar
            // \x80\x81\x82

            // Smaller bar looking thingy
            // \x1D\x1E\x1F
            
            Q_snprintf(msg_buffer, sizeof(msg_buffer),
                "\x1D\x1E\x1E\x1E\x1E\x1E\x1E\x1E\x1E\x1F"
                "\nWelcome, %s!\n"
                "\x1D\x1E\x1E\x1E\x1E\x1E\x1E\x1E\x1E\x1F\n",
                user_data->user_name);
            Jump_Util_Highlight(msg_buffer);
            gi.cprintf(player, PRINT_HIGH, msg_buffer);

            break;
        }
    }
}

/*
===============
Jump_ClientBegin

Initialize player here!!!
===============
*/
void Jump_ClientBegin(edict_t *player)
{
    gclient_t *client = player->client;

    client->jump.pers.team = JUMP_TEAM_NONE;
    client->jump.pers.prev_team = JUMP_TEAM_NONE;
    client->jump.pers.db.best_time = 0;
    client->jump.pers.db.completions = 0;
    client->jump.pers.db.player_id = INVALID_PLAYER_ID;

    Jump_Database_FetchPlayerData(client->pers.netname, LoadPlayerData);
}

/*
===============
Jump_ClientDisconnect
===============
*/
void Jump_ClientDisconnect(edict_t *player)
{
    Jump_Vote_Disconnected(player);
}

/*
===============
Jump_InitClientPersistant

Our version of InitClientPersistant.
===============
*/
void Jump_InitClientPersistant(gclient_t *client)
{
#ifdef _DEBUG
    gi.dprintf("Jump_InitClientPersistant(%i)\n", client - game.clients);
#endif

    gitem_t     *item;
    qboolean spectator = client->pers.spectator;

    // just copy over our entire persistent data.
    jump_client_persdata_t our_pers = client->jump.pers;

    memset(&client->pers, 0, sizeof(client->pers));

    item = FindItem("Blaster");
    client->pers.selected_item = ITEM_INDEX(item);
    client->pers.inventory[client->pers.selected_item] = 1;

    client->pers.weapon = item;

    client->pers.health         = 100;
    client->pers.max_health     = 100;

    client->pers.max_bullets    = 200;
    client->pers.max_shells     = 100;
    client->pers.max_rockets    = 50;
    client->pers.max_grenades   = 50;
    client->pers.max_cells      = 200;
    client->pers.max_slugs      = 50;

    client->pers.connected = true;

    // our different inits
    client->pers.spectator = spectator;

    client->jump.pers = our_pers;
}

/*
===============
Jump_InitClientResp
===============
*/
void Jump_InitClientResp(gclient_t *client)
{
#ifdef _DEBUG
    gi.dprintf("Jump_InitClientResp(%i)\n", client - game.clients);
#endif
}

/*
===============
Jump_PutClientInServer
===============
*/
void Jump_PutClientInServer(edict_t *player)
{
#ifdef _DEBUG
    gi.dprintf("Jump_PutClientInServer(%i)\n", player - g_edicts);
#endif

    gclient_t *client = player->client;

    if (client->jump.pers.team == JUMP_TEAM_NONE) {
        client->jump.pers.team = JUMP_TEAM_EASY;
        client->pers.spectator = false;
    }
    
    client->jump.sp.cur_time_msec = 0;
    client->jump.sp.finished = false;
    client->jump.sp.finished_time = 0.0f;
    client->jump.sp.moved = false;
}

/*
===============
Jump_ClientUserinfoChanged

Our implementation of ClientUserinfoChanged.
===============
*/
void Jump_ClientUserinfoChanged(edict_t *player, char *userinfo)
{
    const char      *s;
    int             playernum = player - g_edicts - 1;
    gclient_t       *client = player->client;
    char            buffer[64];


    // check for malformed or illegal info strings
    if (!Info_Validate(userinfo)) {
        strcpy(userinfo, "\\name\\badinfo\\skin\\male/grunt");
    }

    // set name
    s = Info_ValueForKey(userinfo, "name");
    Q_strlcpy(client->pers.netname, s, sizeof(client->pers.netname));

    // set skin
    Jump_GetPlayerModel(player, Info_ValueForKey(userinfo, "skin"), buffer, sizeof(buffer));

    // combine name and skin into a configstring
    gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s", client->pers.netname, buffer));

    // fov
    client->ps.fov = (float)atoi(Info_ValueForKey(userinfo, "fov"));
    if (client->ps.fov < 1)
        client->ps.fov = 90;
    else if (client->ps.fov > 160)
        client->ps.fov = 160;

    // handedness
    s = Info_ValueForKey(userinfo, "hand");
    if (strlen(s)) {
        client->pers.hand = atoi(s);
    }

    // save off the userinfo in case we want to check something later
    Q_strlcpy(client->pers.userinfo, userinfo, sizeof(client->pers.userinfo));
}

/*
===============
Jump_ClientThinks
===============
*/
void Jump_ClientThink_Pre(edict_t *player, usercmd_t *ucmd)
{
    gclient_t *client = player->client;

    // Update moved state.
    if (!client->jump.sp.moved && client->jump.pers.team >= JUMP_TEAM_EASY) {
        client->jump.sp.moved = ucmd->upmove != 0 || ucmd->sidemove != 0 || ucmd->forwardmove != 0 || ucmd->buttons & (BUTTON_ATTACK | BUTTON_USE);

#ifdef _DEBUG
        if (client->jump.sp.moved) {
            gi.dprintf("Started player %i timer!\n", player - g_edicts);       

            Jump_Replay_OnRunStart(player, ucmd);
        }
#endif
    }
    
    if (client->jump.sp.moved && !client->jump.sp.finished) {
        client->jump.sp.cur_time_msec += (unsigned int)ucmd->msec;
    }

    Jump_Replay_WatchThink(player, ucmd);

    // HACK: when watching a replay, the movetype is changed to dead.
    // to allow the clients to lerp view angles.
    pmtype_t pmtype = client->ps.pmove.pm_type;
    if (client->jump.pers.replay.watching && pmtype != PM_NORMAL)
        client->ps.pmove.pm_type = PM_NORMAL;
    


    // Save our frame specific stuff we'll need.
    client->jump.frame.msec = ucmd->msec;
    client->jump.frame.move_fwd = ucmd->forwardmove;
    client->jump.frame.move_side = ucmd->sidemove;
    client->jump.frame.move_up = ucmd->upmove;
}

void Jump_ClientThink_Post(edict_t *player, usercmd_t *ucmd)
{
    gclient_t *client = player->client;


    Jump_Replay_RecordingThink(player, ucmd);

    // HACK: when watching a replay, the movetype is changed to dead.
    // to allow the clients to lerp view angles.
    pmtype_t pmtype = client->ps.pmove.pm_type;
    if (client->jump.pers.replay.watching && pmtype == PM_NORMAL)
        client->ps.pmove.pm_type = PM_DEAD;
}

qboolean Jump_IsValidTeam(jump_team_t team)
{
    return team >= JUMP_TEAM_SPECTATOR && team <= JUMP_TEAM_HARD;
}

/*
===============
Jump_JoinTeam
===============
*/
qboolean Jump_JoinTeam(edict_t *player, jump_team_t team)
{
#ifdef _DEBUG
    gi.dprintf("Jump_JoinTeam(%i, %i)\n", player - g_edicts, team);
#endif

    if (!Jump_IsValidTeam(team)) {
        return false;
    }

    gclient_t* client = player->client;

    // Already on this team!
    if (client->jump.pers.team == team) {
        return false;
    }

    client->jump.pers.prev_team = client->jump.pers.team;
    client->jump.pers.team = team;

    client->pers.spectator = (team == JUMP_TEAM_SPECTATOR) ? true : false;

    PutClientInServer(player);

    return true;
}

/*
===============
Jump_TouchEnd

When player has touched the end. Entity may be a weapon or trigger_finish.
===============
*/
qboolean Jump_TouchEnd(edict_t *player, edict_t* ent)
{
#ifdef _DEBUG
    //gi.dprintf("Jump_TouchEnd(%i, %i)\n", player - g_edicts, ent - g_edicts);
#endif

    gclient_t       *client = player->client;
    double          new_time;
    qboolean        is_new_rec;
    qboolean        is_pb;


    if (client->jump.sp.cur_time_msec == 0) {
        return false;
    }

    if (client->jump.sp.finished) {
        return false;
    }


    new_time = client->jump.sp.cur_time_msec / 1000.0f;

    is_new_rec = client->jump.pers.db.best_time <= 0.0;
    is_pb = is_new_rec || new_time < client->jump.pers.db.best_time;

    client->jump.sp.finished = true;
    client->jump.sp.finished_time = (float)new_time;

#ifdef _DEBUG
    gi.dprintf("Finished player's %i run with time %.4f\n", player - g_edicts, client->jump.sp.finished_time);
#endif


    Jump_Replay_OnRunEnd(player, new_time);


    if (client->jump.pers.team == JUMP_TEAM_HARD) {
        Jump_UpdateHighscores(player, new_time);

        if (is_new_rec || is_pb) {
            client->jump.pers.db.best_time = new_time;
        }

        client->jump.pers.db.completions++;

        if (is_new_rec) {
            client->jump.pers.db.num_maps_beaten++;
        }

        
        if (client->jump.pers.db.player_id != INVALID_PLAYER_ID) {
            // save to database.
            dbstore_time_t *data = (dbstore_time_t*)gi.TagMalloc(sizeof(dbstore_time_t), TAG_GAME);
            data->player_id = client->jump.pers.db.player_id;
            data->map_id = 1;
            data->time = client->jump.sp.cur_time_msec / 1000.0;
            data->is_new_rec = is_new_rec;
            data->new_pb = is_pb;

            Jump_Database_StoreTime(data);
        } else {
            // PANIC!!!!
            gi.dprintf("Player with no id attempted to finish a run!!!\n");
        }
    }

    return true;
}

/*
===============
Jump_GetPlayerModel
===============
*/
void Jump_GetPlayerModel(edict_t *player, const char *wanted_mdl, char *out_mdl, size_t out_len)
{
    gclient_t       *client = player->client;

    if (client->jump.pers.team == JUMP_TEAM_HARD) {
        Q_strlcpy(out_mdl, "female/ctf_b", out_len);
    } else {
        Q_strlcpy(out_mdl, "female/ctf_r", out_len);
    }
}
