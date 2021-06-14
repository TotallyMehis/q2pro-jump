Q2PRO
=====

Q2PRO is an enhanced, multiplayer oriented Quake 2 client and server.

Features include:

* rewritten OpenGL renderer optimized for stable FPS
* enhanced client console with persistent history
* ZIP packfiles (.pkz), JPEG and PNG textures, MD3 models
* fast HTTP downloads
* multichannel sound using OpenAL
* recording from demos, forward and backward seeking
* server side multiview demos and GTV capabilities

For building Q2PRO, consult the INSTALL file.

For information on using and configuring Q2PRO, refer to client and server
manuals available in doc/ subdirectory.

## Q2PRO Jump

Q2PRO Jump is a fork of Q2PRO, which aims to improve Q2Jump mod. It includes some of Q2PRO Speed's functionality made by [kugelrund](https://github.com/kugelrund/).

### Building

Follow the guide in the INSTALL file. Use `config_jump_win32` as the make config instead.

### New macros

#### Units per second

- Can now use 'dynamic' as a color. Green = accelerating, red = decelerating. Eg. `draw cl_ups x y dynamic`

```
cl_ups - Units per second. Does not take into account Z-axis.
cl_rups - Real units per second. Takes into account Z-axis.
```

#### Framerates

```
cl_fps - True cl_maxfps value
r_fps - True r_maxfps value

cl_mps - Same as cl_fps (moves per second)

cl_mfps - Measured frames per second (Same as q2pro's cl_fps)
r_mfps - Measured frames per second (Same as q2pro's r_fps)
```

#### Miscellaneous

```
cl_playerpos_z - Prints your origin Z-coordinate (not view origin).
cl_texture - Prints the texture you are looking at.
```