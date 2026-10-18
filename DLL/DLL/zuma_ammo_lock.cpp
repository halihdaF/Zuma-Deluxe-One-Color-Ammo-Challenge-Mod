// zuma_ammo_lock.cpp  -- DLL (x86 / Win32, Release, Runtime Library: /MT)
// Link: libMinHook.x86.lib (static)
#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <set>
#include <vector>
#include <mutex>
#include <random>
#include <algorithm>
#include "MinHook.h"

#pragma comment(lib, "libMinHook.x86.lib")

// ====== OFFSET (khusus popcapgame1.exe dari Zuma Deluxe v1.0) ======
static const uintptr_t GUN_CTOR_RVA = 0x0CE5B0;
static const uintptr_t GUN_VTABLE_RVA = 0x1466E8;
static const uintptr_t BALL_VTABLE_RVA = 0x146800;
static const uintptr_t GUN_CUR_OFF = 0x2C;
static const uintptr_t GUN_NXT_OFF = 0x30;
static const uintptr_t BALL_COLOR_OFF = 0x08;
static const DWORD     EXPECTED_IMAGE_SIZE = 0x23D000;  // fingerprint build

// ====== CONFIG (zuma_ammo_lock.ini, sefolder dengan DLL) ======
// [lock]
// detect=6        ; detik deteksi palet
// force_color=-1  ; -1 = random tiap level, 0..N = warna tetap
// console=0       ; 1 = tampilkan console debug
static DWORD g_detectMs = 6000;
static int   g_fixedColor = -1;
static bool  g_console = false;
#define LOG(...) do { if (g_console) printf(__VA_ARGS__); } while (0)

// ====== STATE ======
enum Mode { MODE_DETECT, MODE_LOCK };

static HMODULE   g_hMod;
static uintptr_t g_base, g_gunVt, g_ballVt;
static std::mutex g_mtx;
static std::set<uintptr_t> g_guns;
static std::vector<uintptr_t> g_ammo;
static std::set<uint32_t> g_observed;
static Mode  g_mode = MODE_DETECT;
static int   g_forced = -1;
static DWORD g_detectDeadline = 0;
static DWORD g_lastLevelStart = 0;
static std::mt19937 g_rng;

// ====== SAFE MEMORY ACCESS ======
static bool ReadU32(uintptr_t addr, uint32_t& out) {
    __try { out = *(volatile uint32_t*)addr; return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
static bool WriteU32(uintptr_t addr, uint32_t v) {
    __try { *(volatile uint32_t*)addr = v; return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
static bool HasVtable(uintptr_t p, uintptr_t vt) {
    uint32_t v;
    return p && ReadU32(p, v) && v == (uint32_t)vt;
}
static bool IsBall(uintptr_t p) { return HasVtable(p, g_ballVt); }
static bool IsGun(uintptr_t p) { return HasVtable(p, g_gunVt); }

// ====== CEK VERSI GAME (return nullptr kalau cocok) ======
static const char* VerifyBuild() {
    auto dos = (IMAGE_DOS_HEADER*)g_base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return "header exe tidak valid";
    auto nt = (IMAGE_NT_HEADERS32*)(g_base + dos->e_lfanew);
    DWORD size = nt->OptionalHeader.SizeOfImage;
    if (size != EXPECTED_IMAGE_SIZE) return "ukuran exe beda (bukan Zuma Deluxe v1.0 yang didukung)";

    uint32_t gunFn = 0, ballFn = 0, ctor = 0;
    if (!ReadU32(g_gunVt, gunFn) || !ReadU32(g_ballVt, ballFn) ||
        !ReadU32(g_base + GUN_CTOR_RVA, ctor))
        return "alamat offset tidak bisa dibaca";
    if (gunFn < g_base || gunFn >= g_base + size) return "vtable gun tidak cocok";
    if (ballFn < g_base || ballFn >= g_base + size) return "vtable ball tidak cocok";
    uint8_t b0 = (uint8_t)(ctor & 0xFF);
    if (b0 == 0x00 || b0 == 0xCC) return "alamat ctor gun bukan kode";
    return nullptr;
}

// ====== LOGIC (dipanggil dengan g_mtx sudah di-lock) ======
static void RefreshAmmo() {
    std::vector<uintptr_t> next;
    for (auto it = g_guns.begin(); it != g_guns.end();) {
        uintptr_t gun = *it;
        if (!IsGun(gun)) { it = g_guns.erase(it); continue; }
        ++it;
        for (uintptr_t off : { GUN_CUR_OFF, GUN_NXT_OFF }) {
            uint32_t p;
            if (!ReadU32(gun + off, p)) continue;
            if (IsBall(p) && std::find(next.begin(), next.end(), p) == next.end())
                next.push_back(p);
        }
    }
    g_ammo = next;

    if (g_mode == MODE_DETECT) {
        for (uintptr_t b : g_ammo) {
            uint32_t c;
            if (ReadU32(b + BALL_COLOR_OFF, c) && c < 64 && g_observed.insert(c).second)
                LOG("[PALET] +warna %u (total: %zu)\n", c, g_observed.size());
        }
    }
}

static void ForceNow() {
    if (g_mode != MODE_LOCK || g_forced < 0) return;
    for (uintptr_t b : g_ammo) {
        if (!IsBall(b)) continue;
        uint32_t c;
        if (!ReadU32(b + BALL_COLOR_OFF, c)) continue;
        if (c >= 64 || c == (uint32_t)g_forced) continue;
        WriteU32(b + BALL_COLOR_OFF, (uint32_t)g_forced);
    }
}

static void LockColor(int c) {
    g_forced = c;
    g_mode = MODE_LOCK;
    RefreshAmmo();
    ForceNow();
}

static void BeginLevel() {
    DWORD now = GetTickCount();
    if (now - g_lastLevelStart < 1500) return;   // debounce
    g_lastLevelStart = now;
    g_observed.clear();

    if (g_fixedColor >= 0) {
        LOG("[LEVEL] level baru/restart (warna tetap).\n");
        LockColor(g_fixedColor);
        LOG("[LOCK] warna %d\n", g_fixedColor);
        return;
    }
    g_mode = MODE_DETECT;
    g_forced = -1;
    g_detectDeadline = now + g_detectMs;
    LOG("[LEVEL] level baru/restart! Deteksi palet %.1fs...\n", g_detectMs / 1000.0);
}

// ====== HOOK KONSTRUKTOR GUN ======
extern "C" void __cdecl OnGunCreated(uintptr_t obj) {
    if (!IsGun(obj)) return;
    std::lock_guard<std::mutex> lk(g_mtx);
    if (g_guns.insert(obj).second)
        LOG("[GUN] #%zu di 0x%08X\n", g_guns.size(), (unsigned)obj);
    BeginLevel();
}

static void* g_origCtor = nullptr;
static uintptr_t g_savedRet = 0, g_savedThis = 0;

__declspec(naked) void GunCtorDetour() {
    __asm {
        pop  eax
        mov  g_savedRet, eax
        mov  g_savedThis, ecx
        push offset AfterOrig
        jmp  dword ptr[g_origCtor]
        AfterOrig:
        push eax
            push g_savedThis
            call OnGunCreated
            add  esp, 4
            pop  eax
            push g_savedRet
            ret
    }
}

// ====== WORKER THREAD ======
static DWORD WINAPI Worker(LPVOID) {
    DWORD lastRefresh = 0;
    for (;;) {
        Sleep(1);
        DWORD now = GetTickCount();
        std::lock_guard<std::mutex> lk(g_mtx);

        if (now - lastRefresh >= 250) { RefreshAmmo(); lastRefresh = now; }

        if (g_mode == MODE_DETECT && g_detectDeadline && now >= g_detectDeadline) {
            RefreshAmmo();
            if (g_observed.empty()) {
                g_detectDeadline = now + 500;
            }
            else {
                std::vector<uint32_t> pal(g_observed.begin(), g_observed.end());
                uint32_t chosen = pal[g_rng() % pal.size()];
                g_detectDeadline = 0;
                LockColor((int)chosen);
                LOG("[LOCK] palet berisi %zu warna -> warna acak %u\n", pal.size(), chosen);
            }
        }
        ForceNow();
    }
}

static DWORD WINAPI Init(LPVOID) {
    char ini[MAX_PATH];
    GetModuleFileNameA(g_hMod, ini, MAX_PATH);
    char* slash = strrchr(ini, '\\');
    if (slash) strcpy_s(slash + 1, ini + MAX_PATH - (slash + 1), "zuma_ammo_lock.ini");

    g_detectMs = GetPrivateProfileIntA("lock", "detect", 6, ini) * 1000;
    g_fixedColor = GetPrivateProfileIntA("lock", "force_color", -1, ini);
    g_console = GetPrivateProfileIntA("lock", "console", 0, ini) != 0;

    if (g_console) {
        AllocConsole();
        FILE* f; freopen_s(&f, "CONOUT$", "w", stdout);
        setvbuf(stdout, nullptr, _IONBF, 0);
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        DWORD m;
        if (GetConsoleMode(hIn, &m))
            SetConsoleMode(hIn, (m & ~ENABLE_QUICK_EDIT_MODE) | ENABLE_EXTENDED_FLAGS);
    }

    g_rng.seed(GetTickCount());
    g_base = (uintptr_t)GetModuleHandleA(nullptr);
    g_gunVt = g_base + GUN_VTABLE_RVA;
    g_ballVt = g_base + BALL_VTABLE_RVA;

    if (const char* why = VerifyBuild()) {
        char msg[256];
        sprintf_s(msg, "Mod tidak dipasang: %s.\nHanya mendukung Zuma Deluxe v1.0.", why);
        MessageBoxA(nullptr, msg, "Zuma Ammo Lock", MB_OK | MB_ICONWARNING | MB_TOPMOST);
        return 1;
    }

    if (MH_Initialize() != MH_OK) { LOG("[ERROR] MH_Initialize gagal\n"); return 1; }
    LPVOID target = (LPVOID)(g_base + GUN_CTOR_RVA);
    if (MH_CreateHook(target, (LPVOID)&GunCtorDetour, &g_origCtor) != MH_OK ||
        MH_EnableHook(target) != MH_OK) {
        MessageBoxA(nullptr, "Gagal memasang hook.", "Zuma Ammo Lock",
            MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }
    LOG("Siap. Masuk level / restart -> warna di-random ulang.\n");

    CreateThread(nullptr, 0, Worker, nullptr, 0, nullptr);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hMod = hMod;
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, Init, nullptr, 0, nullptr);
    }
    return TRUE;
}
