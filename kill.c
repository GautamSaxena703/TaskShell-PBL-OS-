#include <windows.h>
#include <stdio.h>

int main() {
    DWORD pid;
    printf("Enter the PID of the process to terminate: ");
    scanf("%lu", &pid);

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        printf("Failed to open process. Error code: %lu\n", GetLastError());
        return 1;
    }

    if (TerminateProcess(hProcess, 0)) {
        printf("Successfully terminated process with PID %lu.\n", pid);
    } else {
        printf("Failed to terminate process. Error code: %lu\n", GetLastError());
    }

    CloseHandle(hProcess);
    return 0;
}
