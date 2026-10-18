# Zuma Deluxe: One Color Ammo Challenge Mod 🐸

[![ID](https://img.shields.io/badge/Language-Indonesian-red.svg)](#indonesian)
[![EN](https://img.shields.io/badge/Language-English-blue.svg)](#english)

---

## <a name="english"></a>🇬🇧 English Version

**Zuma One Color Ammo** is a challenge mod for **Zuma Deluxe v1.0**. It locks the frog's ammo to a single randomly chosen color. The color is selected from the available palette in the current level and is re-rolled every time you start a new level or restart the current one.

### 📥 Download (Ready to Play)
👉 **[CLICK HERE TO DOWNLOAD THE LATEST RELEASE (.ZIP)](https://github.com/halihdaF/Zuma-Deluxe-One-Color-Ammo-Challenge-Mod/releases/latest)** 👈

### 🎮 How to Use
1. Download the `.zip` file from the Releases page and extract it.
2. You will find 3 files. Copy **all three files** and paste them into your Zuma Deluxe game folder (place them right next to `Zuma-windowed.exe` or `popcapgame1.exe`).
3. Double-click `ZumaAmmoLock.exe`. The game will automatically open with the mod activated.

### ⚙️ Configuration
You can customize the mod's behavior by editing the `zuma_ammo_lock.ini` file using Notepad. 
*   You can set a fixed color instead of a random one.
*   You can adjust the initial color detection time.

### ⚠️ Important Notes & Disclaimers
*   **Version Compatibility:** This mod *only* supports **Zuma Deluxe v1.0**. If you use a different version, the mod will display a message and safely deactivate itself without modifying the game.
*   **Flickering Ammo:** If the ball in the frog's mouth flickers rapidly, this is **normal**. Once shot, it will strictly remain the locked color.
*   **Antivirus False Positives:** Your antivirus might flag the launcher as a threat. This is a false positive because the launcher uses standard DLL injection techniques to load the mod. The full source code is available in this repository for verification.
*   **Offline Only:** This is for single-player offline use. This repository does **not** contain any original game files or pirated content.

### 🛠️ Build from Source
If you want to compile the mod yourself, use **Visual Studio**:
*   **Configuration:** `x86 (Win32)`, `Release`
*   **Runtime Library:** Multi-threaded (`/MT`)
*   **Projects:**
    *   `zuma_ammo_lock.cpp`: The main DLL project (requires linking `libMinHook.x86.lib`).
    *   `launcher.cpp`: Windows application project (Subsystem: Windows) that outputs `ZumaAmmoLock.exe`.

### ⚖️ License
This project utilizes [MinHook](https://github.com/TsudaKageyu/minhook), which is licensed under the BSD 2-Clause License. The license is included in this repository.

### 🤝 Contribute!
For reverse engineers and C++ modders: If you can patch the original assembly permanently to eliminate the need for a DLL injector launcher, your Pull Request is highly anticipated!

---

## <a name="indonesian"></a>🇮🇩 Versi Indonesia

**Zuma One Color Ammo** adalah mod tantangan untuk **Zuma Deluxe v1.0** yang mengunci peluru kodok menjadi hanya 1 warna. Warna dipilih secara acak berdasarkan warna bola yang ada di level tersebut, dan akan diacak ulang setiap kali kamu masuk level baru atau me-restart permainan.

### 📥 Link Download (Siap Main)
👉 **[KLIK DI SINI UNTUK DOWNLOAD VERSI TERBARU (.ZIP)](https://github.com/halihdaF/Zuma-Deluxe-One-Color-Ammo-Challenge-Mod/releases/latest)** 👈

### 🎮 Cara Penggunaan
1. Download file `.zip` dari halaman *Releases* lalu ekstrak.
2. Kamu akan menemukan 3 buah file. Pindahkan **ketiga file tersebut** ke dalam folder game Zuma Deluxe kamu (taruh bersebelahan persis dengan file `Zuma-windowed.exe` atau `popcapgame1.exe`).
3. Klik dua kali pada file `ZumaAmmoLock.exe`. Game akan otomatis terbuka dengan mod yang sudah aktif.

### ⚙️ Konfigurasi (Opsional)
Kamu bisa mengatur mod ini dengan membuka file `zuma_ammo_lock.ini` menggunakan Notepad.
*   Bisa mengatur warna tetap (tidak diacak).
*   Bisa mengatur durasi waktu deteksi palet warna.

### ⚠️ Catatan Penting
*   **Hanya untuk v1.0:** Mod ini hanya mendukung **Zuma Deluxe v1.0**. Jika dijalankan di versi lain, mod akan menampilkan pesan peringatan dan mati otomatis (tidak akan memodifikasi file game).
*   **Peluru Berkedip:** Jika warna peluru di mulut kodok terlihat berkedip sebentar, itu adalah hal yang **normal**. Saat ditembakkan, warnanya tetap sesuai yang dikunci.
*   **Peringatan Antivirus (False Positive):** Antivirus kamu mungkin mencurigai file ini. Ini normal karena program *launcher* menggunakan teknik injeksi DLL. *Source code* lengkap tersedia di *repository* ini transparan 100%.
*   **Single-player:** Mod ini murni untuk dimainkan *offline*. *Repository* ini **tidak** menyediakan file asli game Zuma Deluxe.

### 🛠️ Build dari Source Code
Untuk kamu yang ingin meng-*compile* sendiri menggunakan **Visual Studio**:
*   **Pengaturan:** Platform `x86 (Win32)`, Mode `Release`
*   **Runtime Library:** Multi-threaded (`/MT`)
*   **Struktur Project:**
    *   `zuma_ammo_lock.cpp`: Project DLL (wajib me-*link* `libMinHook.x86.lib`).
    *   `launcher.cpp`: Project aplikasi Windows (Subsystem: Windows) untuk menghasilkan `ZumaAmmoLock.exe`.

### ⚖️ Lisensi
Mod ini menggunakan *library* [MinHook](https://github.com/TsudaKageyu/minhook) (Lisensi BSD 2-Clause). File lisensinya disertakan di dalam *repository* ini.

### 🤝 Mari Berkontribusi!
Tantangan khusus untuk para *modder*: Kalau ada yang berhasil mengubah (*patching*) *assembly* di file game aslinya secara permanen sehingga kita tidak butuh *launcher* injeksi lagi, *Pull Request* kalian sangat ditunggu!
