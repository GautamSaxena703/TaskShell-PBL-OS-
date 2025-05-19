#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 

#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iphlpapi.h>
#include<tlhelp32.h>
#define MAX_PROCESSES 1024
#define INPUT_SIZE 100
#define MAX_EXT 256


#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")

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
    ULONGLONG bytesSent;  
    ULONGLONG bytesRecv;
} ProcessUtilInfo;

typedef struct {
    TCHAR ext[16];
    ULONGLONG size;
} ExtStat;

//CPU
void PrintCPUStatus();
int GetCpuProcesses(ProcessCpuInfo processes[]);
void PrintAllProcessCPUUsage();
void SearchCpuProcessByName();

//NETWORK
void PrintNetworkStatus();
int GetProcessesNetworkUsage(ProcessNetworkInfo processes[]);
void PrintAllProcessNetworkUsage();
void SearchNetworkProcessByName();

//RAM

void PrintMemoryStatus();
int compareRAMUsage(const void *a, const void *b);
int GetRamProcesses(ProcessRamInfo processes[]);
void PrintAllProcessRAMUsage();
void SearchRamProcessByName();


void ProcessListing();
void PrintAllProcessUtilization();

// void HandleUserInput(const char* input) ;

void PrintCPUStatus() {
    FILETIME idleTime1, kernelTime1, userTime1;
    FILETIME idleTime2, kernelTime2, userTime2;

    if (!GetSystemTimes(&idleTime1, &kernelTime1, &userTime1)) {
        printf("Failed to get system times (first sample).\n");
        return;
    }

    Sleep(500);  // Wait half a second

    if (!GetSystemTimes(&idleTime2, &kernelTime2, &userTime2)) {
        printf("Failed to get system times (second sample).\n");
        return;
    }

    ULARGE_INTEGER idle1, kernel1, user1;
    ULARGE_INTEGER idle2, kernel2, user2;

    idle1.LowPart = idleTime1.dwLowDateTime;
    idle1.HighPart = idleTime1.dwHighDateTime;
    kernel1.LowPart = kernelTime1.dwLowDateTime;
    kernel1.HighPart = kernelTime1.dwHighDateTime;
    user1.LowPart = userTime1.dwLowDateTime;
    user1.HighPart = userTime1.dwHighDateTime;

    idle2.LowPart = idleTime2.dwLowDateTime;
    idle2.HighPart = idleTime2.dwHighDateTime;
    kernel2.LowPart = kernelTime2.dwLowDateTime;
    kernel2.HighPart = kernelTime2.dwHighDateTime;
    user2.LowPart = userTime2.dwLowDateTime;
    user2.HighPart = userTime2.dwHighDateTime;

    ULONGLONG idleDiff = idle2.QuadPart - idle1.QuadPart;
    ULONGLONG kernelDiff = kernel2.QuadPart - kernel1.QuadPart;
    ULONGLONG userDiff = user2.QuadPart - user1.QuadPart;

    ULONGLONG system = kernelDiff + userDiff;

    if (system == 0) {
        printf("Invalid system time difference.\n");
        return;
    }

    double cpuUsage = (double)(system - idleDiff) * 100.0 / system;
    printf("Total CPU Utilization: %.2f%%\n", cpuUsage);
}

int GetCpuProcesses(ProcessCpuInfo processes[]) {
    DWORD processIDs[MAX_PROCESSES], cbNeeded, numProcesses;
    int processCount = 0;

    if (!EnumProcesses(processIDs, sizeof(processIDs), &cbNeeded)) {
        printf("Failed to enumerate processes.\n");
        return 0;
    }

    numProcesses = cbNeeded / sizeof(DWORD);

    for (DWORD i = 0; i < numProcesses && processCount < MAX_PROCESSES; i++) {
        DWORD pid = processIDs[i];
        if (pid == 0) continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProcess) continue;

        HMODULE hMod;
        DWORD cbNeededMod;
        char processName[MAX_PATH] = "<unknown>";
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
            GetModuleBaseNameA(hProcess, hMod, processName, sizeof(processName));
        }

        FILETIME creationTime, exitTime, kernelTime, userTime;
        if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
            ULONGLONG kTime = (((ULONGLONG)kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
            ULONGLONG uTime = (((ULONGLONG)userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;
            double totalTime = (double)(kTime + uTime) / 10000000.0; // Convert to seconds

            strncpy(processes[processCount].processName, processName, MAX_PATH);
            processes[processCount].pid = pid;
            processes[processCount].cpuUsage = totalTime;
            processCount++;
        }

        CloseHandle(hProcess);
    }

    return processCount;
}

void PrintAllProcessCPUUsage() {
    ProcessCpuInfo processes[MAX_PROCESSES];
    int processCount = GetCpuProcesses(processes);

    if (processCount == 0) return;

    printf("\n--- Process CPU Usage ---\n");
    printf("%-30s  %-6s  %-10s\n", "Process Name", "PID", "CPU Time (s)");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < processCount; i++) {
        printf("%-30s  %-6u  %-10.2f\n", processes[i].processName, processes[i].pid, processes[i].cpuUsage);
    }
}

void SearchCpuProcessByName() {
    ProcessCpuInfo processes[MAX_PROCESSES];
    int processCount = GetCpuProcesses(processes);

    if (processCount == 0) return;

    char searchTerm[MAX_PATH];
    printf("Enter process name to search: ");
    scanf("%s", searchTerm);

    double totalCPU = 0.0;
    int found = 0;

    for (int i = 0; i < processCount; i++) {
        if (strstr(processes[i].processName, searchTerm) != NULL) {
            totalCPU += processes[i].cpuUsage;
            found = 1;
        }
    }

    if (found) {
        printf("\nTotal CPU time for processes matching '%s': %.2f seconds\n", searchTerm, totalCPU);
    } else {
        printf("\nNo processes found matching '%s'.\n", searchTerm);
    }
}



void PrintNetworkStatus() {
    MIB_IFTABLE* pIfTable1 = NULL, * pIfTable2 = NULL;
    DWORD dwSize = 0, dwRetVal;

    // First snapshot
    GetIfTable(NULL, &dwSize, TRUE);
    pIfTable1 = (MIB_IFTABLE*)malloc(dwSize);
    if (!pIfTable1) {
        printf("Memory allocation failed.\n");
        return;
    }

    if ((dwRetVal = GetIfTable(pIfTable1, &dwSize, TRUE)) != NO_ERROR) {
        printf("GetIfTable failed: %lu\n", dwRetVal);
        free(pIfTable1);
        return;
    }

    ULONGLONG sent1 = 0, recv1 = 0;
    for (DWORD i = 0; i < pIfTable1->dwNumEntries; i++) {
        sent1 += pIfTable1->table[i].dwOutOctets;
        recv1 += pIfTable1->table[i].dwInOctets;
    }

    Sleep(1000); // Wait 1 second

    // Second snapshot
    GetIfTable(NULL, &dwSize, TRUE);
    pIfTable2 = (MIB_IFTABLE*)malloc(dwSize);
    if (!pIfTable2) {
        printf("Memory allocation failed (second).\n");
        free(pIfTable1);
        return;
    }

    if ((dwRetVal = GetIfTable(pIfTable2, &dwSize, TRUE)) != NO_ERROR) {
        printf("GetIfTable failed (second): %lu\n", dwRetVal);
        free(pIfTable1);
        free(pIfTable2);
        return;
    }

    ULONGLONG sent2 = 0, recv2 = 0;
    for (DWORD i = 0; i < pIfTable2->dwNumEntries; i++) {
        sent2 += pIfTable2->table[i].dwOutOctets;
        recv2 += pIfTable2->table[i].dwInOctets;
    }

    free(pIfTable1);
    free(pIfTable2);

    double mbps = ((double)((sent2 - sent1) + (recv2 - recv1)) * 8) / 1000000.0;
    printf("Network Utilization: Sent+Received = %.2f Mbps\n", mbps);
}

int GetProcessesNetworkUsage(ProcessNetworkInfo processes[]) {
    DWORD processIDs[1024], cbNeeded;
    unsigned int count = 0;

    if (!EnumProcesses(processIDs, sizeof(processIDs), &cbNeeded)) {
        printf("EnumProcesses failed.\n");
        return 0;
    }

    DWORD numProcs = cbNeeded / sizeof(DWORD);
    for (DWORD i = 0; i < numProcs; i++) {
        DWORD pid = processIDs[i];
        if (pid == 0) continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            HMODULE hMod;
            DWORD cbMod;
            char name[MAX_PATH] = "<unknown>";
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbMod)) {
                GetModuleBaseNameA(hProcess, hMod, name, sizeof(name));
            }

            strncpy(processes[count].processName, name, MAX_PATH);
            processes[count].pid = pid;
            processes[count].bytesSent = 0; // Needs external driver to track
            processes[count].bytesRecv = 0;

            CloseHandle(hProcess);
            count++;
        }
    }

    return count;
}

void PrintAllProcessNetworkUsage() {
    ProcessNetworkInfo processes[1024];
    int count = GetProcessesNetworkUsage(processes);

    if (count == 0) return;

    printf("\n--- Process Network Usage (Bytes sent and received - demo only) ---\n");
    printf("%-30s  %-6s  %-15s  %-15s\n", "Process Name", "PID", "Bytes Sent", "Bytes Received");
    printf("-------------------------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("%-30s  %-6u  %-15llu  %-15llu\n",
            processes[i].processName,
            processes[i].pid,
            processes[i].bytesSent,
            processes[i].bytesRecv);
    }
}

void SearchNetworkProcessByName() {
    ProcessNetworkInfo processes[1024];
    int count = GetProcessesNetworkUsage(processes);
    if (count == 0) return;

    char search[MAX_PATH];
    printf("Enter process name to search: ");
    scanf("%s", search);

    ULONGLONG totalSent = 0, totalRecv = 0;
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (strstr(processes[i].processName, search)) {
            totalSent += processes[i].bytesSent;
            totalRecv += processes[i].bytesRecv;
            found = 1;
        }
    }

    if (found) {
        printf("Total usage for processes matching '%s':\n", search);
        printf("  Bytes Sent: %llu\n", totalSent);
        printf("  Bytes Received: %llu\n", totalRecv);
    } else {
        printf("No processes found matching '%s'.\n", search);
    }
}




void PrintMemoryStatus() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    DWORDLONG physMemUsed = totalPhysMem - memInfo.ullAvailPhys;

    printf("Total RAM: %llu MB\n", totalPhysMem / (1024 * 1024));
    printf("Used RAM : %llu MB\n", physMemUsed / (1024 * 1024));
}

int compareRAMUsage(const void *a, const void *b) {
    ProcessRamInfo *processA = (ProcessRamInfo*)a;
    ProcessRamInfo *processB = (ProcessRamInfo*)b;

    if (processA->ramUsageKB < processB->ramUsageKB) return 1;
    if (processA->ramUsageKB > processB->ramUsageKB) return -1;
    return 0;
}

int GetRamProcesses(ProcessRamInfo processes[]) {
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
            strncpy(processes[processCount].processName, processName, MAX_PATH);
            processes[processCount].pid = pid;
            processes[processCount].ramUsageKB = pmc.WorkingSetSize / 1024; 
            processCount++;
        }

        CloseHandle(hProcess);
    }

    return processCount;
}

void PrintAllProcessRAMUsage() {
    ProcessRamInfo processes[1024];
    int processCount = GetRamProcesses(processes);

    if (processCount == 0) return;

    qsort(processes, processCount, sizeof(ProcessRamInfo), compareRAMUsage);

    printf("\n--- Sorted Process RAM Usage ---\n");
    printf("%-30s  %-6s  %-10s\n", "Process Name", "PID", "RAM (KB)");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < processCount; i++) {
        printf("%-30s  %-6u  %-10lu kb\n", processes[i].processName, processes[i].pid, processes[i].ramUsageKB);
    }
}

void SearchRamProcessByName() {
    ProcessRamInfo processes[1024];
    int processCount = GetRamProcesses(processes);

    if (processCount == 0) return;

    char searchTerm[MAX_PATH];
    printf("Enter process name to search: ");
    scanf("%s", searchTerm);

    SIZE_T totalRAM = 0;
    int found = 0;
    
    for (int i = 0; i < processCount; i++) {
        if (strstr(processes[i].processName, searchTerm) != NULL) {
            totalRAM += processes[i].ramUsageKB;
            found = 1;
        }
    }

    if (found) {
        printf("\nTotal RAM usage for processes matching '%s': %lu MB\n", searchTerm, totalRAM/1024);
    } else {
        printf("\nNo processes found matching '%s'.\n", searchTerm);
    }
}


void ProcessListing(){
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
     
        printf("Error: %lu\n", GetLastError());
        return ;
    }
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(snapshot, &entry)) {
        do {
            _tprintf(_T("Process Name: %s\n"), entry.szExeFile);
        } while (Process32Next(snapshot, &entry));
    }
    printf("\n-------------------------\n");
    printf("Enter a process to be searched:-\n");
    char process_search[30];
    scanf("%s",&process_search);

    if (Process32First(snapshot, &entry)) {
        do {
            if(strcmp(process_search, entry.szExeFile) == 0)
            {
                _tprintf(_T("Process Name found : %s\n"), entry.szExeFile);
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

void PrintAllProcessUtilization() {
    DWORD processIDs[1024], cbNeeded, numProcesses;
    if (!EnumProcesses(processIDs, sizeof(processIDs), &cbNeeded)) {
        printf("Failed to enumerate processes.\n");
        return;
    }
    numProcesses = cbNeeded / sizeof(DWORD);

    ProcessUtilInfo* infos = (ProcessUtilInfo*)malloc(numProcesses * sizeof(ProcessUtilInfo));
    if (!infos) {
        printf("Out of memory.\n");
        return;
    }
    int count = 0;

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

        processName[MAX_PATH - 1] = '\0';

        FILETIME creationTime, exitTime, kernelTime, userTime;
        double cpuTimeSeconds = 0.0;
        if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
            ULONGLONG kTime = (((ULONGLONG)kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
            ULONGLONG uTime = (((ULONGLONG)userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;
            cpuTimeSeconds = (double)(kTime + uTime) / 10000000.0;
        }

        PROCESS_MEMORY_COUNTERS pmc;
        SIZE_T workingSet = 0;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            workingSet = pmc.WorkingSetSize;
        }

        IO_COUNTERS ioCounters;
        ULONGLONG readBytes = 0, writeBytes = 0;
        if (GetProcessIoCounters(hProcess, &ioCounters)) {
            readBytes = ioCounters.ReadTransferCount;
            writeBytes = ioCounters.WriteTransferCount;
        }

        strncpy(infos[count].processName, processName, MAX_PATH);
        infos[count].pid = pid;
        infos[count].cpuTimeSeconds = cpuTimeSeconds;
        infos[count].ramUsageBytes = workingSet;
        infos[count].diskReadBytes = readBytes;
        infos[count].diskWriteBytes = writeBytes;
        count++;

        CloseHandle(hProcess);
    }

    printf("%-25s %-6s %12s %12s %15s %15s %10s\n",
           "Process Name", "PID", "CPU (s)", "RAM (MB)", "Disk Read (MB)", "Disk Write (MB)", "Network");
    printf("-------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("%-25s %-6u %12.2f %12.2f %15.2f %15.2f %10s\n",
            infos[i].processName,
            infos[i].pid,
            infos[i].cpuTimeSeconds,
            infos[i].ramUsageBytes / (1024.0 * 1024.0),
            infos[i].diskReadBytes / (1024.0 * 1024.0),
            infos[i].diskWriteBytes / (1024.0 * 1024.0),
            "N/A");
    }
    free(infos);
}

BOOL GetProcessDiskUsage(DWORD processID, ULONGLONG* readBytes, ULONGLONG* writeBytes) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess == NULL) {
        return FALSE;
    }

    IO_COUNTERS ioCounters;
    BOOL result = GetProcessIoCounters(hProcess, &ioCounters);
    if (result) {
        *readBytes = ioCounters.ReadTransferCount;
        *writeBytes = ioCounters.WriteTransferCount;
    }

    CloseHandle(hProcess);
    return result;
}

void ShowAllProcessesDiskUsage() {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("Error: Unable to create process snapshot.\n");
        return;
    }

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        printf("Error: Unable to get first process.\n");
        return;
    }

    printf("%-6s %-30s %-15s %-15s %-15s\n", "PID", "Process Name", "Read (KB)", "Write (KB)", "Total (KB)");
    printf("-----------------------------------------------------------------------------------------\n");

    do {
        ULONGLONG readBytes = 0, writeBytes = 0;
        if (GetProcessDiskUsage(pe32.th32ProcessID, &readBytes, &writeBytes)) {
            printf("%-6lu %-30s %-15llu %-15llu %-15llu\n",
                pe32.th32ProcessID,
                pe32.szExeFile,
                readBytes / 1024,
                writeBytes / 1024,
                (readBytes + writeBytes) / 1024
            );
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}

//  Show specific process disk usage
void ShowSpecificProcessDiskUsage(DWORD pid) {
    ULONGLONG readBytes = 0, writeBytes = 0;
    if (GetProcessDiskUsage(pid, &readBytes, &writeBytes)) {
        printf("\nDisk Usage for PID %lu:\n", pid);
        printf("Read Bytes  : %llu KB\n", readBytes / 1024);
        printf("Write Bytes : %llu KB\n", writeBytes / 1024);
        printf("Total       : %llu KB\n", (readBytes + writeBytes) / 1024);
    } else {
        printf("Unable to access process with PID %lu.\n", pid);
    }
}

ULONGLONG RoundToCluster(ULONGLONG size, DWORD clusterSize) {
    return ((size + clusterSize - 1) / clusterSize) * clusterSize;
}

//  Get Cluster Size
DWORD GetClusterSize(const TCHAR *path) {
    DWORD sectorsPerCluster, bytesPerSector, freeClusters, totalClusters;
    if (GetDiskFreeSpace(path, &sectorsPerCluster, &bytesPerSector, &freeClusters, &totalClusters)) {
        return sectorsPerCluster * bytesPerSector;
    }
    return 4096; // default fallback
}

//  Recursive function to calculate folder size
ULONGLONG GetFolderSizeRecursive(const TCHAR *folderPath, DWORD clusterSize) {
    WIN32_FIND_DATA findData;
    TCHAR searchPath[MAX_PATH];
    wsprintf(searchPath, TEXT("%s\\*"), folderPath);

    HANDLE hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    ULONGLONG totalSize = 0;

    do {
        if (_tcscmp(findData.cFileName, TEXT(".")) == 0 || _tcscmp(findData.cFileName, TEXT("..")) == 0)
            continue;

        TCHAR fullPath[MAX_PATH];
        wsprintf(fullPath, TEXT("%s\\%s"), folderPath, findData.cFileName);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            totalSize += GetFolderSizeRecursive(fullPath, clusterSize); // recursion
        } else {
            ULONGLONG size = (((ULONGLONG)findData.nFileSizeHigh) << 32) + findData.nFileSizeLow;
            totalSize += RoundToCluster(size, clusterSize);
        }

    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
    return totalSize;
}

// Function 1: Show size of all subfolders
void analyzeFolderSizes(const TCHAR *basePath, DWORD clusterSize) {
    WIN32_FIND_DATA findData;
    TCHAR searchPath[MAX_PATH];
    wsprintf(searchPath, TEXT("%s\\*"), basePath);

    HANDLE hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT(" Error accessing folder.\n"));
        return;
    }

    _tprintf(TEXT("\n Subfolder Size Breakdown for: %s\n"), basePath);
    _tprintf(TEXT("------------------------------------------------------\n"));

    do {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            _tcscmp(findData.cFileName, TEXT(".")) != 0 &&
            _tcscmp(findData.cFileName, TEXT("..")) != 0) {

            TCHAR fullPath[MAX_PATH];
            wsprintf(fullPath, TEXT("%s\\%s"), basePath, findData.cFileName);

            ULONGLONG size = GetFolderSizeRecursive(fullPath, clusterSize);

            _tprintf(TEXT("%-30s : %.2f MB\n"), findData.cFileName, (double)size / (1024 * 1024));
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
}

// Function 2: Show file type distribution
void analyzeExtensionDistribution(const TCHAR *folderPath, DWORD clusterSize) {
    WIN32_FIND_DATA findData;
    TCHAR searchPath[MAX_PATH];
    wsprintf(searchPath, TEXT("%s\\*"), folderPath);

    HANDLE hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("Error accessing folder.\n"));
        return;
    }

    ExtStat stats[MAX_EXT];
    int extCount = 0;
    ULONGLONG totalSize = 0;

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        TCHAR *ext = _tcsrchr(findData.cFileName, '.');
        if (!ext) ext = TEXT("[no_ext]");

        ULONGLONG size = (((ULONGLONG)findData.nFileSizeHigh) << 32) + findData.nFileSizeLow;
        size = RoundToCluster(size, clusterSize);
        totalSize += size;

        int found = 0;
        for (int i = 0; i < extCount; i++) {
            if (_tcscmp(stats[i].ext, ext) == 0) {
                stats[i].size += size;
                found = 1;
                break;
            }
        }
        if (!found && extCount < MAX_EXT) {
            _tcscpy(stats[extCount].ext, ext);
            stats[extCount].size = size;
            extCount++;
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);

    _tprintf(TEXT("\nFile Extension Distribution in: %s\n"), folderPath);
    _tprintf(TEXT("------------------------------------------------------\n"));
    for (int i = 0; i < extCount; i++) {
        double percent = (double)stats[i].size * 100.0 / totalSize;
        _tprintf(TEXT("%-10s : %.2f MB (%.2f%%)\n"), stats[i].ext, (double)stats[i].size / (1024 * 1024), percent);
    }
}


