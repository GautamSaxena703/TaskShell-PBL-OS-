#define WIN32_LEAN_AND_MEAN
#define _WIN32_IE 0x0500

#include <windows.h>
#include <commctrl.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "comctl32.lib")

typedef struct {
    char processName[MAX_PATH];
    DWORD pid;
    double cpuTimeSeconds;
    SIZE_T ramUsageBytes;
    ULONGLONG diskReadBytes;
    ULONGLONG diskWriteBytes;
} GuiProcessUtilInfo;

HWND hListView;
int g_sortColumn = 0; // Default sort by Process Name
BOOL g_sortAscending = TRUE; // Ascending order by default

// Dynamic array for processes
GuiProcessUtilInfo* g_procArray = NULL;
size_t g_procCount = 0;

void InitListViewColumns(HWND hwndListView);
void InsertListViewItem(HWND hwndListView, int index, const GuiProcessUtilInfo* pInfo);
void PopulateProcessData(HWND hwndListView);
void ClearProcessData();
int CompareProcesses(const void* a, const void* b);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


int LaunchGUI() {
    HINSTANCE hInst = GetModuleHandle(NULL);

    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    const char CLASS_NAME[] = "ProcessUtilApp_Class";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Process Utilization Monitoring",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 600,
        NULL, NULL, hInst, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 500, NULL); 

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    const char CLASS_NAME[] = "ProcessUtilApp_Class";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Process Utilization Monitoring",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 600,
        NULL, NULL, hInst, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 500, NULL);  // Update every 0.5 seconds

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ClearProcessData();

    return (int) msg.wParam;
}

void InitListViewColumns(HWND hwndListView) {
    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    const char* columns[] = {
        "Process Name", "PID", "CPU Time (s)", "RAM (MB)", "Disk Read (MB)", "Disk Write (MB)", "Network"
    };
    int widths[] = { 220, 70, 100, 100, 120, 120, 80 };
    int colCount = (int)(sizeof(columns) / sizeof(columns[0]));

    for (int i = 0; i < colCount; i++) {
        lvc.iSubItem = i;
        lvc.cx = widths[i];
        lvc.pszText = (LPSTR)columns[i];
        ListView_InsertColumn(hwndListView, i, &lvc);
    }
}

void InsertListViewItem(HWND hwndListView, int index, const GuiProcessUtilInfo* pInfo) {
    char buf[256];

    LVITEM lvi = {0};
    lvi.mask = LVIF_TEXT;
    lvi.iItem = index;
    lvi.iSubItem = 0;
    lvi.pszText = (LPSTR)pInfo->processName;
    ListView_InsertItem(hwndListView, &lvi);

    snprintf(buf, sizeof(buf), "%u", pInfo->pid);
    ListView_SetItemText(hwndListView, index, 1, buf);

    snprintf(buf, sizeof(buf), "%.2f", pInfo->cpuTimeSeconds);
    ListView_SetItemText(hwndListView, index, 2, buf);

    snprintf(buf, sizeof(buf), "%.2f", pInfo->ramUsageBytes / (1024.0 * 1024.0));
    ListView_SetItemText(hwndListView, index, 3, buf);

    snprintf(buf, sizeof(buf), "%.2f", pInfo->diskReadBytes / (1024.0 * 1024.0));
    ListView_SetItemText(hwndListView, index, 4, buf);

    snprintf(buf, sizeof(buf), "%.2f", pInfo->diskWriteBytes / (1024.0 * 1024.0));
    ListView_SetItemText(hwndListView, index, 5, buf);

    ListView_SetItemText(hwndListView, index, 6, "N/A");
}

void ClearProcessData() {
    if (g_procArray) {
        free(g_procArray);
        g_procArray = NULL;
    }
    g_procCount = 0;
}

void PopulateProcessData(HWND hwndListView) {
    // Enumerate processes
    DWORD processes[1024], cbNeeded, count;
    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        MessageBox(hwndListView, "Failed to enumerate processes!", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    count = cbNeeded / sizeof(DWORD);

    // Free old data
    ClearProcessData();

    // Allocate enough memory for process info array
    g_procArray = (GuiProcessUtilInfo*)malloc(sizeof(GuiProcessUtilInfo) * count);
    if (!g_procArray) {
        MessageBox(hwndListView, "Memory allocation failed!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    size_t actualCount = 0;

    for (size_t i = 0; i < count; i++) {
        DWORD pid = processes[i];
        if (pid == 0) continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProcess) continue;

        HMODULE hMod;
        DWORD cbNeededMod;
        char procName[MAX_PATH] = "<unknown>";
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
            GetModuleBaseNameA(hProcess, hMod, procName, sizeof(procName));
        }

        FILETIME creationTime, exitTime, kernelTime, userTime;
        double cpuSecs = 0.0;
        if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
            ULONGLONG kTime = (((ULONGLONG)kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
            ULONGLONG uTime = (((ULONGLONG)userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;
            cpuSecs = (double)(kTime + uTime) / 10000000.0;
        }

        PROCESS_MEMORY_COUNTERS pmc = {0};
        SIZE_T ramBytes = 0;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            ramBytes = pmc.WorkingSetSize;
        }

        IO_COUNTERS ioCounters;
        ULONGLONG readBytes = 0, writeBytes = 0;
        if (GetProcessIoCounters(hProcess, &ioCounters)) {
            readBytes = ioCounters.ReadTransferCount;
            writeBytes = ioCounters.WriteTransferCount;
        }

        GuiProcessUtilInfo* pInfo = &g_procArray[actualCount];
        strncpy(pInfo->processName, procName, MAX_PATH);
        pInfo->processName[MAX_PATH - 1] = '\0';
        pInfo->pid = pid;
        pInfo->cpuTimeSeconds = cpuSecs;
        pInfo->ramUsageBytes = ramBytes;
        pInfo->diskReadBytes = readBytes;
        pInfo->diskWriteBytes = writeBytes;

        actualCount++;

        CloseHandle(hProcess);
    }

    g_procCount = actualCount;

    // Sort the array
    if (g_sortColumn >= 0 && g_sortColumn <= 6) {
        qsort(g_procArray, g_procCount, sizeof(GuiProcessUtilInfo), CompareProcesses);
    }

    // Clear ListView items
    ListView_DeleteAllItems(hwndListView);

    // Insert sorted items
    for (size_t i = 0; i < g_procCount; i++) {
        InsertListViewItem(hwndListView, (int)i, &g_procArray[i]);
    }
}

// Comparator for qsort
int CompareProcesses(const void* a, const void* b) {
    const GuiProcessUtilInfo* p1 = (const GuiProcessUtilInfo*)a;
    const GuiProcessUtilInfo* p2 = (const GuiProcessUtilInfo*)b;
    int result = 0;

    switch (g_sortColumn) {
        case 0: // Process Name
            result = strcmp(p1->processName, p2->processName);
            break;
        case 1: // PID
            if (p1->pid < p2->pid) result = -1;
            else if (p1->pid > p2->pid) result = 1;
            else result = 0;
            break;
        case 2: // CPU Time
            if (p1->cpuTimeSeconds < p2->cpuTimeSeconds) result = -1;
            else if (p1->cpuTimeSeconds > p2->cpuTimeSeconds) result = 1;
            else result = 0;
            break;
        case 3: // RAM
            if (p1->ramUsageBytes < p2->ramUsageBytes) result = -1;
            else if (p1->ramUsageBytes > p2->ramUsageBytes) result = 1;
            else result = 0;
            break;
        case 4: // Disk Read
            if (p1->diskReadBytes < p2->diskReadBytes) result = -1;
            else if (p1->diskReadBytes > p2->diskReadBytes) result = 1;
            else result = 0;
            break;
        case 5: // Disk Write
            if (p1->diskWriteBytes < p2->diskWriteBytes) result = -1;
            else if (p1->diskWriteBytes > p2->diskWriteBytes) result = 1;
            else result = 0;
            break;
        case 6: // Network - no data, keep equal
        default:
            result = 0;
            break;
    }

    return g_sortAscending ? result : -result;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hListView = CreateWindowEx(
                WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
                WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_VSCROLL | WS_HSCROLL,
                10, 10, 860, 540,
                hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
            if (!hListView) {
                MessageBox(hwnd, "Failed to create ListView", "Error", MB_OK | MB_ICONERROR);
                return -1;
            }
            InitListViewColumns(hListView);
            PopulateProcessData(hListView);
            return 0;

        case WM_SIZE: {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            SetWindowPos(hListView, NULL, 10, 10, rcClient.right - 20, rcClient.bottom - 20, SWP_NOZORDER);
            return 0;
        }

        case WM_NOTIFY: {
            NMHDR* pnmh = (NMHDR*)lParam;
            if (pnmh->hwndFrom == hListView && pnmh->code == LVN_COLUMNCLICK) {
                NMLISTVIEW* pnmv = (NMLISTVIEW*)lParam;
                int clickedColumn = pnmv->iSubItem;

                if (g_sortColumn == clickedColumn) {
                    g_sortAscending = !g_sortAscending;  // toggle
                } else {
                    g_sortColumn = clickedColumn;
                    g_sortAscending = TRUE;
                }

                PopulateProcessData(hListView); // Refresh with sorting
                return 0;
            }
            break;
        }

        case WM_TIMER:
            PopulateProcessData(hListView);
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            ClearProcessData();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

