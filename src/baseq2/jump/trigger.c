#include "g_local.h"

#include "client.h"


void InitTrigger(edict_t *self);


/*
==============================================================================

trigger_finish

==============================================================================
*/
void trigger_finish_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    if (!other->client)
        return;

    Jump_TouchEnd(other, self);
}

void SP_trigger_finish(edict_t *self)
{
    InitTrigger(self);
    self->touch = trigger_finish_touch;
}
