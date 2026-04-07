# Praktikum Sistem Operasi Modul 2

## Identitas
| Nama | NRP |
|------|-----|
| Dian Piramidiana Rachmatika | 5027251031 |

---

## Soal 1 - Kasbon Warga Kampung Durian Runtuh

### Penjelasan

Program `kasir_muthu.c` mengamankan data buku hutang secara otomatis menggunakan **Sequential Process** dengan kombinasi `fork()`, `exec()`, dan `waitpid()`.

Program ini menjalankan 4 langkah secara berurutan:
1. Membuat folder `brankas_kedai` menggunakan `mkdir`
2. Menyalin `buku_hutang.csv` ke dalam `brankas_kedai` menggunakan `cp`
3. Memfilter data berstatus **"Belum Lunas"** ke file `daftar_penunggak.txt` menggunakan `grep`
4. Mengompres `brankas_kedai` menjadi `rahasia_muthu.zip` menggunakan `zip`

Setiap langkah dijalankan oleh **child process** menggunakan `fork()` dan `execv()`, sementara **parent process** menunggu child selesai menggunakan `waitpid()`. Jika ada langkah yang gagal, program langsung berhenti dan mencetak pesan error.

### Cara Menjalankan
```bash
gcc kasir_muthu.c -o kasir_muthu
./kasir_muthu
```

### Dokumentasi
[screenshot output soal 1]

---

## Soal 2 - The world never stops, even when you feel tired

### Penjelasan

Program `contract_daemon.c` adalah **daemon process** yang berjalan di background secara terus menerus untuk memantau file `contract.txt`.

Cara kerja daemon:
1. Saat pertama dijalankan, membuat `contract.txt` berisi teks dan timestamp
2. Setiap **5 detik** menulis `still working... [status acak]` ke `work.log`
3. Jika `contract.txt` **dihapus**, daemon membuat ulang file tersebut dalam 1-2 detik
4. Jika `contract.txt` **diubah isinya**, daemon menulis `contract violated.` ke log dan restore file
5. Saat daemon **dihentikan**, menulis `We really weren't meant to be together` ke log

### Cara Menjalankan
```bash
gcc contract_daemon.c -o contract_daemon
./contract_daemon

# Cek daemon berjalan
ps aux | grep contract_daemon

# Matikan daemon
kill $(ps aux | grep contract_daemon | grep -v grep | awk '{print $2}')
```

### Dokumentasi
[screenshot output soal 2]

---

## Soal 3 - One letter for destiny

### Penjelasan

Program `angel.c` adalah **daemon process** dengan nama proses `maya` yang memiliki beberapa fitur:

- **Fitur secret**: Setiap 10 detik mengganti isi `LoveLetter.txt` dengan kalimat acak dari 4 pilihan
- **Fitur surprise**: Setelah fitur secret berjalan, mengenkripsi isi `LoveLetter.txt` menggunakan **Base64**
- **Fitur decrypt**: Mendekripsi isi `LoveLetter.txt` dan menampilkan isi aslinya
- **Fitur kill**: Menghentikan daemon yang sedang berjalan
- **Fitur logging**: Semua aktivitas dicatat ke `ethereal.log` dengan format `[dd:mm:yyyy]-[hh:mm:ss]_proses_STATUS`

### Cara Menjalankan
```bash
gcc angel.c -o angel

# Jalankan daemon
./angel -daemon

# Decrypt LoveLetter.txt
./angel -decrypt

# Matikan daemon
./angel -kill
```

### Dokumentasi
[screenshot output soal 3]

---
