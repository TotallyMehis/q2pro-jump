#include "g_local.h"

#include "hud.h"
#include "main.h"

/*
==============
Jump_Scoreboard_1

Prints our scoreboard (first half).
==============
*/
void Jump_Scoreboard_1(edict_t *ent)
{
	char				entry[1024];
	char				string[1400];
	int					stringlength;
	int					i, j, k;
	int					sorted[MAX_CLIENTS];
	float				sortedscores[MAX_CLIENTS];
	float				score;
	int					total;
	int					picnum;
	int					y;
	gclient_t			*cl;
	edict_t				*cl_ent;
	int					total_easy;
	int					total_specs;
	char				teamstring[5];
	qboolean			idle = false;

	//
	// sort the clients by score
	//
	total = 0;
	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		cl = cl_ent->client;

		if (!cl_ent->inuse)
			continue;
		if (!cl)
			continue;

		if (cl->jump.pers.team != JUMP_TEAM_HARD)
			continue;


		score = (float)cl->jump.pers.db.num_maps_beaten;
		if (score <= 0)
			score = 99998;

		for (j = 0; j < total; j++) {
			if (score < sortedscores[j])
				break;
		}

		for (k = total; k > j; k--) {
			sorted[k] = sorted[k - 1];
			sortedscores[k] = sortedscores[k - 1];
		}

		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	//if (total > 16)
	//	total = 16;

	
	Q_snprintf(entry, sizeof(entry),
		"xv -16 yv 0 string2 \"Ping Pos Player          Best Comp Maps     %%  Team\" ");
	j = strlen(entry);
	Q_strlcpy(string + stringlength, entry, sizeof(string) - stringlength);
	stringlength += j;

	for (i = 0; i < total; i++) {
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex("i_fixme");
		y = 16 + 10 * i;



		// send the layout
		if (cl->jump.pers.idle_state != JUMP_IDLESTATE_NONE) {
			Q_strlcpy(teamstring, "Idle", sizeof(teamstring));
		} else {
			Q_strlcpy(teamstring, "Hard", sizeof(teamstring));
		}

		if (cl->jump.pers.db.best_time != INVALID_TIME) {
			Q_snprintf(entry, sizeof(entry),
				"ctf %d %d %d %d %d xv 152 string \"%8.3f %4i %4i  %4.1f  %s\"",
				-8, y, sorted[i], cl->ping, cl->jump.pers.db.rank,
				cl->jump.pers.db.best_time,
				cl->jump.pers.db.completions,

				cl->jump.pers.db.num_maps_beaten,
				cl->jump.pers.db.num_maps_beaten / (float)jump.num_maps * 100.0f,
				teamstring
			);

		} else {
			if (cl->jump.pers.db.rank != INVALID_RANK) {
				Q_snprintf(entry, sizeof(entry),
					"ctf %d %d %d %d %d xv 152 string \"  ------ ---- %4i  %4.1f  %s\"",
					-8, y, sorted[i], cl->ping, cl->jump.pers.db.rank,
					cl->jump.pers.db.num_maps_beaten,
					cl->jump.pers.db.num_maps_beaten / (float)jump.num_maps * 100.0f,
					teamstring
				);
			} else {
				Q_snprintf(entry, sizeof(entry),
					"ctf %d %d %d %d %d xv 152 string \"    ------ ----           %s\"",
					-8, y, sorted[i], cl->ping, 1000, teamstring
				);
			}
		}

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		Q_strlcpy(string + stringlength, entry, sizeof(string) - stringlength);
		stringlength += j;
	}

	//
	// easy team
	//
	total_easy = 0;
	total_specs = 0;
	for (i = 0; i < maxclients->value; i++) {
		cl = &game.clients[i];
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->jump.pers.team == JUMP_TEAM_SPECTATOR) {
			total_specs++;
			continue;
		}
		if (cl_ent->client->jump.pers.team != JUMP_TEAM_EASY)
			continue;

		if (total) {
			//if hard team has players, increase gap
			y = 24 + (10 * (total_easy + total));
		} else {
			y = 16 + (10 * (total_easy));
		}

		if (cl->jump.pers.idle_state != JUMP_IDLESTATE_NONE) {
			Q_strlcpy(teamstring, "Idle", sizeof(teamstring));
		} else {
			Q_strlcpy(teamstring, "Easy", sizeof(teamstring));
		}

		if (cl->jump.pers.db.best_time != INVALID_TIME) {
			Q_snprintf(entry, sizeof(entry),
				"ctf %d %d %d %d %d xv 152 string \"%8.3f %4i %4i  %4.1f  %s\"",
				-8, y, i, cl->ping, cl->jump.pers.db.rank,
				cl->jump.pers.db.best_time,
				cl->jump.pers.db.completions,

				cl->jump.pers.db.num_maps_beaten,
				cl->jump.pers.db.num_maps_beaten / (float)(jump.num_maps) * 100.0f,
				teamstring
			);
		} else {
			if (cl->jump.pers.db.rank != INVALID_RANK) {
				Q_snprintf(entry, sizeof(entry),
					"ctf %d %d %d %d %d xv 152 string \"  ------ ---- %4i  %4.1f  %s\"",
					-8, y, i, cl->ping, cl->jump.pers.db.rank,
					cl->jump.pers.db.num_maps_beaten,
					cl->jump.pers.db.num_maps_beaten / (float)jump.num_maps * 100.0f,
					teamstring
				);
			} else {
				Q_snprintf(entry, sizeof(entry),
					"ctf %d %d %d %d %d xv 152 string \"    ------ ----           %s\"",
					-8, y, i, cl->ping, 1000, teamstring
				);

			}
		}

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		Q_strlcpy(string + stringlength, entry, sizeof(string) - stringlength);
		stringlength += j;
		total_easy++;
	}

	//spectators

	if ((total) && (total_easy)) {
		//if we have players on both teams, theres an extra 8 gap
		y = 48 + (8 * (total + total_easy));
	} else {
		y = 40 + (8 * (total + total_easy));
	}

	if (total_specs) {
		Q_snprintf(entry, sizeof(entry),
			"xv -16 yv %d string2 \"Spectators\" ", y);
		j = strlen(entry);
		Q_strlcpy(string + stringlength, entry, sizeof(string) - stringlength);
		stringlength += j;
	}

	//any spectators idle, if so, add extra gap for the idle tag...
	for (i = 0; i < maxclients->value; i++) {
		cl = &game.clients[i];
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->jump.pers.team != JUMP_TEAM_SPECTATOR)
			continue;
		if (cl_ent->client->jump.pers.idle_state != JUMP_IDLESTATE_NONE)
			idle = true;
	}


	total_specs = 0;
	for (i = 0; i < maxclients->value; i++) {
		cl = &game.clients[i];
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->jump.pers.team != JUMP_TEAM_SPECTATOR)
			continue;
		if ((total) && (total_easy)) {
			//if we have players on both teams, theres an extra 8 gap
			y = 56 + (8 * (total + total_easy + total_specs));
		} else {
			y = 48 + (8 * (total + total_easy + total_specs));
		}

		// add idle tag if spectator is idle.
		if (cl->jump.pers.idle_state != JUMP_IDLESTATE_NONE) {
			Q_snprintf(entry, sizeof(entry),
				"xv %d yv %d string \" (idle)\"", 56 + (strlen(cl->pers.netname) * 8), y);
			j = strlen(entry);
			Q_strlcpy(string + stringlength, entry, sizeof(string) - stringlength);
			stringlength += j;
		}

		//if (cl->resp.replaying) {
		//	if (cl->resp.replaying == MAX_HIGHSCORES + 1) {
		//		Q_snprintf(entry, sizeof(entry),
		//			"ctf %d %d %d %d %d xv %d string \" (Replay now)\"",
		//			-8, y, i,
		//			cl->ping,
		//			0, idle ? 224 : 168
		//		);
		//	} else {
		//		Q_snprintf(entry, sizeof(entry),
		//			"ctf %d %d %d %d %d xv %d string \" (Replay %d)\"",
		//			-8, y, i,
		//			cl->ping,
		//			0, idle ? 224 : 168,
		//			cl->resp.replaying

		//		);
		//	}
		//}
		//else
		{
			Q_snprintf(entry, sizeof(entry),
				"ctf %d %d %d %d %d xv %d string \"%s%s\"",
				-8, y, i,
				cl->ping,
				0, idle ? 224 : 168,
				cl->chase_target ? " -> " : "",
				cl->chase_target ? cl->chase_target->client->pers.netname : ""
			);
		}


		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		Q_strlcpy(string + stringlength, entry, sizeof(string) - stringlength);
		stringlength += j;

		total_specs++;
	}

	y += 64;

	Q_snprintf(entry, sizeof(entry),
		"xv -16 yv %d string2 \"Next Maps (type nominate <map> or rand)\" yv %d stat_string %d yv %d stat_string %d yv %d stat_string %d ",
		y,
		y + 16,
		JUMP_STAT_NEXTMAP1,
		y + 24,
		JUMP_STAT_NEXTMAP2,
		y + 32,
		JUMP_STAT_NEXTMAP3
	);

	j = strlen(entry);
	Q_strlcpy(string + stringlength, entry, sizeof(string) - stringlength);
	stringlength += j;

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast(ent, true);
}

/*
==============
Jump_Scoreboard_2

Prints our scoreboard (second half).
The best times.
==============
*/
void Jump_Scoreboard_2(edict_t *ent)
{
	char		string[1400];
	char		entry[256];
	int			i, j;
	size_t		stringlength = 0;
	

	stringlength = Q_strlcpy(string, "xv 0 yv 0 string2 \"No   Player              Time  Date \" ", sizeof(string));

	for (i = 0; i < MAX_HIGHSCORES; i++) {
		if (i < jump.map.num_best_times) {
			Q_snprintf(entry, sizeof(entry), "yv %d %s \"%2d%s %s%-16s%8.3f  %s\" ",
				i * 10 + 16,
				"string",
				i + 1,
				" ", //(level_items.recorded_time_frames[i] == 0 ? " " : "\x0D"), // JUMP TODO
				jump.map.best[i].was_beaten_this_session ? "*" : " ",
				jump.map.best[i].player_name,
				jump.map.best[i].time,
				jump.map.best[i].date
			);
		} else {
			Q_snprintf(entry, sizeof(entry), "yv %d string \"%2d \" ",
				i * 10 + 16,
				i + 1);
		}

		j = strlen(entry);
		Q_strlcpy(string + stringlength, entry, sizeof(string) - stringlength);
		stringlength += j;
	}
	

	Q_snprintf(entry, sizeof(entry),
		"yv %d string \"    %d players completed map %d times\" ",
		i * 10 + 24,
		jump.map.num_records,
		jump.map.num_completions_total);

	j = strlen(entry);
	Q_strlcpy(string + stringlength, entry, sizeof(string) - stringlength);
	stringlength += j;

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast(ent, true);
}
