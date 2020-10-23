# gmsv_rcon2
Garry's Mod binary module for controlling RCON authentication directly from Lua!

## Supported hooks

#### <a name="on_rcon_write_request"></a> `OnRconWriteRequest (listenerId, requestId, requestType, data, isLanIpAddress, ip, port)`

Main rcon hook **OnRconWriteRequest**, according engine function is - `CServerRemoteAccess::WriteDataRequest`.

On both platforms `::WriteDataRequest` checks for password first time and then process RCON requests. But here's a little difference between platforms. In Linux build each request bein logged with external call of `CServerRemoteAccess::LogCommand` (which is a hook **OnRconLogCommand**), but on Windows this function are force-inlined. Than means, you won't be not able to use `OnRconLogCommand` for Windows game server. Although `::LogCommand` function exists on both platforms, it is not being called on Windows.

#### <a name="on_rcon_check_password"></a> `OnRconCheckPassword (password, listenerId, isLanIpAddress, ip, port)`

Hook for engine function `CServerRemoteAccess::IsPassword`, which is being called in `CServerRemoteAccess::WriteDataRequest` to verity that incoming user password matches server (originally rcon_password console variable). You may not wanted to use **OnRconWriteRequest** but wish to override default password, or make it dynamic over time - so this is an option.

#### <a name="on_rcon_log_command"></a> `OnRconLogCommand (listenerId, msg, isKnownListener, isLanIpAddress, ip, port)`

Hook for engine function `CServerRemoteAccess::LogCommand`, originally it print out messages like `rcon from \"%s\": %s\n`. You can override printing on Linux platform using this hook.
Does not work under Windows, see [OnRconWriteRequest](#on_rcon_write_request).

# Building module
Requirements for manual build:
- SSDK2013
- CMake 3.8+
- GCC (g++)

Build variables:
- **GMSV_SSDK_PATH** - Path to [Valve Source SDK 2013](https://github.com/ValveSoftware/source-sdk-2013)
- **GMSV_PLATFORM** - [Optional] Set build platform (only LINUX or WINDOWS values allowed), by default platform is deteching automatically.
- **GMSV_TARGET** - [Optional] Type of target build, can be Debug or Release (default).

## Linux
First, go to root folder of gmsv_rcon2 files and configure build with follow commands (example):
- `cmake -D GMSV_SSDK_PATH=/usr/lib/ssdk-2013 .`
- `make`

## Windows
You can use Visual Studio 2019 (my version is 16.7.6) to open gmsv_rcon2 root folder, then select `x86-debug-win` configuration and press `Ctrl+Shit+B`.
Another option is to open **x64_x86 Cross Tools Command Prompt for VS 2019** and run following commands (example):
- `cmake -D GMSV_SSDK_PATH=B:\libs\ssdk2013 -A Win32 .`
- `cmake --build .`

# Updates & Issues
Module uses specific patterns for signature scanning and should work after game updates, but still better to test it before updating server.
In case of problems with newer versions or other things, feel free to open an issue.