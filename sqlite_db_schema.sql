/*
======================================
SQLite database schema for jump mod.
Not compatible with MySQL.
======================================
*/


/*
Maps referenced in the database.
Primary key: 'map_id'
*/
CREATE TABLE IF NOT EXISTS jump_maps (
	map_id INTEGER PRIMARY KEY,
	map_name VARCHAR(128) NOT NULL UNIQUE,
	added_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
	last_played_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

/*
This is where players are.
Name needs to be unique.
Primary key: 'ply_id'
*/
CREATE TABLE IF NOT EXISTS jump_users (
	ply_id INTEGER PRIMARY KEY,
	ply_name VARCHAR(128) NOT NULL UNIQUE,
	join_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
	last_played_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

/*
All the times player has beaten the map. See 'jump_times_cache' for more.
Primary key: 'time_id'.
*/
CREATE TABLE IF NOT EXISTS jump_times (
	time_id INTEGER PRIMARY KEY,
	map_id INTEGER NOT NULL,
	ply_id INTEGER NOT NULL,
	run_time REAL NOT NULL,
	run_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
	FOREIGN KEY(map_id) REFERENCES jump_maps(map_id),
	FOREIGN KEY(ply_id) REFERENCES jump_users(ply_id)
);

/*
Used to easily query for best times.
There can only be a single record per map and player.
Primary key: 'map_id' and 'ply_id'
*/
CREATE TABLE IF NOT EXISTS jump_times_cache (
	map_id INTEGER NOT NULL,
	ply_id INTEGER NOT NULL,
	best_time_id INTEGER NOT NULL,
	completions INTEGER NOT NULL DEFAULT 1,
	PRIMARY KEY(map_id, ply_id),
	FOREIGN KEY(map_id) REFERENCES jump_maps(map_id),
	FOREIGN KEY(ply_id) REFERENCES jump_users(ply_id),
	FOREIGN KEY(best_time_id) REFERENCES jump_times(time_id)
);
