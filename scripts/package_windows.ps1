param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.7",
    [string]$Configuration = "Development",
    [string]$ArchiveDirectory = ""
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$ProjectFile = Join-Path $ProjectRoot "IpcCharacterWorld.uproject"
$RunUat = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"

if (-not (Test-Path $ProjectFile)) {
    throw "Project file not found: $ProjectFile"
}

if (-not (Test-Path $RunUat)) {
    throw "Unreal RunUAT.bat not found: $RunUat. Pass -EngineRoot if UE is installed elsewhere."
}

if ([string]::IsNullOrWhiteSpace($ArchiveDirectory)) {
    $ArchiveDirectory = Join-Path $ProjectRoot "Game\Windows"
}

$VsDevCmdCandidates = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat"
)

$VsDevCmd = $VsDevCmdCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $VsDevCmd) {
    throw "VsDevCmd.bat was not found. Install Visual Studio 2022 or Build Tools with C++ support."
}

$VsRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $VsDevCmd))
$MsvcToolsRoot = Join-Path $VsRoot "VC\Tools\MSVC"
$MsvcVersionDir = Get-ChildItem $MsvcToolsRoot -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending | Select-Object -First 1

$ExtraLibs = @()
if ($MsvcVersionDir) {
    $DesktopLib = Join-Path $MsvcVersionDir.FullName "lib\x64"
    $OneCoreLib = Join-Path $MsvcVersionDir.FullName "lib\onecore\x64"

    $DesktopHasRuntime = (Test-Path (Join-Path $DesktopLib "vcruntime.lib")) -and (Test-Path (Join-Path $DesktopLib "libcpmt.lib")) -and (Test-Path (Join-Path $DesktopLib "delayimp.lib"))
    if (-not $DesktopHasRuntime -and (Test-Path $OneCoreLib)) {
        $ExtraLibs += $OneCoreLib
        Write-Host "Using fallback MSVC OneCore libraries: $OneCoreLib"
    }
}

$ScopeLib = Join-Path $VsRoot "SDK\ScopeCppSDK\vc15\VC\lib"
if (Test-Path $ScopeLib) {
    $ExtraLibs += $ScopeLib
    Write-Host "Using fallback ScopeCppSDK libraries: $ScopeLib"
}

$LibAppend = ($ExtraLibs | Select-Object -Unique) -join ";"
$Command = "call `"$VsDevCmd`" -arch=x64"
if ($LibAppend.Length -gt 0) {
    $Command += " && set `"LIB=%LIB%;$LibAppend`""
}

$Command += " && `"$RunUat`" BuildCookRun -project=`"$ProjectFile`" -noP4 -platform=Win64 -clientconfig=$Configuration -build -cook -stage -pak -archive -archivedirectory=`"$ArchiveDirectory`""

Write-Host "Packaging Windows build to: $ArchiveDirectory"
cmd.exe /d /c $Command
if ($LASTEXITCODE -ne 0) {
    throw "Unreal packaging failed with exit code $LASTEXITCODE"
}
