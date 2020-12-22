#include "g_local.h"

#include "main.h"


/*
===============
voting.c

The voting stuff.

JUMP TODO: Finish this crap.
===============
*/

typedef struct {
    qboolean can_vote[MAX_CLIENTS];
    int num_voters;

    qboolean voted[MAX_CLIENTS];
    int num_voted;

    // current type they are voting for.
    jump_votetype_t type;

    int num_votes_required;

    // level time
    float vote_end_time;

    // what are they voting about?
    char vote_data[256];

    int last_vote_hud_time;
} jump_voting_t;


jump_voting_t voting = {0};



static qboolean Jump_Vote_FinishVote(void);
void Jump_Vote_OnVotesChanged(void);
void Jump_Vote_UpdateHudTime(void);
void Jump_Vote_PrintStatus(void);


cvar_t *jump_vote_percentage_required;
cvar_t *jump_vote_time;

/*
===============
Jump_Vote_InitCvars

Should only be called once when the game dll is started.
===============
*/
void Jump_Vote_InitCvars(void)
{
    jump_vote_percentage_required = gi.cvar("jump_vote_percentage_required", "75", CVAR_ARCHIVE);
    jump_vote_time = gi.cvar("jump_vote_time", "60", CVAR_ARCHIVE);
}

/*
===============
Jump_Vote_Init

Called every map change.
===============
*/
void Jump_Vote_Init(void)
{
    voting.type = JUMP_VOTE_NONE;

    memset(voting.can_vote, 0, sizeof(voting.can_vote));
    voting.num_voters = 0;

    memset(voting.voted, 0, sizeof(voting.voted));
    voting.num_voted = 0;

    voting.num_votes_required = 0;

    voting.vote_end_time = 0;

    voting.last_vote_hud_time = 0;
}

void Jump_Vote_Disconnected(edict_t *player)
{
    int playernum = player - g_edicts - 1;

    voting.can_vote[playernum] = false;
    
    if (voting.voted[playernum]) {
        voting.voted[playernum] = false;
        voting.num_voted--;

        Jump_Vote_OnVotesChanged();

        Jump_Vote_PrintStatus();
    }
}

/*
===============
Jump_Vote_CanStartVote

Check if the player is allowed to start this type of vote. (ie. admin check)
===============
*/
qboolean Jump_Vote_CanStartVote(edict_t *player, jump_votetype_t type, const char *arg)
{
    return true;
}

/*
===============
Jump_Vote_CheckRules
===============
*/
qboolean Jump_Vote_CheckRules(void)
{
    if (voting.type == JUMP_VOTE_NONE)
        return false;


    Jump_Vote_UpdateHudTime();


    if (voting.num_voted >= voting.num_votes_required) {
        Jump_Vote_FinishVote();
        return true;
    }

    if (level.time >= voting.vote_end_time) {
        
        return true;
    }

    return false;
}

/*
===============
Jump_Vote_FinishVote

JUMP TODO: Finish voting
===============
*/
qboolean Jump_Vote_FinishVote(void)
{
    
}

/*
===============
Jump_Vote_IsVoting
===============
*/
qboolean Jump_Vote_IsVoting(void)
{
    return voting.type != JUMP_VOTE_NONE;
}

void Jump_Vote_PrintStatus(void)
{
    gi.bprintf(PRINT_CHAT, "Votes: %d  Needed: %d  Time left: %ds\n",
        voting.num_voted,
        voting.num_votes_required,
		(int)(voting.vote_end_time - level.time)
    );
}

/*
===============
Jump_Vote_VoteYes
===============
*/
qboolean Jump_Vote_VoteYes(edict_t *player)
{
    int playernum = player - g_edicts - 1;

    if (voting.type == JUMP_VOTE_NONE) {
        gi.cprintf(player, PRINT_HIGH, "There is no vote going on!\n");
        return false;
    }

    if (voting.voted[playernum]) {
        gi.cprintf(player, PRINT_HIGH, "You've already voted!\n");
        return false;
    }

    if (!voting.can_vote[playernum]) {
        gi.cprintf(player, PRINT_HIGH, "You cannot vote in this election!\n");
        return false;
    }

    voting.voted[playernum] = true;
    voting.num_voted++;

    Jump_Vote_OnVotesChanged();

    Jump_Vote_PrintStatus();
}

/*
===============
Jump_Vote_VoteNo
===============
*/
qboolean Jump_Vote_VoteNo(edict_t *player)
{
    int playernum = player - g_edicts - 1;

    if (voting.type == JUMP_VOTE_NONE) {
        gi.cprintf(player, PRINT_HIGH, "There is no vote going on!\n");
        return false;
    }

    if (voting.voted[playernum]) {
        gi.cprintf(player, PRINT_HIGH, "You've already voted!\n");
        return false;
    }

    if (!voting.can_vote[playernum]) {
        gi.cprintf(player, PRINT_HIGH, "You cannot vote in this election!\n");
        return false;
    }

    voting.can_vote[playernum] = false;

    Jump_Vote_PrintStatus();
}

/*
===============
Jump_Vote_FormatVoteType
===============
*/
void Jump_Vote_FormatVoteType(char* dst, size_t dst_len, jump_votetype_t type, const char *arg)
{
    int i;

    switch(type) {
    case JUMP_VOTE_EXTEND:
        i = atoi(arg);
        Q_snprintf(dst, dst_len, "Extend by %i", i);
        break;
    case JUMP_VOTE_CHANGEMAP:
        Q_snprintf(dst, dst_len, "Change map to %s", arg);
        break;
    default:
        Q_strlcpy(dst, "N/A", dst_len);
        break;
    }
}

/*
===============
Jump_Vote_OnVotesChanged

Change the HUD state.
===============
*/
void Jump_Vote_OnVotesChanged(void)
{
    gi.configstring(
        JUMP_CONFIG_VOTE_CAST,
        va("Votes: %i of %i", voting.num_voted, voting.num_votes_required)
    );
}

void Jump_Vote_UpdateHudTime(void)
{
    int new_time = (int)(level.time - jump_vote_time->value);
    
    if (new_time != voting.last_vote_hud_time) {
        gi.configstring(
            JUMP_CONFIG_VOTE_REMAINING,
            va("%i seconds", (int)jump_vote_time->value)
        );

        voting.last_vote_hud_time = new_time;
    }
}

/*
===============
Jump_Vote_AttemptStart

A player is trying to start a vote. How cute.
===============
*/
qboolean Jump_Vote_AttemptStart(edict_t *player, jump_votetype_t type, const char *arg)
{
    int i;
    gclient_t *cl;
    edict_t *cl_ent;
    int playernum = player - g_edicts - 1;
    float req;
    char buffer[256];


    if (voting.type != JUMP_VOTE_NONE) {
        gi.cprintf(player, PRINT_HIGH, "There is already a vote going on!\n");
        return false;
    }

    if (!Jump_Vote_CanStartVote(player, type, arg)) {
        gi.cprintf(player, PRINT_HIGH, "You cannot start vote!\n");
        return false;
    }


    voting.type = type;

    memset(voting.can_vote, 0, sizeof(voting.can_vote));
    voting.num_voters = 0;

    memset(voting.voted, 0, sizeof(voting.voted));
    voting.num_voted = 0;

    voting.voted[playernum] = true;
    voting.num_voters = 1;
    voting.num_voted = 1;

    // count possible voters
    for (i = 0; i <= game.maxclients; i++) {
        cl_ent = g_edicts + i;
		cl = cl_ent->client;

		if (!cl_ent->inuse)
			continue;
		if (!cl)
			continue;
        if (cl_ent == player)
            continue;

        // idle, don't count me :)
        if (cl->jump.pers.idle_state != JUMP_IDLESTATE_NONE)
            continue;


        voting.can_vote[cl_ent - g_edicts - 1] = true;
        voting.num_voters++;
    }

    // set number of votes required.
    req = jump_vote_percentage_required->value * 0.01f * voting.num_voters;

    if (voting.num_voters == 2) {
        req = 2;
    } else if (req < 1.0f) {
        req = 1;
    }

    voting.num_votes_required = (int)req;

    // ending time
    voting.vote_end_time = level.time + jump_vote_time->value;


    gi.configstring(
        JUMP_CONFIG_VOTE_INITIATED,
        va("Vote by %s", player->client->pers.netname)
    );
    Jump_Vote_FormatVoteType(buffer, sizeof(buffer), voting.type, arg);
    gi.configstring(JUMP_CONFIG_VOTE_TYPE, buffer);

    Jump_Vote_UpdateHudTime();

    Jump_Vote_CheckRules();

    return true;
}
