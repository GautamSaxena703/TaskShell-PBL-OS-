
#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include "ui.h"

void HandleUserInput(const char* input) {
    if (strcmp(input, "cpuStatus") == 0) {
        PrintCPUStatus();
      
    } else if (strcmp(input, "networkStatus") == 0) {
        PrintNetworkStatus();
  
    } else if (strcmp(input, "ramStatus") == 0) {
        PrintMemoryStatus();
     
    }else if (strcmp(input, "cpu") == 0) {
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
    } else if(strcmp(input,"show_process_diskusage_all") == 0){
        ShowAllProcessesDiskUsage();
    } else if(strcmp(input,"show_process_diskusagebypid") == 0){
        printf("\nEnter PID to analyze specific process: ");
        DWORD pid;
        scanf("%lu", &pid);

        ShowSpecificProcessDiskUsage(pid);
    }else if(strcmp(input,"analyze_folder_size") == 0){
        TCHAR path[MAX_PATH];
        _tprintf(TEXT(" Enter folder path: "));
        _getts(path);

        DWORD clusterSize = GetClusterSize(path);

        analyzeFolderSizes(path, clusterSize);
    } else if(strcmp(input,"analyze_extension_distribution") == 0){
        TCHAR path[MAX_PATH];
        _tprintf(TEXT(" Enter folder path: "));
        _getts(path);

        DWORD clusterSize = GetClusterSize(path);

        analyzeExtensionDistribution(path, clusterSize);
    }
    else {
        printf("Invalid input. Try: cpuStatus, ramStatus, networkStatus, cpu, ram, network, utilization, search_cpu, search_ram, search_network, list, gui, show_process_diskusage_all, show_process_diskusagebypid, analyze_folder_size, analyze_extension_distribution\n");
    }
}


int main() {
    char input[INPUT_SIZE];
    printf("\nEnter a command (cpuStatus, ramStatus, networkStatus, cpu, ram, network, utilization, search_cpu, search_ram, search_network, list, gui,  show_process_diskusage_all, show_process_diskusagebypid, analyze_folder_size, analyze_extension_distribution, exit )\n ");
    while (1) {
        printf("command : ");
        if (fgets(input, INPUT_SIZE, stdin)) {
           
            input[strcspn(input, "\n")] = 0;

            if (strcmp(input, "exit") == 0) {
                break;
            }

            HandleUserInput(input);
        }
    }

    return 0;
}