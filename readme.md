# Process Utilization Monitoring App

This application displays real-time information about running processes on your Windows system, including CPU time, RAM usage, disk I/O, and more, using a GUI built with the Win32 API and ListView control.

## Features

- Lists all active processes with details such as:
  - Process Name
  - PID
  - CPU Time (seconds)
  - RAM Usage (MB)
  - Disk Read/Write (MB)
- Auto-refreshes every 0.5 seconds
- Simple and intuitive Windows GUI

## Requirements

- Windows OS
- GCC for Windows (MinGW or MSYS2)
- Windows SDK libraries: `iphlpapi`, `psapi`, `comctl32`

## Building and Running

1. Open a terminal (MSYS2 or MinGW shell recommended).

2. Compile the application using the following command:

   ```bash
   gcc main.c UI.c helper.c -liphlpapi -lpsapi -lcomctl32 -lkernel32 -luser32 -lgdi32 -o ProcessUtilApp.exe
   ```

3. Run the executable:

   ```bash
   ./ProcessUtilApp.exe
   ```

The GUI window will open and display the list of running processes with utilization data.
