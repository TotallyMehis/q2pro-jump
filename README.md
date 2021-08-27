# Q2PRO Jump

Q2PRO Jump is a fork of Q2PRO, which aims to improve Q2Jump mod. It includes some of Q2PRO Speed's functionality made by [kugelrund](https://github.com/kugelrund/).

## Features

- Strafe hud
- Decimal FPS values.
- New and modified macros.
- Q2Jump mod command autocompletion.

## New commands

```
cl_drawstrafehelper - Draws strafe hud. Use cl_strafehelper*-cvars for options.

cl_drawworldorigin - Draws world origin. Use cl_worldorigin*-cvars for options.
```

## New macros

### Units per second

- Can now use 'dynamic' as a color. Green = accelerating, red = decelerating. Eg. `draw cl_ups x y dynamic`

```
cl_ups - Units per second. Does not take into account Z-axis.
cl_rups - Real units per second. Takes into account Z-axis.
```

### Framerates

```
cl_fps - True cl_maxfps value
r_fps - True r_maxfps value

cl_mps - Same as cl_fps (moves per second)

cl_mfps - Measured frames per second (Same as q2pro's cl_fps)
r_mfps - Measured frames per second (Same as q2pro's r_fps)
```

### Miscellaneous

```
cl_playerpos_z - Prints your origin Z-coordinate (not view origin).
cl_texture - Prints the texture you are looking at.
```

## Building

Follow the guide in the INSTALL file. Use `config_jump_win32` as the make config instead.
