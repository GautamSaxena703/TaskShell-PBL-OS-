#include <windows.h>
#include <stdio.h>
#include <string.h>

#define TABLE_FILE "task_table.txt"

int main() {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    char command[100];

    printf("Enter process to launch (e.g., notepad.exe): ");
    fgets(command, sizeof(command), stdin);
    command[strcspn(command, "\n")] = 0;

    if (CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("Started %s with PID: %lu\n", command, pi.dwProcessId);

        // Append process info to table file
        FILE* file = fopen(TABLE_FILE, "a");
        if (file) {
            fprintf(file, "%lu %s\n", pi.dwProcessId, command);
            fclose(file);
        } else {
            printf("Failed to write to task table.\n");
        }

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    } else {
        printf("CreateProcess failed (%lu)\n", GetLastError());
    }

    return 0;
}
