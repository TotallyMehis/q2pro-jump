#pragma once

/*
=====================
MSETs, ie. map specific configuration values

Not quite like cvars... but essentially that yea... -_-
=====================
*/

#define JUMP_MSET_STRING_SIZE       256

typedef enum {
    // worldspawn (the mapper).
    JUMP_MSET_ISSUER_WORLDSPAWN = -2,

    // server console
    JUMP_MSET_ISSUER_SERVER_CONSOLE = -1,

    // start players here.
    JUMP_MSET_ISSUER_PLAYER_START = 0,


} jump_mset_change_issuer_t;


typedef struct {
    // is the gun 'hyperblaster' allowed to be picked up?
    // otherwise picking up said gun will act as finishing a run.
    qboolean hyperblaster;

    // is the gun 'rocket launcher' allowed to be picked up?
    // otherwise picking up said gun will act as finishing a run.
    qboolean rocket;

    // is the gun 'grenade launcher' allowed to be picked up?
    // otherwise picking up said gun will act as finishing a run.
    qboolean grenadelauncher;

    // is the gun 'bfg' allowed to be picked up?
    // otherwise picking up said gun will act as finishing a run.
    qboolean bfg;

    // is the gun 'railgun' allowed to be picked up?
    // otherwise picking up said gun will act as finishing a run.
    qboolean railgun;

    // are other weapons allowed to be fired?
    // weapon specific rules do not follow this one.
    qboolean weapons;

    // are items (weapons) dropped to the floor?
    // by default this is true.
    qboolean droptofloor;

    // some weird rocket jump fix(????)
    qboolean rocketjump_fix;

    // anti super rocket jump fix(????)
    qboolean allowsrj;

    // teleports by default freeze the player inputs for a bit.
    qboolean fasttele;

    // how many checkpoints player needs before finishing a run.
    int checkpoint_total;

    // path to the replay ghost model
    char ghosty_model[JUMP_MSET_STRING_SIZE];
} jump_msets_t;


extern const jump_msets_t *mset;


qboolean Jump_Mset_Change(const char *name, const char *value, jump_mset_change_issuer_t issuer);
void Jump_Mset_PrintAll(edict_t *player);

