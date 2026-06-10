# IpcCharacterWorld

This repository prepares the Unreal Engine part of the IPC lab:

- a C++ Unreal project skeleton,
- a FIFO-based command receiver component,
- small C examples for students,
- a helper header for sending commands,
- a simple launch script for the packaged Linux build.
- a student guide with build, run, and communication instructions.

## What students do

Students do not edit Unreal code. They:

1. run the packaged game,
2. compile one of the example C programs,
3. send commands into `/tmp/ue_character_commands`,
4. observe the character move in the game.

## External requirements

The project assumes:

- Unreal Engine 5.x installed locally,
- a Linux toolchain with `gcc` or `clang`,
- the project created as a C++ Third Person template.

The Unreal package itself is not bundled here. This repository contains the source side and the student-side IPC examples.

## Project layout

- `Source/IpcCharacterWorld/` - Unreal C++ module, character, game mode, command receiver
- `Config/DefaultEngine.ini` - default map and game mode
- `Game/IpcCharacterWorld.sh` - launch wrapper for a packaged Linux build
- `include/ue_ipc.h` - helper function for examples
- `examples/` - small student programs
- `Config/SubmissionConfig.json` - optional submission metadata for the server
- `UNREAL_IPC_STUDENT_GUIDE.md` - detailed instructions for students
- `Makefile` - builds the examples

## Build the examples

```bash
make
```

This creates binaries in `build/`.

## Run the lab

1. Package the Unreal project for Linux.
2. Start the game with `./Game/IpcCharacterWorld.sh`.
3. In another terminal, run one of the example programs or use:

```bash
echo FORWARD > /tmp/ue_character_commands
echo RIGHT > /tmp/ue_character_commands
echo FORWARD > /tmp/ue_character_commands
```

## Notes

- Commands are case-insensitive.
- Invalid commands are ignored and logged.
- `RESET` teleports the character back to the initial transform.
