# Zuma Deluxe: One Color Ammo Challenge Mod 🐸

[![ID](https://img.shields.io/badge/Language-Indonesian-red.svg)](#indonesian)
[![EN](https://img.shields.io/badge/Language-English-blue.svg)](#english)

---

## <a name="english"></a>🇬🇧 English Version

**Zuma Deluxe: One Color Ammo** is a challenge mod for Zuma Deluxe (`popcapgame1.exe`) that locks the frog's ammo to a single randomly chosen color every time you start or restart a level. 

This mod was built using Python and Frida, then compiled into a single `.exe` file so you can play it instantly without setting up a programming environment.

### 🌟 Features
*   **Auto-Lock Color:** Every time a new level starts, the mod detects the colors on the map for a few seconds, then randomly selects and locks one color for your ammo.
*   **Customizable Detection Time:** Extend the color detection period using the `--detect` argument.
*   **Force Color:** Manually force a specific color instead of a random one using the `--force-color` argument.
*   **Plug & Play:** Run the compiled `.exe` directly without installing Python or Frida.

### 🎮 How to Use (Crucial Steps)
1.  Open **Zuma Deluxe** and stay at the *Main Menu* or *Map Screen*.
2.  Run `zuma-ammo-lock.exe` (or the Python script). **You MUST run the mod BEFORE entering a level**, as it hooks into the level creation system.
3.  Enter a level. The mod will detect colors (default is 6 seconds) and then lock your ammo to one color.

### 💻 Command Line Arguments (Optional)
If you run the `.exe` or script via Command Prompt, you can use these arguments:
*   `zuma-ammo-lock.exe --detect 8` : Sets the initial color palette detection time to 8 seconds.
*   `zuma-ammo-lock.exe --force-color 2` : Locks the ammo to a specific color ID (skips randomization).

### ⚠️ Disclaimer & Normal Bugs
**Flickering Ammo:** If the ball in the frog's mouth flickers rapidly between colors, **this is NORMAL**. The game engine is trying to load the original color, but the mod is constantly overwriting it with the locked color. Once you shoot the ball, it will strictly remain the locked color.

### ❓ FAQ
*   **Q: Why is the `.exe` file 66 MB if it's just a simple mod?**
    **A:** The `.exe` was compiled using `pyinstaller --onefile`. This means the entire Python runtime and heavy libraries like `Frida` (which handles memory injection) are bundled inside one file for your convenience. If you prefer, you can download the lightweight `zuma-ammo-lock.py` script (around 13 KB) and run it manually using your own Python environment.

    ### 📥 Download Link (For Regular Players)
Don't know how to code? No problem! You don't need to download the source code. Just download the ready-to-play application (EXE) file below:

👉 **[CLICK HERE TO DOWNLOAD THE LATEST RELEASE (.EXE)](https://github.com/halihdaF/Zuma-Deluxe-One-Color-Ammo-Challenge-Mod/releases/latest)** 👈

*(Once you click the link above, look under "Assets" and click on **zuma-ammo-lock.exe** to start downloading).*

---

## <a name="indonesian"></a>🇮🇩 Versi Indonesia

**Zuma Deluxe: One Color Ammo** adalah mod tantangan untuk game Zuma Deluxe (`popcapgame1.exe`) yang mengunci peluru dari mulut kodok menjadi hanya 1 warna secara acak setiap kali kamu berganti level atau me-restart permainan.

Mod ini dibuat menggunakan Python dan Frida, lalu di-*compile* menjadi file `.exe` agar kalian bisa langsung memainkannya tanpa perlu repot menginstal bahasa pemrograman.

### 🌟 Fitur Utama
*   **Auto-Lock Color:** Tiap masuk level baru, mod akan membaca palet warna di layar selama beberapa detik, lalu memilih 1 warna acak untuk dikunci.
*   **Customizable Detection Time:** Waktu deteksi palet warna awal bisa diatur lebih lama menggunakan argumen `--detect`.
*   **Force Color:** Memaksa peluru terkunci ke satu warna spesifik tanpa diacak menggunakan argumen `--force-color`.
*   **Plug & Play:** Tinggal klik file `.exe` dan langsung main tanpa instalasi tambahan.

### 🎮 Cara Penggunaan (Sangat Penting)
1.  Buka game **Zuma Deluxe** terlebih dahulu dan diam di *Main Menu*.
2.  Buka/jalankan `zuma-ammo-lock.exe`. **Mod ini WAJIB dijalankan SEBELUM kalian masuk ke dalam level** karena sistem mendeteksi pergantian level lewat konstruktor game.
3.  Masuk ke level. Mod akan melakukan deteksi warna (*default* 6 detik) lalu amunisi akan terkunci ke satu warna.

### 💻 Command Line Arguments (Opsional)
Jika kamu menjalankan file lewat *Command Prompt* (CMD) atau Terminal, kamu bisa menambahkan argumen ini:
*   `zuma-ammo-lock.exe --detect 8` : Memperpanjang waktu deteksi warna awal menjadi 8 detik.
*   `zuma-ammo-lock.exe --force-color 2` : Mengunci peluru secara paksa ke satu warna tanpa diacak.

### ⚠️ Disclaimer & Bugs
**Peluru Berkedip (Flickering):** Kalau warna peluru di mulut kodok terlihat berkedip atau berganti-ganti warna, **itu NORMAL**. Game masih berusaha memunculkan warna aslinya, namun script mod ini langsung menimpanya. Saat peluru ditembakkan, warnanya dipastikan akan tetap mengikuti warna yang sudah dikunci.

### ❓ FAQ (Pertanyaan yang Sering Diajukan)
*   **Q: Kenapa ukuran file `.exe` mod ini sampai 66 MB padahal fungsinya simpel?**
    **A:** File `.exe` ini di-*compile* menggunakan `pyinstaller --onefile`. Artinya, seluruh sistem *interpreter* Python beserta *library* berat seperti `Frida` (untuk injeksi memori) dibungkus ke dalam satu file agar kamu bisa langsung memainkannya. Jika ragu, kamu selalu bisa mengunduh *source code* aslinya (`zuma-ammo-lock.py`) yang berukuran sekitar 13 KB dan menjalankannya sendiri secara manual.

    ### 📥 Link Download (Untuk Pemain / Orang Awam)
Tidak mengerti bahasa pemrograman? Tenang saja! Kamu tidak perlu *download* file kodingannya. Langsung saja *download* file aplikasi (EXE) yang sudah jadi dan siap main di bawah ini:

👉 **[KLIK DI SINI UNTUK DOWNLOAD VERSI TERBARU (.EXE)](https://github.com/halihdaF/Zuma-Deluxe-One-Color-Ammo-Challenge-Mod/releases/latest)** 👈

*(Setelah masuk ke halaman tautan di atas, cari dan klik file bernama **zuma-ammo-lock.exe** di bawah tulisan Assets untuk mulai mendownload).*
