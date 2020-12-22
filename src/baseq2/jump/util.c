#include "g_local.h"

#include "util.h"

// JUMP HACK
#undef svc_muzzleflash
#undef svc_muzzleflash2
#undef svc_temp_entity
#undef svc_layout
#undef svc_inventory
#undef svc_stufftext
#undef svc_sound

#include <common/protocol.h>


void Jump_Util_PadToRight(char* dst, size_t dst_len, const char* src)
{
    size_t src_len = strlen(src);
    
    if (!src_len || src_len >= dst_len) {
        Q_strlcpy(dst, src, dst_len);
        return;
    }
    
    memset(dst, ' ', dst_len - 1);
    dst[dst_len - 1] = NULL;

    Q_strlcpy(&(dst[dst_len - src_len - 1]), src, src_len + 1);
}

void Jump_Util_Highlight(char* dst)
{
    unsigned int i;
    size_t len;

    if (!dst || dst[0] == NULL)
        return;

    len = strlen(dst);
    for (i = 0; i < len; i++ ) {
        if (Q_isalnum(dst[i])) {
            dst[i] |= 0x80;
        }
    }
}

/*
===============
Jump_Util_PlaySound

Plays a sound and takes into account if the source player
is hidden for other players.
===============
*/
void Jump_Util_PlaySound(edict_t *src, int sound_index, int channel_in, float volume, int attenuation)
{
    edict_t     *cl_ent;
    gclient_t   *cl;
    int         flags;
    int         ent_num;
    int         channel;
    int         i;


    flags = SND_VOLUME | SND_ATTENUATION | SND_ENT;
    ent_num = src - globals.edicts;
    // entity and channel are encoded in the channel portion.
    channel = (ent_num << 3) | (channel_in & 7);

	for (i = 1; i <= maxclients->value; i++) {
		cl_ent = g_edicts + i;
        cl = cl_ent->client;

		if (!cl_ent->inuse)
			continue;
		if (!cl)
			continue;

        // wants to hide players or should be played to a single player.
        if (cl->jump.pers.hide_others) {
            // must not be the chase target.
            if (cl_ent != src && cl->chase_target != src) {
                continue;
            }
        }

        gi.WriteByte(svc_sound);
        gi.WriteByte(flags);
        gi.WriteByte(sound_index);
        gi.WriteByte(volume);
        gi.WriteByte(attenuation);
        gi.WriteShort(channel);
        gi.unicast(cl_ent, true);
    }
}

/*
===============
Jump_Util_PlaySoundLocal

Plays a sound to the single player and takes into account
if other players are spectating them.
===============
*/
void Jump_Util_PlaySoundLocal(edict_t *src, int sound_index, int channel_in, float volume, int attenuation)
{
    edict_t     *cl_ent;
    gclient_t   *cl;
    int         flags;
    int         ent_num;
    int         channel;
    int         i;


    flags = SND_VOLUME | SND_ATTENUATION | SND_ENT;
    ent_num = src - globals.edicts;
    // entity and channel are encoded in the channel portion.
    channel = (ent_num << 3) | (channel_in & 7);

	for (i = 1; i <= maxclients->value; i++) {
		cl_ent = g_edicts + i;
        cl = cl_ent->client;

		if (!cl_ent->inuse)
			continue;
		if (!cl)
			continue;

        if (cl_ent != src && cl->chase_target != src) {
            continue;
        }

        gi.WriteByte(svc_sound);
        gi.WriteByte(flags);
        gi.WriteByte(sound_index);
        gi.WriteByte(volume);
        gi.WriteByte(attenuation);
        gi.WriteShort(channel);
        gi.unicast(cl_ent, true);
    }
}

