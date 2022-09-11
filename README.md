# PipeLines

## The traditional connect the pipes game

Project developed to college focused on the following learnings: 

- Windows Registry (Create keys, Query keys, Delete keys)
- Synchronization mechanisms on Win32 API (mutex, threads, semaphores)
- Named Pipes (server/client multithreaded with overlapped I/O)
- Win32 Desktop Application

## Description

This implementantion features the following:
- Server responsible for game management
- Monitor responsible for updates during the game
- Client responsible for showing the user the current game and making it playable

The game has a grid that dimensions can be defined on Server initialization as arguments. The water moves at the speed set at Server initialization also
from arguments.<br>
This settings are stored on the Windows Registry under `HKEY_CURRENT_USER/SOFTWARE/temp/SO2/TP`.<br>
This settings can be retrieved after they are saved.

## Run

```Servidor water_time map_lin map_col```<br>
```Monitor```<br>
```Client```<br>

### Required
- Microsoft Visual C++ version 5.0
