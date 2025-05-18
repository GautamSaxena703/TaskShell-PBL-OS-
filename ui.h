#ifndef PROCESS_UTIL_MONITOR_H
#define PROCESS_UTIL_MONITOR_H

#include <windows.h>
#include <commctrl.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "comctl32.lib")

#define MAX_PROCESS_NAME_LENGTH MAX_PATH

typedef struct {
    char processName[MAX_PROCESS_NAME_LENGTH];
    DWORD pid;
    double cpuTimeSeconds;
    SIZE_T ramUsageBytes;
    ULONGLONG diskReadBytes;
    ULONGLONG diskWriteBytes;
}  GuiProcessUtilInfo;

extern HWND hListView;

void InitListViewColumns(HWND hwndListView);
void InsertListViewItem(HWND hwndListView, int index, GuiProcessUtilInfo* pInfo);
void PopulateProcessData(HWND hwndListView);
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int LaunchGUI(void);

#endif 
