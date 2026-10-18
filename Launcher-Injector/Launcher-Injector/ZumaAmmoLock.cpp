// launcher.cpp -- ZumaAmmoLock.exe  (x86 / Win32, Release, /MT, Subsystem: Windows)
// Taruh bareng zuma_ammo_lock.dll di folder game, lalu klik dua kali.
#include <windows.h>
#include <tlhelp32.h>
#include <cwchar>

static const wchar_t* WRAPPERS[] = { L"Zuma-windowed.exe", L"Zuma.exe" };

static DWORD FindGame() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe = { sizeof(pe) };
    DWORD pid = 0;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (!_wcsicmp(pe.szExeFile, L"popcapgame1.exe") ||
                !_wcsicmp(pe.szExeFile, L"popcapgame2.exe") ||
                !_wcsicmp(pe.szExeFile, L"popcapgame3.exe")) {
                pid = pe.th32ProcessID; break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

static void Fail(const wchar_t* msg) {
    MessageBoxW(nullptr, msg, L"Zuma Ammo Lock", MB_OK | MB_ICONERROR);
}

static bool Inject(DWORD pid, const wchar_t* dll) {
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!h) return false;
    WaitForInputIdle(h, 15000);
    Sleep(500);

    SIZE_T sz = (wcslen(dll) + 1) * sizeof(wchar_t);
    LPVOID mem = VirtualAllocEx(h, nullptr, sz, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    bool ok = false;
    if (mem && WriteProcessMemory(h, mem, dll, sz, nullptr)) {
        LPTHREAD_START_ROUTINE ll = (LPTHREAD_START_ROUTINE)
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
        HANDLE t = CreateRemoteThread(h, nullptr, 0, ll, mem, 0, nullptr);
        if (t) {
            WaitForSingleObject(t, 10000);
            DWORD ret = 0; GetExitCodeThread(t, &ret);
            ok = ret != 0;
            CloseHandle(t);
        }
        VirtualFreeEx(h, mem, 0, MEM_RELEASE);
    }
    CloseHandle(h);
    return ok;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    wchar_t dir[MAX_PATH];
    GetModuleFileNameW(nullptr, dir, MAX_PATH);
    wchar_t* slash = wcsrchr(dir, L'\\');
    if (slash) *slash = 0;

    wchar_t dll[MAX_PATH], exe[MAX_PATH] = L"";
    swprintf_s(dll, L"%s\\zuma_ammo_lock.dll", dir);
    if (GetFileAttributesW(dll) == INVALID_FILE_ATTRIBUTES) {
        Fail(L"File zuma_ammo_lock.dll tidak ada di folder ini.");
        return 1;
    }

    for (const wchar_t* w : WRAPPERS) {
        wchar_t p[MAX_PATH];
        swprintf_s(p, L"%s\\%s", dir, w);
        if (GetFileAttributesW(p) != INVALID_FILE_ATTRIBUTES) { wcscpy_s(exe, p); break; }
    }
    if (!exe[0]) {
        Fail(L"Zuma-windowed.exe tidak ketemu.\nTaruh file ini di folder game (sebelah Zuma-windowed.exe).");
        return 1;
    }

    if (FindGame()) {
        Fail(L"Game sudah berjalan. Tutup dulu, lalu buka lewat launcher ini.");
        return 1;
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(exe, nullptr, nullptr, nullptr, FALSE, 0, nullptr, dir, &si, &pi)) {
        Fail(L"Gagal menjalankan game.");
        return 1;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    DWORD pid = 0;
    for (int i = 0; i < 240 && !(pid = FindGame()); i++) Sleep(250);   // maks 60 detik
    if (!pid) { Fail(L"Proses game (popcapgame*.exe) tidak muncul."); return 1; }

    if (!Inject(pid, dll))
        Fail(L"Gagal memasang mod.\nCoba klik kanan launcher -> Run as administrator.");
    return 0;
}