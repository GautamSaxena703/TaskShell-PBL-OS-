#ifndef PROCESS_UTIL_H
#define PROCESS_UTIL_H

#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iphlpapi.h>
#include <tlhelp32.h>

#define MAX_PROCESSES 1024
#define INPUT_SIZE 100

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")

// Structs

typedef struct {
    char processName[MAX_PATH];
    DWORD pid;
    double cpuUsage; 
} ProcessCpuInfo;

typedef struct {
    char processName[MAX_PATH];
    DWORD pid;
    ULONGLONG bytesSent;  
    ULONGLONG bytesRecv;
} ProcessNetworkInfo;

typedef struct {
    char processName[MAX_PATH];
    DWORD pid;
    SIZE_T ramUsageKB; 
} ProcessRamInfo;

typedef struct {
    char processName[MAX_PATH];
    DWORD pid;
    double cpuTimeSeconds;
    SIZE_T ramUsageBytes;
    ULONGLONG diskReadBytes;
    ULONGLONG diskWriteBytes;
} ProcessUtilInfo;

// Function declarations

// CPU
void PrintCPUStatus();
int GetCpuProcesses(ProcessCpuInfo processes[]);
void PrintAllProcessCPUUsage();
void SearchCpuProcessByName();

// NETWORK
void PrintNetworkStatus();
int GetProcessesNetworkUsage(ProcessNetworkInfo processes[]);
void PrintAllProcessNetworkUsage();
void SearchNetworkProcessByName();

// RAM
void PrintMemoryStatus();
int compareRAMUsage(const void *a, const void *b);
int GetRamProcesses(ProcessRamInfo processes[]);
void PrintAllProcessRAMUsage();
void SearchRamProcessByName();

// UTILIZATION
void ProcessListing();
void PrintAllProcessUtilization();

// CLI Input Handler
// void HandleUserInput(const char* input);

#endif // PROCESS_UTIL_H
