#include "g_local.h"

#include "msets.h"
#include "util.h"
#include "weapon.h"

#include <assert.h>

/*
===============
Jump_Weapon_IsEnding

Returns true if this weapon should finish player's run.
===============
*/
qboolean Jump_Weapon_IsEnding(const edict_t *ent)
{
    int i;


    if (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
        return false;

    if (!ent->item)
        return false;

    if (!(ent->item->flags & IT_WEAPON))
        return false;


    const gitem_t* weapons[] = {
        FindItem("Railgun"),
        FindItem("Rocket Launcher"),
        FindItem("HyperBlaster"),
        FindItem("BFG10K"),
        FindItem("Chaingun"),
        FindItem("Grenade Launcher"),
    };

    const qboolean can_be_picked_up[] = {
        mset->railgun,
        mset->rocket,
        mset->hyperblaster,
        mset->bfg,
        false,
        mset->grenadelauncher
    };

    const int num_weapons = ARRAY_SIZE(weapons);
    
    assert(num_weapons == ARRAY_SIZE(can_be_picked_up));

    // find them in the list of allowed finishing weapons.
    for (i = 0; i < num_weapons; i++) {
        if (ent->item == weapons[i]) {
            break;
        }
    }

    // not on the list.
    if (i >= num_weapons) {
        return false;
    }

    // is this finishing weapon allowed to be picked up.
    return !can_be_picked_up[i];
}
