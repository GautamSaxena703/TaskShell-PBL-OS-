#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char command[50];
    char description[200];
} ShellCommand;

typedef struct {
    char processName[MAX_PATH];
    DWORD pid;
    SIZE_T ramUsageKB;
} ProcessInfo;

// Predefined list of commands
ShellCommand commands[] = {
    {"show_processes", "Shows foreground and background processes"},
    {"search_process", "Search for a specific process by name"},
    {"help", "Display all available commands"},
    {"exit", "Exit the shell"}
};

// Get process list
int GetProcesses(ProcessInfo processes[]) {
    DWORD processIDs[1024], cbNeeded, numProcesses;
    int processCount = 0;

    if (!EnumProcesses(processIDs, sizeof(processIDs), &cbNeeded)) {
        printf("Failed to enumerate processes.\n");
        return 0;
    }

    numProcesses = cbNeeded / sizeof(DWORD);

    for (DWORD i = 0; i < numProcesses; i++) {
        DWORD pid = processIDs[i];
        if (pid == 0) continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProcess) continue;

        HMODULE hMod;
        DWORD cbNeededMod;
        char processName[MAX_PATH] = "<unknown>";
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
            GetModuleBaseNameA(hProcess, hMod, processName, sizeof(processName) / sizeof(char));
        }

        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            strncpy(processes[processCount].processName, processName, MAX_PATH - 1);
            processes[processCount].processName[MAX_PATH - 1] = '\0';
            processes[processCount].pid = pid;
            processes[processCount].ramUsageKB = pmc.WorkingSetSize / 1024;
            processCount++;
        }

        CloseHandle(hProcess);
    }

    return processCount;
}

// Unified function: Show foreground and background processes
void ShowProcesses() {
    ProcessInfo processes[1024];
    int count = GetProcesses(processes);

    HWND hwnd = GetForegroundWindow();
    DWORD fgPid;
    GetWindowThreadProcessId(hwnd, &fgPid);

    char windowTitle[256] = "<No Title>";
    if (hwnd) {
        GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
    }

    printf("\n==== Foreground Process ====\n");
    int foundForeground = 0;
    for (int i = 0; i < count; i++) {
        if (processes[i].pid == fgPid) {
            printf("Process Name : %s\n", processes[i].processName);
            printf("Window Title : %s\n", windowTitle);
            printf("PID          : %lu\n", processes[i].pid);
            printf("RAM (KB)     : %lu\n", processes[i].ramUsageKB);
            foundForeground = 1;
            break;
        }
    }
    if (!foundForeground) {
        printf("No foreground process found.\n");
    }

    printf("\n==== Background Processes ====\n");
    printf("%-30s %-10s %-10s\n", "Process Name", "PID", "RAM (KB)");
    printf("-------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        if (processes[i].pid != fgPid) {
            printf("%-30s %-10u %-10lu\n",
                   processes[i].processName,
                   processes[i].pid,
                   processes[i].ramUsageKB);
        }
    }
}

// Search for a process by name
void SearchProcessByName() {
    ProcessInfo processes[1024];
    int count = GetProcesses(processes);

    char name[MAX_PATH];
    printf("Enter process name to search: ");
    if (scanf("%99s", name) == 1) {
        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strstr(processes[i].processName, name) != NULL) {
                printf("Found: %s (PID: %lu, RAM: %lu KB)\n",
                       processes[i].processName, processes[i].pid, processes[i].ramUsageKB);
                found = 1;
            }
        }

        if (!found) {
            printf("No process found with name containing '%s'.\n", name);
        }
        while (getchar() != '\n'); // Clear input buffer
    } else {
        printf("Invalid input.\n");
        while (getchar() != '\n'); // Clear input buffer
    }
}

// Show all available commands
void ShowHelp() {
    int num = sizeof(commands) / sizeof(commands[0]);
    printf("\nAvailable Commands:\n");
    for (int i = 0; i < num; i++) {
        printf("  %-20s - %s\n", commands[i].command, commands[i].description);
    }
}

// Handle user command input
void HandleCommand() {
    char input[100];

    while (1) {
        printf("\nTaskShell > ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nExiting TaskShell...\n");
            break;
        }
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "show_processes") == 0) {
            ShowProcesses();
        } else if (strcmp(input, "search_process") == 0) {
            SearchProcessByName();
        } else if (strcmp(input, "help") == 0) {
            ShowHelp();
        } else if (strcmp(input, "exit") == 0) {
            printf("Exiting TaskShell...\n");
            break;
        } else if (strcmp(input, "") != 0) {
            printf("Invalid command. Type 'help' to see available commands.\n");
        }
    }
}

int main() {
    printf("Welcome to TaskShell - Foreground & Background Process Viewer\n");
    ShowHelp();
    HandleCommand();
    return 0;
}
