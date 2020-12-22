#pragma once


/*
===============
Voting interface
===============
*/
qboolean    Jump_Vote_IsVoting(void);

qboolean    Jump_Vote_VoteYes(edict_t *player);
qboolean    Jump_Vote_VoteNo(edict_t *player);

qboolean    Jump_Vote_AttemptStart(edict_t *player, jump_votetype_t type, const char *arg);

void        Jump_Vote_PrintStatus(void);
