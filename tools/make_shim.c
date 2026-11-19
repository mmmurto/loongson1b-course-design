/*
 * make.exe compatibility shim for LoongIDE on Windows 11.
 *
 * Problem: msys-1.0.dll is fixed-base at 0x71000000 and Windows 11 already owns
 * 0x71110000, so the genuine MSYS make dies with
 *   "Couldn't reserve space for cygwin's heap, Win32 error 0"
 * LoongIDE probes `make -v` for MSYS identity, so builds/debug refuse to start
 * ("Need MSYS support, but not found it...").
 *
 * This shim:
 *   -v / --version  -> prints the MSYS-flavoured banner LoongIDE expects
 *   everything else -> runs the native GNU make from SDE Lite (cs-make.exe)
 *                      with the same arguments and forwards its exit code.
 *
 * Build:  gcc -O2 -o make.exe make_shim.c
 */
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define REAL_MAKE "C:\\LoongIDE\\mips-2015.05\\bin\\cs-make.exe"
#define MAKE_VER  "3.81"

int main(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("GNU Make %s\n", MAKE_VER);
            printf("This program built for i686-pc-msys\n");
            fflush(stdout);
            return 0;
        }
    }

    if (GetFileAttributesA(REAL_MAKE) == INVALID_FILE_ATTRIBUTES) {
        fprintf(stderr, "make: cannot find %s\n", REAL_MAKE);
        return 127;
    }

    /* rebuild the command line: "cs-make.exe" + original args (quoted) */
    {
        char cmd[32768];
        size_t len;
        strcpy(cmd, "\"" REAL_MAKE "\"");
        len = strlen(cmd);
        for (i = 1; i < argc && len < sizeof(cmd) - 3; i++) {
            cmd[len++] = ' ';
            cmd[len++] = '"';
            cmd[len] = 0;
            strncat(cmd, argv[i], sizeof(cmd) - len - 3);
            len = strlen(cmd);
            cmd[len++] = '"';
            cmd[len] = 0;
        }

        {
            STARTUPINFOA si;
            PROCESS_INFORMATION pi;
            DWORD rc = 127;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            if (CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                GetExitCodeProcess(pi.hProcess, &rc);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            } else {
                fprintf(stderr, "make: failed to launch real make (err %lu)\n",
                        (unsigned long)GetLastError());
            }
            return (int)rc;
        }
    }
}
