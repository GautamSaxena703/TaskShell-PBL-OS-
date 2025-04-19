#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char command[50];
    char description[200];
} ShellCommand;

ShellCommand commands[] = {
    {"list_processes", "Lists all running processes (foreground & background)"},
    {"list_fg_process", "Displays the currently active (foreground) process"},
    {"list_bg_processes", "Displays all background processes"},
    {"help", "Display all available commands"}
};

void listProcesses() {
    DWORD processes[1024], needed, count;
    if (!EnumProcesses(processes, sizeof(processes), &needed)) {
        printf("Failed to list processes.\n");
        return;
    }

    count = needed / sizeof(DWORD);
    printf("\nAll Running Processes:\n");
    printf("%-30s  %-6s\n", "Process Name", "PID");

    for (DWORD i = 0; i < count; i++) {
        DWORD pid = processes[i];
        if (pid == 0) continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            HMODULE hMod;
            DWORD cbNeededMod;
            char name[MAX_PATH] = "<unknown>";

            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
                GetModuleBaseNameA(hProcess, hMod, name, sizeof(name) / sizeof(char));
                printf("%-30s  %-6u\n", name, pid);
            }

            CloseHandle(hProcess);
        }
    }
}

void listForegroundProcess() {
    HWND hwnd = GetForegroundWindow();
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess) {
        char name[MAX_PATH] = "<unknown>";
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
            GetModuleBaseNameA(hProcess, hMod, name, sizeof(name));
            printf("\nForeground Process:\n");
            printf("Process Name: %s\n", name);
            printf("PID         : %u\n", pid);
        }
        CloseHandle(hProcess);
    } else {
        printf("Could not get foreground process.\n");
    }
}

void listBackgroundProcesses() {
    DWORD processes[1024], needed, count;
    DWORD fgPid = 0;
    HWND hwnd = GetForegroundWindow();
    GetWindowThreadProcessId(hwnd, &fgPid);

    if (!EnumProcesses(processes, sizeof(processes), &needed)) {
        printf("Failed to list processes.\n");
        return;
    }

    count = needed / sizeof(DWORD);
    printf("\nBackground Processes:\n");
    printf("%-30s  %-6s\n", "Process Name", "PID");

    for (DWORD i = 0; i < count; i++) {
        DWORD pid = processes[i];
        if (pid == 0 || pid == fgPid) continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            HMODULE hMod;
            DWORD cbNeededMod;
            char name[MAX_PATH] = "<unknown>";

            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
                GetModuleBaseNameA(hProcess, hMod, name, sizeof(name));
                printf("%-30s  %-6u\n", name, pid);
            }

            CloseHandle(hProcess);
        }
    }
}

void showHelp() {
    printf("\nAvailable Commands:\n");
    int count = sizeof(commands) / sizeof(commands[0]);
    for (int i = 0; i < count; i++) {
        printf(" - %-20s : %s\n", commands[i].command, commands[i].description);
    }
}

void handleCommand() {
    char input[100];
    int found = 0;
    printf("\nEnter a command: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';

    int numCommands = sizeof(commands) / sizeof(commands[0]);
    for (int i = 0; i < numCommands; i++) {
        if (strcmp(input, commands[i].command) == 0) {
            found = 1;
            printf("Command: %s\n", commands[i].command);
            printf("Function: %s\n", commands[i].description);

            // Execute the function
            if (strcmp(input, "list_processes") == 0) {
                listProcesses();
            } else if (strcmp(input, "list_fg_process") == 0) {
                listForegroundProcess();
            } else if (strcmp(input, "list_bg_processes") == 0) {
                listBackgroundProcesses();
            } else if (strcmp(input, "help") == 0) {
                showHelp();
            }
            break;
        }
    }

    if (!found) {
        printf("Command does not exist.\n");
    }
}

int main() {
    while (1) {
        handleCommand();
        char again;
        printf("\nDo you want to enter another command? (y/n): ");
        scanf(" %c", &again);
        getchar(); // clear newline
        if (again != 'y' && again != 'Y') break;
    }
    return 0;
}
