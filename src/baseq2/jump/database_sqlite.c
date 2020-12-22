#include "g_local.h"
#include "threading.h"

#include <sqlite/sqlite3.h>

#include "util.h"
#include "main.h"
#include "database.h"


/*
===============
database_sqlite.c

The jump mod's database in SQLite.
Easy to use and serverless.

JUMP TODO: Do the ranking system
===============
*/


static sqlite3 *db = NULL;


/*
===============
Jump_Database_Init
===============
*/
void Jump_Database_Init(void)
{
    char    *errmsg;
    int     rc;

    //
    // open a database (create if not found) in the jump directory.
    // uses serialized threading mode so we don't have to worry about mutexes.
    //
    rc = sqlite3_open_v2(
        va("%s/jump_database.sqlite3", JUMP_GAMEDIR),
        &db,
        SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX,
        NULL);

    if (rc != SQLITE_OK) {
        gi.error("Failed to open database! SQLite3 error (%i): %s\n", rc, sqlite3_errmsg(db));
        return;
    }

    const char schema[] =
        "CREATE TABLE IF NOT EXISTS jump_maps ("
        "	map_id INTEGER PRIMARY KEY,"
        "	map_name VARCHAR(128) NOT NULL UNIQUE,"
        "	added_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE TABLE IF NOT EXISTS jump_users ("
        "	ply_id INTEGER PRIMARY KEY,"
        "	ply_name VARCHAR(128) NOT NULL UNIQUE,"
        "	join_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "	last_played_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE TABLE IF NOT EXISTS jump_times ("
        "	time_id INTEGER PRIMARY KEY,"
        "	map_id INTEGER NOT NULL,"
        "	ply_id INTEGER NOT NULL,"
        "	run_time REAL NOT NULL,"
        "	run_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "	FOREIGN KEY(map_id) REFERENCES jump_maps(map_id),"
        "	FOREIGN KEY(ply_id) REFERENCES jump_users(ply_id)"
        ");"
        "CREATE TABLE IF NOT EXISTS jump_times_cache ("
        "	map_id INTEGER NOT NULL,"
        "	ply_id INTEGER NOT NULL,"
        "	best_time_id INTEGER NOT NULL,"
        "	completions INTEGER NOT NULL DEFAULT 1,"
        "	PRIMARY KEY(map_id, ply_id),"
        "	FOREIGN KEY(map_id) REFERENCES jump_maps(map_id),"
        "	FOREIGN KEY(ply_id) REFERENCES jump_users(ply_id),"
        "	FOREIGN KEY(best_time_id) REFERENCES jump_times(time_id)"
        ");";


    if (sqlite3_exec(db, schema, NULL, NULL, &errmsg) != SQLITE_OK) {
        gi.error("Failed creating database tables! SQLite3 error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return;
    }
}

/*
===============
Jump_Database_Shutdown
===============
*/
void Jump_Database_Shutdown(void)
{
    sqlite3_close(db);
    db = NULL;
}


/*
===============
FastQuerySingleValue

Returns an int. 0 on error.
===============
*/
static int FastQuerySingleValue(const char *query)
{
    sqlite3_stmt    *stmt;
    int             rc;
    int             val;

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        gi.dprintf("Failed to do a fast query for a single value! SQLite3 error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        gi.dprintf("Failed to do a fast query for a single value! SQLite3 error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    val = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return val;
}

/*
===============
FastQuerySingleValueDouble

Returns 0 on error.
===============
*/
static double FastQuerySingleValueDouble(const char *query)
{
    sqlite3_stmt    *stmt;
    int             rc;
    double          val;

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        gi.dprintf("Failed to do a fast query for a single value! SQLite3 error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        gi.dprintf("Failed to do a fast query for a single value! SQLite3 error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    val = sqlite3_column_double(stmt, 0);

    sqlite3_finalize(stmt);

    return val;
}


/*
===============
Fetch Map Data
===============
*/
static void *FetchMapData(const char *mapname)
{
    int                 rc;
    sqlite3_stmt        *stmt;
    dbcb_map_t          *out_data = (dbcb_map_t*)gi.TagMalloc(sizeof(dbcb_map_t), TAG_GAME);
    const char          *query;
    int                 i;
    
    memset(out_data, 0, sizeof(dbcb_map_t));
    out_data->map_id = INVALID_MAP_ID;

    

    rc = sqlite3_prepare_v2(db, "SELECT map_id FROM jump_maps WHERE map_name=?", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        gi.error("Failed to prepare a statement to fetch map data! SQLite3 error: %s\n", sqlite3_errmsg(db));
        return out_data;
    }

    rc = sqlite3_bind_text(stmt, 1, mapname, -1, NULL);
    if (rc != SQLITE_OK) {
        gi.error("Failed to bind map name to fetch map data! SQLite3 error: %s\n", sqlite3_errmsg(db));
        return out_data;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        //
        // found something
        //

        out_data->map_id = sqlite3_column_int64(stmt, 0);

        sqlite3_finalize(stmt);


        out_data->num_records = FastQuerySingleValue(va("SELECT COUNT(*) FROM jump_times_cache WHERE map_id=%lld", out_data->map_id));
        out_data->num_completions_total = FastQuerySingleValue(va("SELECT COUNT(*) FROM jump_times WHERE map_id=%lld", out_data->map_id));

        //
        // query best times
        //
        query = va(
            "SELECT"
            "	_t2.ply_id,_t2.run_time,_u.ply_name,strftime('%s', _t2.run_date) "
            "FROM jump_times_cache AS _t "
            "INNER JOIN jump_users AS _u ON _t.ply_id=_u.ply_id "
            "INNER JOIN jump_times AS _t2 ON _t.best_time_id=_t2.time_id "
            "WHERE _t.map_id=%lld "
            "ORDER BY _t2.run_time ASC LIMIT %i;",
            DATE_FORMAT,
            out_data->map_id,
            ARRAY_SIZE(out_data->best_times)
        );

        rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            gi.error("Failed to prepare a statement to fetch map data! SQLite3 error: %s\n", sqlite3_errmsg(db));
            return out_data;
        }


        i = 0;
        const int arraysize = ARRAY_SIZE(out_data->best_times);

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && i < arraysize) {
            out_data->best_times_ids[i] = sqlite3_column_int64(stmt, 0);
            out_data->best_times[i] = sqlite3_column_double(stmt, 1);
            Q_strlcpy(out_data->best_times_name[i], sqlite3_column_text(stmt, 2), sizeof(out_data->best_times_name[i]));
            Q_strlcpy(out_data->best_times_date[i], sqlite3_column_text(stmt, 3), sizeof(out_data->best_times_date[i]));

            ++i;
        }

        out_data->num_best_times = i;
        
        
        if (rc != SQLITE_OK && rc != SQLITE_DONE) {
            gi.error("Failed to query map best times! SQLite3 error: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);
    } else if (rc == SQLITE_DONE) {
        //
        // nothing
        // insert new map into the database.
        //
        sqlite3_finalize(stmt);


        rc = sqlite3_prepare_v2(db, "INSERT INTO jump_maps (map_name) VALUES (?)", -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            gi.error("Failed to prepare a statement to insert a new map into database! SQLite3 error: %s\n", sqlite3_errmsg(db));
            return out_data;
        }

        rc = sqlite3_bind_text(stmt, 1, mapname, -1, NULL);
        if (rc != SQLITE_OK) {
            gi.error("Failed to bind map name to insert a new map! SQLite3 error: %s\n", sqlite3_errmsg(db));
            return out_data;
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_OK && rc != SQLITE_DONE) {
            gi.error("Failed to insert new map! SQLite3 error: %s\n", sqlite3_errmsg(db));
            return out_data;
        }

        out_data->map_id = sqlite3_last_insert_rowid(db);
        out_data->is_new_map = true;

        sqlite3_finalize(stmt);

    } else {
        //
        // error
        //
        sqlite3_finalize(stmt);

        gi.error("Failed retrieving map data! SQLite3 error: %s\n", sqlite3_errmsg(db));

        return out_data;
    }

    out_data->num_maps_total = FastQuerySingleValue("SELECT COUNT(*) FROM jump_maps");

    return out_data;
}

void Jump_Database_FetchMapData(const char *mapname, void (*cb)(dbcb_map_t *map_data))
{
    gi.dprintf("Fetching map %s's data.\n", mapname);

    // fetching map data should be completely synchronous.
    void* data = FetchMapData(mapname);
    cb(data);
    gi.TagFree(data);
}

/*
===============
Fetch Player Data
===============
*/
static void *FetchPlayerData(const char *name)
{
    sqlite3_stmt        *stmt;
    int                 rc;
    dbcb_user_t         *out_data = (dbcb_user_t*)gi.TagMalloc(sizeof(dbcb_user_t), TAG_GAME);
    
    memset(out_data, 0, sizeof(dbcb_user_t));
    out_data->player_id = INVALID_PLAYER_ID;
    out_data->map_id = jump.map.map_id;
    Q_strlcpy(out_data->user_name, name, sizeof(out_data->user_name));

    

    rc = sqlite3_prepare_v2(db, "SELECT ply_id FROM jump_users WHERE ply_name=?", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        gi.dprintf("Failed to prepare a statement to fetch player data! SQLite3 error: %s\n", sqlite3_errmsg(db));
        return out_data;
    }

    rc = sqlite3_bind_text(stmt, 1, name, -1, NULL);
    if (rc != SQLITE_OK) {
        gi.dprintf("Failed to bind player name to fetch player data! SQLite3 error: %s\n", sqlite3_errmsg(db));
        return out_data;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        //
        // found something
        //

        out_data->player_id = sqlite3_column_int64(stmt, 0);

        sqlite3_finalize(stmt);

        out_data->my_time = FastQuerySingleValueDouble(va("SELECT run_time FROM jump_times_cache AS _tc INNER JOIN jump_times AS _t ON _tc.best_time_id=_t.time_id WHERE _tc.ply_id=%lld", out_data->player_id));
        out_data->completions = FastQuerySingleValue(va("SELECT COUNT(*) FROM jump_times WHERE map_id=%lld AND ply_id=%lld", jump.map.map_id, out_data->player_id));
        out_data->num_maps_beaten = FastQuerySingleValue(va("SELECT COUNT(*) FROM jump_times_cache WHERE ply_id=%lld", out_data->player_id));
        out_data->rank = 0; // JUMP TODO: Actually do this shit.
    } else if (rc == SQLITE_DONE) {
        //
        // nothing
        // insert new player into the database
        //
        sqlite3_finalize(stmt);


        rc = sqlite3_prepare_v2(db, "INSERT INTO jump_users (ply_name) VALUES (?)", -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            gi.dprintf("Failed to prepare a statement to insert a new user into database! SQLite3 error: %s\n", sqlite3_errmsg(db));
            return out_data;
        }

        rc = sqlite3_bind_text(stmt, 1, name, -1, NULL);
        if (rc != SQLITE_OK) {
            gi.dprintf("Failed to bind player name to insert new player data! SQLite3 error: %s\n", sqlite3_errmsg(db));
            return out_data;
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_OK && rc != SQLITE_DONE) {
            gi.dprintf("Failed to insert new player data! SQLite3 error: %s\n", sqlite3_errmsg(db));
            return out_data;
        }

        out_data->player_id = sqlite3_last_insert_rowid(db);

        sqlite3_finalize(stmt);
    } else {
        //
        // error
        //
        sqlite3_finalize(stmt);

        gi.dprintf("Failed retrieving player data! SQLite3 error: %s\n", sqlite3_errmsg(db));
    }

    return out_data;
}

void Jump_Database_FetchPlayerData(const char *player_name, void (*cb)(dbcb_user_t *user_data))
{
    if (!player_name || !player_name[0])
        return;

    gi.dprintf("Fetching player %s's data.\n", player_name);

    size_t len = strlen(player_name) + 1;
    char* data = (char*)gi.TagMalloc(len, TAG_GAME);
    Q_strlcpy(data, player_name, len);
    
    Jump_Thread_Create(FetchPlayerData, cb, (void*)data, true);
}



/*
===============
Store Time
===============
*/
static void *StoreTime(dbstore_time_t *data)
{
    const char          *query;
    sqlite3_int64       time_id;
    char                *errmsg;

    // nothing else to update except completions.
    if (!data->is_new_rec && !data->new_pb) {
        query = va(
            "UPDATE jump_times_cache SET completions=completions+1 WHERE map_id=%lld AND ply_id=%lld",
            data->map_id,
            data->player_id);

    
        if (sqlite3_exec(db, query, NULL, NULL, &errmsg) != SQLITE_OK) {
            gi.dprintf("Failed updating completions in time cache! SQLite3 error: %s\n", errmsg);
            sqlite3_free(errmsg);
            return NULL;
        }

        return NULL;
    }

    // new record or new personal best.
    // insert a new record into database.
    query = va(
        "INSERT INTO jump_times (map_id, ply_id, run_time) VALUES (%lld, %lld, %f)",
        data->map_id,
        data->player_id,
        data->time);


    // JUMP TODO: Mutex here to make sure last insert row id is correct?

    if (sqlite3_exec(db, query, NULL, NULL, &errmsg) != SQLITE_OK) {
        gi.dprintf("Failed inserting time! SQLite3 error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return NULL;
    }


    time_id = sqlite3_last_insert_rowid(db);





    // completely new record. insert new cache
    if (data->is_new_rec) {
        query = va(
            "INSERT INTO jump_times_cache (map_id, ply_id, best_time_id) VALUES (%lld, %lld, %lld)",
            data->map_id,
            data->player_id,
            time_id);

    
        if (sqlite3_exec(db, query, NULL, NULL, &errmsg) != SQLITE_OK) {
            gi.dprintf("Failed inserting time cache! SQLite3 error: %s\n", errmsg);
            sqlite3_free(errmsg);
            return NULL;
        }
    } else {
        // new pb, update everything.
        query = va(
            "UPDATE jump_times_cache SET best_time_id=%lld, completions=completions+1 WHERE map_id=%lld AND ply_id=%lld",
            data->time,
            time_id,
            data->map_id,
            data->player_id);

    
        if (sqlite3_exec(db, query, NULL, NULL, &errmsg) != SQLITE_OK) {
            gi.dprintf("Failed updating time cache! SQLite3 error: %s\n", errmsg);
            sqlite3_free(errmsg);
            return NULL;
        }
    }

    return NULL;
}

void Jump_Database_StoreTime(dbstore_time_t* data)
{
    gi.dprintf("Saving player's time (id: %lld) of %.4f\n",
        data->player_id,
        data->time);
    
    Jump_Thread_Create(StoreTime, data->cb, data, true);
}

/*
===============
Fetch rank list (for printing)
===============
*/
static void *FetchRankList(dbload_ranklist_t *in_data)
{
    char                *errmsg;
    const char          *query;
    int                 rc;
    sqlite3_stmt        *stmt;
    dbcb_ranklist_t     *out_data = (dbcb_ranklist_t*)gi.TagMalloc(sizeof(dbcb_ranklist_t), TAG_GAME);
    int                 i;

    memset(out_data, 0, sizeof(dbcb_ranklist_t));
    out_data->player_num = in_data->player_num;
    out_data->count = in_data->count;
    out_data->offset = in_data->offset;


    query = va(
        "SELECT ply_name,"
        "	(SELECT COUNT(*) FROM jump_times_cache WHERE ply_id=_u.ply_id AND ply_cur_pos=1) AS p1,"
        "	(SELECT COUNT(*) FROM jump_times_cache WHERE ply_id=_u.ply_id AND ply_cur_pos=2) AS p2,"
        "	(SELECT COUNT(*) FROM jump_times_cache WHERE ply_id=_u.ply_id AND ply_cur_pos=3) AS p3,"
        "	(SELECT COUNT(*) FROM jump_times_cache WHERE ply_id=_u.ply_id AND ply_cur_pos=4) AS p4,"
        "	(SELECT COUNT(*) FROM jump_times_cache WHERE ply_id=_u.ply_id AND ply_cur_pos=5) AS p5,"
        "	(SELECT SUM(points) FROM "
        "		(SELECT "
        "			CASE "
        "				WHEN ply_cur_pos>=5 THEN (-ply_cur_pos + 16) "
        "				ELSE (0.5*(ply_cur_pos*ply_cur_pos)-6.5*ply_cur_pos+31) "
        "			END AS points "
        "		FROM jump_times_cache WHERE ply_id=_u.ply_id)"
        "	) AS total_points "
        "FROM jump_users AS _u "
        "ORDER BY total_points DESC LIMIT %d OFFSET %d;",
        in_data->count,
        in_data->offset
    );

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        gi.dprintf("Failed to prepare a statement to fetch rank list data! SQLite3 error: %s\n", sqlite3_errmsg(db));
        return out_data;
    }


    i = 0;
    const int arraysize = ARRAY_SIZE(out_data->num_1);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && i < arraysize) {
        Q_strlcpy(out_data->player_name[i], sqlite3_column_text(stmt, 0), sizeof(out_data->player_name[i]));
        out_data->num_1[i] = sqlite3_column_int(stmt, 1);
        out_data->num_2[i] = sqlite3_column_int(stmt, 2);
        out_data->num_3[i] = sqlite3_column_int(stmt, 3);
        out_data->num_4[i] = sqlite3_column_int(stmt, 4);
        out_data->num_5[i] = sqlite3_column_int(stmt, 5);
        out_data->total_points[i] = sqlite3_column_int(stmt, 6);
        
        ++i;
    }

    out_data->num_retrieved = i;
        
        
    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        gi.dprintf("Failed to query rank list! SQLite3 error: %s\n", sqlite3_errmsg(db));
    }

    return out_data;
}

void Jump_Database_FetchRankList(dbload_ranklist_t *data, void (*cb)(dbcb_ranklist_t *ranklist))
{
    Jump_Thread_Create(FetchRankList, cb, data, true);
}
