#pragma once


typedef enum jump_cmdret_e {
    // something went wrong.
    JUMP_CMDRET_FAILED = 0,

    // command failed, but fail silently.
    JUMP_CMDRET_FAILED_SILENTLY,
    
    // command was run successfully.
    JUMP_CMDRET_OK,
} jump_cmdret_t;


jump_cmdret_t Jump_ClientCommand(edict_t *player);

