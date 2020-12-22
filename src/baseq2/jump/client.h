#pragma once

void Jump_ClientBegin(edict_t *player);
void Jump_ClientDisconnect(edict_t *player);

void Jump_InitClientPersistant(gclient_t *client);
void Jump_InitClientResp(gclient_t *client);

void Jump_PutClientInServer(edict_t *player);

void Jump_ClientUserinfoChanged(edict_t *player, char *userinfo);

void Jump_ClientThink_Pre(edict_t *player, usercmd_t *ucmd);
void Jump_ClientThink_Post(edict_t *player, usercmd_t *ucmd);

qboolean Jump_IsValidTeam(jump_team_t team);

qboolean Jump_JoinTeam(edict_t *player, jump_team_t team);

qboolean Jump_TouchEnd(edict_t *player, edict_t* ent);

void Jump_GetPlayerModel(edict_t *player, const char *wanted_mdl, char *out_mdl, size_t out_len);
