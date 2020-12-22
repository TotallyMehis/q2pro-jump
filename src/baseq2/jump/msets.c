#include "g_local.h"

#include "util.h"
#include "msets.h"


typedef enum {
    MSET_VALUETYPE_INTEGER = 0,
    MSET_VALUETYPE_BOOLEAN,

    MSET_VALUETYPE_STRING,
} jump_msets_valuetype_t;

typedef enum {
    MSET_FLAG_WORLDSPAWN_CANNOTCHANGE = ( 1 << 0 ),
} jump_msets_flags_t;

typedef struct {
    const char* name;

    void* value_out;

    const jump_msets_valuetype_t value_type;

    const union {
        const char* str;
        int integer;
    } initial_value;

    const jump_msets_flags_t flags;
} jump_mset_data_t;



// interface
static jump_msets_t msets_;
const jump_msets_t *mset = &msets_;


static void ChangeMsetInternal(jump_mset_data_t *data, const char *value);
static qboolean CanChangeMset(jump_mset_change_issuer_t issuer, jump_msets_flags_t flags);



#define REGISTER_MSET_INT(name, value)      {#name, &(msets_.name), MSET_VALUETYPE_INTEGER, value, 0}
#define REGISTER_MSET_BOOL(name, value)     {#name, &(msets_.name), MSET_VALUETYPE_BOOLEAN, value, 0}
#define REGISTER_MSET_STR(name, value)      {#name, &(msets_.name), MSET_VALUETYPE_STRING, value, 0}


/*
===============
Add msets here.
===============
*/
static jump_mset_data_t registered_msets[] = {
    // booleans
    REGISTER_MSET_BOOL(hyperblaster, false),
    REGISTER_MSET_BOOL(rocket, false),
    REGISTER_MSET_BOOL(grenadelauncher, false),
    REGISTER_MSET_BOOL(bfg, false),
    REGISTER_MSET_BOOL(railgun, false),
    REGISTER_MSET_BOOL(weapons, false),

    REGISTER_MSET_BOOL(droptofloor, true),
    REGISTER_MSET_BOOL(rocketjump_fix, false),
    REGISTER_MSET_BOOL(allowsrj, false),
    REGISTER_MSET_BOOL(fasttele, false),

    // integers
    REGISTER_MSET_INT(checkpoint_total, 0),

    // strings
    REGISTER_MSET_STR(ghosty_model, "players/ghost/gumby"),
};

/*
===============
Jump_Mset_Init

Init the msets with default values.
===============
*/
void Jump_Mset_Init(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(registered_msets); i++) {
        switch (registered_msets[i].value_type) {
        case MSET_VALUETYPE_BOOLEAN:
            *((qboolean*)registered_msets[i].value_out) = registered_msets[i].initial_value.integer ? true : false;
            break;
        case MSET_VALUETYPE_INTEGER:
            *((int*)registered_msets[i].value_out) = registered_msets[i].initial_value.integer;
            break;
        case MSET_VALUETYPE_STRING:
            Q_strlcpy((char*)registered_msets[i].value_out, registered_msets[i].initial_value.str, JUMP_MSET_STRING_SIZE);
            break;
        default:
            gi.error("Invalid value type in mset '%s'!\n", registered_msets[i].name);
            break;
        }
    }
}

/*
===============
Jump_Mset_Change

Return true if mset was found and changed.
===============
*/
qboolean Jump_Mset_Change(const char *name, const char *value, jump_mset_change_issuer_t issuer)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(registered_msets); i++) {
        if (Q_stricmp(name, registered_msets[i].name)) {
            continue;
        }

        // found match!
        if (!CanChangeMset(issuer, registered_msets[i].flags)) {
            return false;
        }

        ChangeMsetInternal(&(registered_msets[i]), value);
        return true;
    }

    return false;
}

/*
===============
Jump_Mset_WorldSpawn

Parse msets from worldspawn's mset field.
===============
*/
void Jump_Mset_WorldSpawn(const char *arg)
{
    char buffer[1024];
    char *p;
    const char *args[64];
    int num_args;
    int i;

    if (!arg || arg[0] == NULL)
        return;


    Q_strlcpy(buffer, arg, sizeof(buffer));

    p = buffer;

    args[0] = p;
    num_args = 1;


    p = strtok(p, " ");
    
    while ((p = strtok(NULL, " ")) != NULL && num_args < (ARRAY_SIZE(args) - 1)) {
        *(p - 1) = '\0';
        args[num_args++] = p;
    }

    for (i = 0; i < num_args; i += 2) {
        if (!Jump_Mset_Change(args[i], args[i + 1], JUMP_MSET_ISSUER_WORLDSPAWN)) {
            gi.dprintf("WORLDSPAWN: Couldn't set mset '%s' with value '%s'!\n", args[i], args[i + 1]);
        }
    }
}

/*
===============
Jump_Mset_PrintAll

Prints all msets to player.
===============
*/
void Jump_Mset_PrintAll(edict_t *player)
{
    int i;
    char value[JUMP_MSET_STRING_SIZE];

    for (i = 0; i < ARRAY_SIZE(registered_msets); i++) {
        switch (registered_msets[i].value_type) {
        case MSET_VALUETYPE_BOOLEAN:
            Q_snprintf(value, sizeof(value), "%d", (*(qboolean*)registered_msets[i].value_out));
            break;
        case MSET_VALUETYPE_INTEGER:
            Q_snprintf(value, sizeof(value), "%d", *(int*)registered_msets[i].value_out);
            break;
        case MSET_VALUETYPE_STRING:
            Q_snprintf(value, sizeof(value), "%s", (const char*)registered_msets[i].value_out);
            break;
        default:
            gi.error("Invalid value type in mset '%s'!\n", registered_msets[i].name);
            break;
        }

        gi.cprintf(player, PRINT_HIGH, "'%s' = '%s'\n", registered_msets[i].name, value);
    }
}

static qboolean CanChangeMset(jump_mset_change_issuer_t issuer, jump_msets_flags_t flags)
{
    if (issuer == JUMP_MSET_ISSUER_WORLDSPAWN) {
        return (flags & MSET_FLAG_WORLDSPAWN_CANNOTCHANGE) ? false : true;
    }

    // JUMP TODO: add admin check

    return true;
}

static void ChangeMsetInternal(jump_mset_data_t *data, const char *value)
{
    switch (data->value_type) {
    case MSET_VALUETYPE_BOOLEAN:
        *((qboolean*)data->value_out) = atoi(value) ? true : false;
        break;
    case MSET_VALUETYPE_INTEGER:
        *((int*)data->value_out) = atoi(value);
        break;
    case MSET_VALUETYPE_STRING:
        Q_strlcpy((char*)data->value_out, value, JUMP_MSET_STRING_SIZE);
        break;
    default:
        gi.error("Invalid value type in mset '%s'!\n", data->name);
        break;
    }
}
