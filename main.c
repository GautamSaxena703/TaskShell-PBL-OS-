#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include "ui.h"

// ANSI color codes
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

void PrintBanner() {
    printf(GREEN "\n=========================================\n" RESET);
    printf(YELLOW "      SYSTEM MONITORING CLI TOOL         \n" RESET);
    printf(GREEN "=========================================\n\n" RESET);
}

void PrintHelp() {
    printf(BLUE "\nAvailable commands:\n" RESET);
    printf(YELLOW "  cpuStatus              " RESET "- Show overall CPU status\n");
    printf(YELLOW "  ramStatus              " RESET "- Show RAM usage status\n");
    printf(YELLOW "  networkStatus          " RESET "- Show network status\n");
    printf(YELLOW "  cpu                    " RESET "- CPU status + per-process usage\n");
    printf(YELLOW "  ram                    " RESET "- RAM status + per-process usage\n");
    printf(YELLOW "  network                " RESET "- Network + per-process usage\n");
    printf(YELLOW "  utilization            " RESET "- All process utilization\n");
    printf(YELLOW "  search_cpu             " RESET "- Search process CPU usage\n");
    printf(YELLOW "  search_ram             " RESET "- Search process RAM usage\n");
    printf(YELLOW "  search_network         " RESET "- Search process network usage\n");
    printf(YELLOW "  list                   " RESET "- List running processes\n");
    printf(YELLOW "  gui                    " RESET "- Launch GUI interface\n");
    printf(YELLOW "  show_process_diskusage_all   " RESET "- Show disk usage for all processes\n");
    printf(YELLOW "  show_process_diskusagebypid  " RESET "- Show disk usage by specific PID\n");
    printf(YELLOW "  analyze_folder_size    " RESET "- Analyze folder size\n");
    printf(YELLOW "  analyze_extension_distribution" RESET "- Extension distribution in folder\n");
    printf(YELLOW "  exit                   " RESET "- Exit the tool\n\n");
}

void HandleUserInput(const char* input) {
    printf(CYAN "\n[+] Executing: %s\n" RESET, input);

    if (strcmp(input, "cpuStatus") == 0) {
        PrintCPUStatus();

    } else if (strcmp(input, "networkStatus") == 0) {
        PrintNetworkStatus();

    } else if (strcmp(input, "ramStatus") == 0) {
        PrintMemoryStatus();

    } else if (strcmp(input, "cpu") == 0) {
        PrintCPUStatus();
        PrintAllProcessCPUUsage();

    } else if (strcmp(input, "network") == 0) {
        PrintNetworkStatus();
        PrintAllProcessNetworkUsage();

    } else if (strcmp(input, "ram") == 0) {
        PrintMemoryStatus();
        PrintAllProcessRAMUsage();

    } else if (strcmp(input, "utilization") == 0) {
        PrintAllProcessUtilization();

    } else if (strcmp(input, "search_cpu") == 0) {
        SearchCpuProcessByName();

    } else if (strcmp(input, "search_ram") == 0) {
        SearchRamProcessByName();

    } else if (strcmp(input, "search_network") == 0) {
        SearchNetworkProcessByName();

    } else if (strcmp(input, "list") == 0) {
        ProcessListing();

    } else if (strcmp(input, "gui") == 0) {
        LaunchGUI();

    } else if (strcmp(input, "show_process_diskusage_all") == 0) {
        ShowAllProcessesDiskUsage();

    } else if (strcmp(input, "show_process_diskusagebypid") == 0) {
        printf(YELLOW "\nEnter PID to analyze specific process: " RESET);
        DWORD pid;
        scanf("%lu", &pid);
        ShowSpecificProcessDiskUsage(pid);

    } else if (strcmp(input, "analyze_folder_size") == 0) {
        TCHAR path[MAX_PATH];
        _tprintf(TEXT(YELLOW " Enter folder path: " RESET));
        _getts(path);
        DWORD clusterSize = GetClusterSize(path);
        analyzeFolderSizes(path, clusterSize);

    } else if (strcmp(input, "analyze_extension_distribution") == 0) {
        TCHAR path[MAX_PATH];
        _tprintf(TEXT(YELLOW " Enter folder path: " RESET));
        _getts(path);
        DWORD clusterSize = GetClusterSize(path);
        analyzeExtensionDistribution(path, clusterSize);

    } else {
        printf(RED "\n[!] Invalid input!\n" RESET);
        PrintHelp();
    }

    printf(GREEN "\n-----------------------------------------\n" RESET);
}

int main() {
    char input[INPUT_SIZE];
    PrintBanner();
    PrintHelp();

    while (1) {
        printf(BLUE "command > " RESET);
        if (fgets(input, INPUT_SIZE, stdin)) {
            input[strcspn(input, "\n")] = 0;

            if (strcmp(input, "exit") == 0) {
                printf(GREEN "\nExiting... Goodbye!\n" RESET);
                break;
            }

            HandleUserInput(input);
        }
    }

    return 0;
}
