#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <string>

static DWORD FindProcessId(const std::string& processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);
        if (Process32FirstW(snapshot, &entry)) {
            do {
                std::wstring name(entry.szExeFile);
                std::string narrowName(name.begin(), name.end());
                if (_stricmp(narrowName.c_str(), processName.c_str()) == 0) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
    }
    return pid;
}

int main() {
    std::cout << "========================================================\n";
    std::cout << " [V.I.I.B.E CS 1.6 Dedicated Injection Engine v2.7]\n";
    std::cout << " Target: OyunYoneticisi / GoldSrc hl.exe / cstrike.exe\n";
    std::cout << "========================================================\n\n";

    DWORD pid = FindProcessId("hl.exe");
    if (pid == 0) pid = FindProcessId("cstrike.exe");

    if (pid == 0) {
        std::cout << "[-] Waiting for hl.exe / cstrike.exe to start...\n";
        while (pid == 0) {
            Sleep(500);
            pid = FindProcessId("hl.exe");
            if (pid == 0) pid = FindProcessId("cstrike.exe");
        }
    }

    std::cout << "[+] Found game process! PID: " << pid << "\n";

    char dllPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, dllPath);
    strcat_s(dllPath, "\\cs16_esp_internal.dll");

    char newDllPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, newDllPath);
    strcat_s(newDllPath, "\\cs16_esp_internal_new.dll");

    if (GetFileAttributesA(newDllPath) != INVALID_FILE_ATTRIBUTES) {
        if (CopyFileA(newDllPath, dllPath, FALSE)) {
            DeleteFileA(newDllPath);
        } else {
            // If primary is locked by running game, inject the newly compiled binary directly
            strcpy_s(dllPath, newDllPath);
        }
    }

    DWORD attr = GetFileAttributesA(dllPath);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "[-] Error: cs16_esp_internal.dll not found in current directory!\n";
        system("pause");
        return 1;
    }

    std::cout << "[+] Target DLL: " << dllPath << "\n";

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        std::cerr << "[-] Failed to open process. Error: " << GetLastError() << "\n";
        std::cout << "[-] Make sure to Run as Administrator.\n";
        system("pause");
        return 1;
    }

    size_t len = strlen(dllPath) + 1;
    void* remoteMem = VirtualAllocEx(hProc, NULL, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMem) {
        std::cerr << "[-] VirtualAllocEx failed. Error: " << GetLastError() << "\n";
        CloseHandle(hProc);
        system("pause");
        return 1;
    }

    if (!WriteProcessMemory(hProc, remoteMem, dllPath, len, NULL)) {
        std::cerr << "[-] WriteProcessMemory failed. Error: " << GetLastError() << "\n";
        VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProc);
        system("pause");
        return 1;
    }

    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    FARPROC pLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");

    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, remoteMem, 0, NULL);
    if (!hThread) {
        std::cerr << "[-] CreateRemoteThread failed. Error: " << GetLastError() << "\n";
        VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProc);
        system("pause");
        return 1;
    }

    std::cout << "[+] Injection thread created successfully!\n";
    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
    VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
    CloseHandle(hProc);

    std::cout << "\n[SUCCESS] V.I.I.B.E Internal ESP injected.\n";
    std::cout << "Controls in-game:\n";
    std::cout << "  - INSERT : Toggle Menu\n";
    std::cout << "  - ARROWS : Navigate Menu\n";
    std::cout << "  - F11    : Dump Diagnostic Log\n";
    std::cout << "  - END    : Eject DLL cleanly\n\n";

    Sleep(2000);
    return 0;
}
