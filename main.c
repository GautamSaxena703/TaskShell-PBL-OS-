
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
    }else if (strcmp(input, "gui") == 0) {
    LaunchGUI();
} else {
        printf("Invalid input. Try: cpuStatus, ramStatus, networkStatus, cpu, ram, network, utilization, search_cpu, search_ram, search_network, list, gui\n");
    }
}


int main() {
    char input[INPUT_SIZE];
    printf("\nEnter a command (cpuStatus, ramStatus, networkStatus, cpu, ram, network, utilization, search_cpu, search_ram, search_network, list, hui, exit )\n ");
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