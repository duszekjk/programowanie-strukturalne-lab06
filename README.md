# IpcCharacterWorld

This repository prepares the Unreal Engine part of the IPC lab:

- a C++ Unreal project skeleton,
- a FIFO-based command receiver component,
- small C examples for students,
- a helper header for sending commands,
- a simple launch script for the packaged Linux build,
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
- Visual Studio 2022 or Build Tools on Windows when building the Unreal editor module,
- a Linux toolchain with `gcc` or `clang` for the student C examples,
- the project created as a C++ Third Person template.

The Unreal package itself is not bundled here. This repository contains the source side and the student-side IPC examples.

## Windows: build the Unreal editor module

From the repository root, run:

```powershell
Set-ExecutionPolicy -Scope Process Bypass -Force
.\scripts\build_editor_windows.ps1
```

The script locates `IpcCharacterWorld.uproject`, runs Visual Studio's `VsDevCmd.bat`, and then calls Unreal's `Build.bat` for `IpcCharacterWorldEditor`.

Some lab machines have an incomplete Visual Studio layout where the normal MSVC desktop library folder `VC\Tools\MSVC\...\lib\x64` is missing `delayimp.lib`, `vcruntime.lib`, or `libcpmt.lib`, while the libraries exist under `lib\onecore\x64` and `SDK\ScopeCppSDK\vc15\VC\lib`. In that case the script adds those fallback library folders to `LIB` for the current build process only. It does not modify the project files or the system Visual Studio installation.

If Unreal is installed somewhere else, pass the engine path explicitly:

```powershell
.\scripts\build_editor_windows.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.7"
```

## Project layout

- `Source/IpcCharacterWorld/` - Unreal C++ module, character, game mode, command receiver
- `Config/DefaultEngine.ini` - default map and game mode
- `Game/IpcCharacterWorld.sh` - launch wrapper for a packaged Linux build
- `include/ue_ipc.h` - helper function for examples
- `examples/` - small student programs
- `Config/SubmissionConfig.json` - optional submission metadata for the server
- `UNREAL_IPC_STUDENT_GUIDE.md` - detailed instructions for students
- `Makefile` - builds the examples
- `scripts/build_editor_windows.ps1` - Windows helper for building the Unreal editor module

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
- On Windows lab machines, use `scripts/build_editor_windows.ps1` instead of calling Unreal `Build.bat` directly.
