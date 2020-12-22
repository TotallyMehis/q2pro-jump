#include "g_local.h"

#include "cmds.h"

#include "util.h"
#include "replay.h"
#include "msets.h"
#include "voting.h"
#include "database.h"
#include "client.h"

#include <assert.h>


void Jump_Scoreboard_1(edict_t *player);
void Jump_Scoreboard_2(edict_t *player);


jump_cmdret_t Cmd_Team(edict_t *player);
jump_cmdret_t Cmd_Recall(edict_t *player);
jump_cmdret_t Cmd_Store(edict_t *player);
jump_cmdret_t Cmd_Idle(edict_t *player);
jump_cmdret_t Cmd_Noclip(edict_t *player);
jump_cmdret_t Cmd_Replay(edict_t *player);
jump_cmdret_t Cmd_ToggleShowPlayers(edict_t *player);
jump_cmdret_t Cmd_PrintRankList(edict_t *player);
jump_cmdret_t Cmd_VoteYes(edict_t *player);
jump_cmdret_t Cmd_VoteNo(edict_t *player);
jump_cmdret_t Cmd_VoteMap(edict_t *player);
jump_cmdret_t Cmd_VoteTime(edict_t *player);
jump_cmdret_t Cmd_Scoreboard(edict_t *player);
jump_cmdret_t Cmd_Mset(edict_t *player);
jump_cmdret_t Cmd_MsetPrint(edict_t *player);


typedef jump_cmdret_t (*jump_cmd_cb)(edict_t *player);

typedef struct jump_cmdhelper_s {
    const char* cmd;
    jump_cmd_cb cb;
} jump_cmdhelper_t;


#define REGISTER_CMD(name, cb)            {#name, cb}


/*
===============
Add commands here.
===============
*/
static jump_cmdhelper_t cmds[] = {
    // team
    REGISTER_CMD(team, Cmd_Team),
    REGISTER_CMD(join, Cmd_Team),

    REGISTER_CMD(kill, Cmd_Recall),

    // store/recall
    REGISTER_CMD(recall, Cmd_Recall),
    REGISTER_CMD(store, Cmd_Store),

    // misc
    REGISTER_CMD(idle, Cmd_Idle),
    REGISTER_CMD(noclip, Cmd_Noclip),
    REGISTER_CMD(replay, Cmd_Replay),
    REGISTER_CMD(jumpers, Cmd_ToggleShowPlayers),

    REGISTER_CMD(ranks, Cmd_PrintRankList),

    // voting
    REGISTER_CMD(yes, Cmd_VoteYes),
    REGISTER_CMD(no, Cmd_VoteNo),
    REGISTER_CMD(votemap, Cmd_VoteMap),
    REGISTER_CMD(votetime, Cmd_VoteTime),
    REGISTER_CMD(timevote, Cmd_VoteTime),

    // scoreboard
    REGISTER_CMD(score, Cmd_Scoreboard),
    REGISTER_CMD(help, Cmd_Scoreboard),

    // mset
    REGISTER_CMD(mset, Cmd_Mset),
    REGISTER_CMD(mset_print, Cmd_MsetPrint),
};


/*
===============
Jump_ClientCommand
===============
*/
jump_cmdret_t Jump_ClientCommand(edict_t *player)
{
    int i;
    const char* cmd = gi.argv(0);
    jump_cmdret_t cmdret = JUMP_CMDRET_FAILED;

    // loop through all commands
    for (i = 0; i < ARRAY_SIZE(cmds); i++) {
        if (!Q_stricmp(cmd, cmds[i].cmd)) {
            // found match!
            cmdret = cmds[i].cb(player);

            switch (cmdret) {
            case JUMP_CMDRET_FAILED:
            case JUMP_CMDRET_FAILED_SILENTLY:
                break;
            case JUMP_CMDRET_OK:
            default:
                return JUMP_CMDRET_OK;
            }
        }
    }

    return cmdret;
}



/*
===============
Team
===============
*/

jump_cmdret_t Cmd_Team(edict_t *player)
{
    const char* team_name;
    jump_team_t team_num = JUMP_TEAM_NONE;

    if (gi.argc() <= 1) {
        gi.cprintf(player, PRINT_HIGH, "Usage: %s easy/hard/spectator\n", gi.argv(0));
        return JUMP_CMDRET_FAILED_SILENTLY;
    }

    team_name = gi.argv(1);

    if (team_name && team_name[0]) {
        if (Q_isdigit(team_name[0])) {
            team_num = (jump_team_t)atoi(team_name);
        } else {
            if (!Q_stricmp(team_name, "easy")) {
                team_num = JUMP_TEAM_EASY;
            } else if (!Q_stricmp(team_name, "hard")) {
                team_num = JUMP_TEAM_HARD;
            } else if ( !Q_stricmp(team_name, "spectator")||
                        !Q_stricmp(team_name, "spectate") ||
                        !Q_stricmp(team_name, "observe") ||
                        !Q_stricmp(team_name, "observer")) {
                team_num = JUMP_TEAM_SPECTATOR;
            }
        }
    }


    if (!Jump_IsValidTeam(team_num)) {
        gi.cprintf(player, PRINT_HIGH, "Invalid team '%s'. Use 'easy', 'hard' or 'spectator' instead.\n", team_name);
        return JUMP_CMDRET_FAILED_SILENTLY;
    }


    Jump_JoinTeam(player, team_num);

    return JUMP_CMDRET_OK;
}


/*
===============
UI
===============
*/
jump_cmdret_t Cmd_Scoreboard(edict_t *player)
{
    switch (++player->client->jump.pers.scoreboard) {
    case 1:
        Jump_Scoreboard_1(player);
        break;
    case 2:
        Jump_Scoreboard_2(player);
        break;
    default:
        player->client->jump.pers.scoreboard = 0;
        break;
    }

    return JUMP_CMDRET_OK;
}

/*
===============
Misc
===============
*/

jump_cmdret_t Cmd_Noclip(edict_t *player)
{
    gclient_t   *client = player->client;
    char        msgbuffer[64];

    if (player->deadflag) {
        gi.cprintf(player, PRINT_HIGH, "You must be alive to noclip!\n");
        return JUMP_CMDRET_FAILED_SILENTLY;
    }

    if (client->jump.pers.team != JUMP_TEAM_EASY) {
        gi.cprintf(player, PRINT_HIGH, "You must be on team Easy!\n");
        return JUMP_CMDRET_FAILED_SILENTLY;
    }


    player->movetype = (player->movetype == MOVETYPE_NOCLIP) ? MOVETYPE_WALK : MOVETYPE_NOCLIP;

    Q_strlcpy(msgbuffer, (player->movetype == MOVETYPE_NOCLIP) ? "ON" : "OFF", sizeof(msgbuffer));
    Jump_Util_Highlight(msgbuffer);

    gi.cprintf(player, PRINT_HIGH, "Noclip: %s\n", msgbuffer);

    return JUMP_CMDRET_OK;
}

jump_cmdret_t Cmd_Idle(edict_t *player)
{
    gclient_t *client = player->client;

    if (client->jump.pers.idle_state != JUMP_IDLESTATE_SELF) {
        client->jump.pers.idle_state = JUMP_IDLESTATE_SELF;
    } else {
        client->jump.pers.idle_state = JUMP_IDLESTATE_NONE;
        client->jump.pers.idle_time = 0;
    }
    

    return JUMP_CMDRET_OK;
}

static void PrintRankList(dbcb_ranklist_t *ranklist)
{
    assert(ranklist->player_num >= 0);

    edict_t *player = &(g_edicts[ranklist->player_num + 1]);
    gclient_t *client = player->client;
    int i;

    if (!player->inuse || !client)
        return;


    gi.cprintf(player, PRINT_HIGH, "\n-----------------------------------------\n");
    // No. Name 1st 2nd 3rd 4th 5th Score
    gi.cprintf(player, PRINT_HIGH, "\xce\xef\xae \xce\xe1\xed\xe5             \xb1\xf3\xf4 \xb2\xee\xe4 \xb3\xf2\xe4 \xb4\xf4\xe8 \xb5\xf4\xe8 \xd3\xe3\xef\xf2\xe5\n"); 

    for (i = 0; i < ranklist->num_retrieved; i++) {
        gi.cprintf(player, PRINT_HIGH,
            "%-3d %-16s %3d %3d %3d %3d %3d %5d\n",
            i + 1,
            ranklist->player_name[i],
            ranklist->num_1[i],
            ranklist->num_2[i],
            ranklist->num_3[i],
            ranklist->num_4[i],
            ranklist->num_5[i],
            ranklist->total_points[i]
        );
    }

    gi.cprintf(player, PRINT_HIGH, "\nPage %i\n", (ranklist->offset / ranklist->count) + 1);
    gi.cprintf(player, PRINT_HIGH, "-----------------------------------------\n");
}

jump_cmdret_t Cmd_PrintRankList(edict_t *player)
{
    const int rows_on_page = 15;
    int page = 1;

    if (gi.argc() > 1) {
        page = abs(atoi(gi.argv(1)));

        page = max(1, page);
    }


    dbload_ranklist_t *data = gi.TagMalloc(sizeof(dbload_ranklist_t), TAG_GAME);
    data->count = rows_on_page;
    data->offset = rows_on_page * (page - 1);
    data->player_num = player - g_edicts - 1;

    Jump_Database_FetchRankList(data, PrintRankList);

    return JUMP_CMDRET_OK;
}


/*
===============
Store
===============
*/

jump_cmdret_t Cmd_Recall(edict_t *player)
{
    gclient_t *client = player->client;
    qboolean switched = false;


    if (client->jump.pers.team != JUMP_TEAM_EASY) {

        if (client->jump.pers.team == JUMP_TEAM_SPECTATOR) {
            // Try previous team.
            Jump_JoinTeam(player, Jump_IsValidTeam(client->jump.pers.prev_team) ? client->jump.pers.prev_team : JUMP_TEAM_EASY);
        } else {
            switched = Jump_JoinTeam(player, JUMP_TEAM_HARD);
            if (!switched) {
                PutClientInServer(player);
            }
        }
        
        return JUMP_CMDRET_OK;
    }



    return JUMP_CMDRET_OK;
}

jump_cmdret_t Cmd_Store(edict_t *player)
{
    gclient_t *client = player->client;

    if (client->jump.pers.team == JUMP_TEAM_SPECTATOR || player->deadflag) {
        gi.cprintf(player, PRINT_HIGH, "You must be alive to store!\n");
        return JUMP_CMDRET_FAILED_SILENTLY;
    }

    return JUMP_CMDRET_OK;
}

/*
===============
Voting
===============
*/

jump_cmdret_t Cmd_VoteMap(edict_t *player)
{
    if (gi.argc() < 2) {
        gi.cprintf(player, PRINT_HIGH, "Usage: %s <time in minutes>\n", gi.argv(0));
        return JUMP_CMDRET_FAILED_SILENTLY;
    }

    Jump_Vote_AttemptStart(player, JUMP_VOTE_CHANGEMAP, gi.argv(1));

    return JUMP_CMDRET_OK;
}

jump_cmdret_t Cmd_VoteTime(edict_t *player)
{
    if (gi.argc() < 2) {
        gi.cprintf(player, PRINT_HIGH, "Usage: %s <time in minutes>\n", gi.argv(0));
        return JUMP_CMDRET_FAILED_SILENTLY;
    }

    Jump_Vote_AttemptStart(player, JUMP_VOTE_EXTEND, gi.argv(1));

    return JUMP_CMDRET_OK;
}



jump_cmdret_t Cmd_VoteYes(edict_t *player)
{
    if (!Jump_Vote_VoteYes(player)) {
        return JUMP_CMDRET_FAILED;
    }

    return JUMP_CMDRET_OK;
}

jump_cmdret_t Cmd_VoteNo(edict_t *player)
{
    if (!Jump_Vote_VoteNo(player)) {
        return JUMP_CMDRET_FAILED;
    }

    return JUMP_CMDRET_OK;
}

/*
===============
Misc
===============
*/

jump_cmdret_t Cmd_Replay(edict_t *player)
{
    int replay_num = 1;

    if (gi.argc() > 1) {
        const char* arg = gi.argv(1);

        if (Q_isdigit(arg[0])) {
            replay_num = atoi(arg);
        }
    }

    Jump_Replay_StartWatching(player, replay_num);

    return JUMP_CMDRET_OK;
}

jump_cmdret_t Cmd_ToggleShowPlayers(edict_t *player)
{
    // JUMP TODO
    player->client->jump.pers.hide_others = !player->client->jump.pers.hide_others;

    return JUMP_CMDRET_OK;
}

/*
===============
MSET
===============
*/

jump_cmdret_t Cmd_Mset(edict_t *player)
{
    const char *name;
    const char *value;
    qboolean changed;
    jump_mset_change_issuer_t issuer;

    if (gi.argc() < 3) {
        gi.cprintf(player, PRINT_HIGH, "Usage: mset <name> <value>\n");
        return JUMP_CMDRET_FAILED_SILENTLY;
    }


    name = gi.argv(1);
    value = gi.argv(2);

    if (player == g_edicts) {
        issuer = JUMP_MSET_ISSUER_SERVER_CONSOLE;
    } else {
        issuer = JUMP_MSET_ISSUER_PLAYER_START + (player - g_edicts);
    }

    changed = Jump_Mset_Change(name, value, issuer);


    if (changed) {
        gi.cprintf(player, PRINT_HIGH, "'%s' is now '%s'!\n", name, value);
    } else {
        gi.cprintf(player, PRINT_HIGH, "Unable to change '%s' to '%s'\n", name, value);
    }

    return JUMP_CMDRET_OK;
}

jump_cmdret_t Cmd_MsetPrint(edict_t *player)
{
    Jump_Mset_PrintAll(player);
    return JUMP_CMDRET_OK;
}
