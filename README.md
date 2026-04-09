# Praktikum Sistem Operasi Modul 2

## Identitas
| Nama | NRP |
|------|-----|
| Dian Piramidiana Rachmatika | 5027251031 |

---

## Soal 1 - Kasbon Warga Kampung Durian Runtuh

### Penjelasan

Program `kasir_muthu.c` mengamankan data buku hutang secara otomatis menggunakan **Sequential Process** dengan kombinasi `fork()`, `exec()`, dan `waitpid()`.

Program menjalankan 4 langkah secara berurutan:
1. Membuat folder `brankas_kedai` menggunakan `mkdir`
2. Menyalin `buku_hutang.csv` ke dalam `brankas_kedai` menggunakan `cp`
3. Memfilter data berstatus **"Belum Lunas"** ke file `daftar_penunggak.txt` menggunakan `grep`
4. Mengompres `brankas_kedai` menjadi `rahasia_muthu.zip` menggunakan `zip`

Setiap langkah dijalankan oleh **child process (Ipin)** menggunakan `fork()` dan `execv()`, sementara **parent process (Upin)** menunggu child selesai menggunakan `waitpid()` sebelum melanjutkan ke langkah berikutnya. Jika ada langkah yang gagal, program langsung berhenti dan mencetak pesan error.

### Kode Program

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Fungsi helper untuk menjalankan satu child process
// path  = lokasi program yang akan dijalankan (misal /bin/mkdir)
// argv  = argumen program tersebut
void run(char *path, char *argv[]) {
    pid_t pid = fork(); // duplikasi proses → parent + child

    if (pid < 0) {
        // fork() gagal
        printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
        exit(1);
    }
    if (pid == 0) {
        // INI CHILD PROCESS
        // execv menggantikan child dengan program yang diminta
        execv(path, argv);
        // Kalau sampai sini berarti execv gagal
        printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
        exit(1);
    }
    // INI PARENT PROCESS
    // Tunggu child selesai sebelum lanjut ke langkah berikutnya
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
        exit(1);
    }
}

int main() {
    // LANGKAH 1: Buat folder brankas_kedai
    // Sama seperti mengetik: mkdir -p brankas_kedai
    char *a1[] = {"/bin/mkdir", "-p", "brankas_kedai", NULL};
    run("/bin/mkdir", a1);

    // LANGKAH 2: Salin buku_hutang.csv ke brankas_kedai
    // Sama seperti mengetik: cp buku_hutang.csv brankas_kedai/
    char *a2[] = {"/bin/cp", "buku_hutang.csv", "brankas_kedai/", NULL};
    run("/bin/cp", a2);

    // LANGKAH 3: Filter "Belum Lunas" → daftar_penunggak.txt
    // Pakai bash -c karena butuh redirect ">" yang merupakan fitur shell
    // Sama seperti mengetik: grep "Belum Lunas" ... > daftar_penunggak.txt
    pid_t pid = fork();
    if (pid < 0) {
        printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
        exit(1);
    }
    if (pid == 0) {
        execlp("bash", "bash", "-c",
            "grep \"Belum Lunas\" brankas_kedai/buku_hutang.csv > brankas_kedai/daftar_penunggak.txt",
            NULL);
        printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
        exit(1);
    }

    // LANGKAH 4: Kompres brankas_kedai → rahasia_muthu.zip
    // Sama seperti mengetik: zip -r rahasia_muthu.zip brankas_kedai
    char *a4[] = {"/usr/bin/zip", "-r", "rahasia_muthu.zip", "brankas_kedai", NULL};
    run("/usr/bin/zip", a4);

    printf("[INFO] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.\n");
    return 0;
}
```

### Cara Menjalankan
```bash
# Compile program
gcc kasir_muthu.c -o kasir_muthu

# Jalankan program
./kasir_muthu
```

### Dokumentasi

**Screenshot 1 - Output saat berhasil**

Saat `./kasir_muthu` dijalankan dan semua langkah berhasil, akan muncul pesan sukses di terminal.

![SS1](assets/soal_1/ss1.png)

**Screenshot 2 - Isi daftar_penunggak.txt**

File `daftar_penunggak.txt` berisi semua data pelanggan yang berstatus "Belum Lunas" hasil filter `grep`.

![SS2](assets/soal_1/ss2.png)

**Screenshot 3 - Output saat error**

Saat `buku_hutang.csv` dihapus lalu program dijalankan, program mendeteksi file tidak ada dan langsung berhenti dengan pesan error.

![SS3](assets/soal_1/ss3.png)

---

## Soal 2 - The world never stops, even when you feel tired

### Penjelasan

Program `contract_daemon.c` adalah **daemon process** yang berjalan di background secara terus menerus untuk memantau file `contract.txt`.

Cara kerja daemon:
1. Saat pertama dijalankan, membuat `contract.txt` berisi teks dan timestamp `created at`
2. Setiap **5 detik** menulis `still working... [status acak]` ke `work.log`
3. Jika `contract.txt` **dihapus**, daemon membuat ulang file dalam 1-2 detik dengan timestamp `restored at`
4. Jika `contract.txt` **diubah isinya**, daemon menulis `contract violated.` ke log dan restore file
5. Saat daemon **dihentikan**, menulis `We really weren't meant to be together` ke log

### Kode Program

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

#define FILE_KONTRAK "contract.txt"
#define FILE_LOG "work.log"

// Catat pesan ke work.log dengan mode append
void catat_log(char *pesan) {
    FILE *f = fopen(FILE_LOG, "a");
    if (f) { fprintf(f, "%s\n", pesan); fclose(f); }
}

// Ambil waktu sekarang dalam format YYYY-MM-DD HH:MM:SS
char* ambil_waktu() {
    static char buf[64];
    time_t t = time(NULL);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buf;
}

// Buat contract.txt
// dipulihkan=0 → "created at", dipulihkan=1 → "restored at"
void tulis_kontrak(int dipulihkan) {
    FILE *f = fopen(FILE_KONTRAK, "w");
    if (f) {
        fprintf(f, "\"A promise to keep going, even when unseen.\"\n\n");
        fprintf(f, "%s at: %s\n", dipulihkan ? "restored" : "created", ambil_waktu());
        fclose(f);
    }
}

// Simpan isi contract.txt ke variabel global untuk deteksi perubahan
char isi_lama[1024];
void simpan_isi_kontrak() {
    FILE *f = fopen(FILE_KONTRAK, "r");
    if (f) {
        int n = fread(isi_lama, 1, sizeof(isi_lama)-1, f);
        isi_lama[n] = '\0';
        fclose(f);
    }
}

// Handler saat daemon dimatikan dengan kill
void tangkap_sigterm(int sig) {
    catat_log("We really weren't meant to be together");
    exit(0);
}

int main() {
    // LANGKAH 1: Jadikan daemon dengan fork()
    // Parent langsung exit → terminal bebas dipakai
    // Child terus jalan di background sebagai daemon
    pid_t pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0); // parent exit

    // LANGKAH 2: Setup daemon
    umask(0);    // izin penuh untuk file yang dibuat daemon
    setsid();    // buat session baru, lepas dari terminal

    // LANGKAH 3: Pindah ke folder soal_2
    chdir("/home/piramidiana/SISOP-2-2026-IT_5027251031/soal_2");

    // LANGKAH 4: Tutup stdin, stdout, stderr
    // Daemon tidak boleh pakai terminal
    close(0); close(1); close(2);

    // Tangkap sinyal SIGTERM (saat di-kill)
    signal(SIGTERM, tangkap_sigterm);

    // Buat contract.txt pertama kali
    tulis_kontrak(0);
    simpan_isi_kontrak();

    char *kondisi[] = {"[awake]", "[drifting]", "[numbness]"};
    srand(time(NULL));
    int detik = 0;

    while (1) {
        sleep(1);
        detik++;

        // Cek apakah contract.txt ada
        if (access(FILE_KONTRAK, F_OK) != 0) {
            // File dihapus! Buat ulang
            tulis_kontrak(1);
            simpan_isi_kontrak();
        } else {
            // File ada, cek apakah isinya berubah
            char isi_baru[1024];
            FILE *f = fopen(FILE_KONTRAK, "r");
            if (f) {
                int n = fread(isi_baru, 1, sizeof(isi_baru)-1, f);
                isi_baru[n] = '\0';
                fclose(f);
            }
            if (strcmp(isi_lama, isi_baru) != 0) {
                // Isi berubah!
                catat_log("contract violated.");
                tulis_kontrak(1);
                simpan_isi_kontrak();
            }
        }

        // Setiap 5 detik tulis log
        if (detik % 5 == 0) {
            char pesan[64];
            snprintf(pesan, sizeof(pesan), "still working... %s", kondisi[rand() % 3]);
            catat_log(pesan);
        }
    }
    return 0;
}
```

### Cara Menjalankan
```bash
# Compile program
gcc contract_daemon.c -o contract_daemon

# Jalankan daemon
./contract_daemon

# Cek daemon berjalan
ps aux | grep contract_daemon

# Matikan daemon
kill $(ps aux | grep contract_daemon | grep -v grep | awk '{print $2}')
```

### Dokumentasi

**Screenshot 1 - Daemon berjalan**

Setelah `./contract_daemon` dijalankan, terminal langsung kembali ke prompt karena daemon berjalan di background. Perintah `ps aux | grep contract_daemon` membuktikan daemon sedang berjalan.

![SS1](assets/soal_2/ss1.png)

**Screenshot 2 - Isi contract.txt**

File `contract.txt` dibuat otomatis oleh daemon saat pertama kali dijalankan.

![SS2](assets/soal_2/ss2.png)

**Screenshot 3 - Isi work.log (3 screenshot)**

Output sangat panjang karena daemon lama dari sesi sebelumnya masih berjalan dan belum dimatikan sehingga log sudah menumpuk.

![SS3_1](assets/soal_2/ss3_1.png)
![SS3_2](assets/soal_2/ss3_2.png)
![SS3_3](assets/soal_2/ss3_3.png)

**Screenshot 4 - Test hapus contract.txt**

Perintah `rm contract.txt` menghapus file. Setelah `sleep 2`, file dibuat ulang otomatis dengan timestamp `restored at`.

![SS4](assets/soal_2/ss4.png)

**Screenshot 5 - Test ubah contract.txt (2 screenshot)**

Perintah `echo "hacked" > contract.txt` mengubah isi file. Daemon mendeteksi perubahan dan menulis `contract violated.` ke work.log.

![SS5_1](assets/soal_2/ss5_1.png)
![SS5_2](assets/soal_2/ss5_2.png)

**Screenshot 6 - Matikan daemon (2 screenshot)**

Perintah `kill $(ps aux | grep contract_daemon | grep -v grep | awk '{print $2}')` menghentikan daemon. Setelah dimatikan, `We really weren't meant to be together` muncul di baris terakhir work.log.

![SS6_1](assets/soal_2/ss6_1.png)
![SS6_2](assets/soal_2/ss6_2.png)

---

## Soal 3 - One letter for destiny

### Penjelasan

Program `angel.c` adalah **daemon process** dengan nama proses `maya` yang memiliki beberapa fitur:

- **Fitur secret**: Setiap 10 detik mengganti isi `LoveLetter.txt` dengan kalimat acak dari 4 pilihan
- **Fitur surprise**: Setelah fitur secret berjalan, mengenkripsi isi `LoveLetter.txt` menggunakan **Base64**
- **Fitur decrypt**: Mendekripsi isi `LoveLetter.txt` dan menampilkan isi aslinya
- **Fitur kill**: Menghentikan daemon menggunakan PID yang tersimpan di `/tmp/angel.pid`
- **Fitur logging**: Semua aktivitas dicatat ke `ethereal.log` dengan format `[dd:mm:yyyy]-[hh:mm:ss]_proses_STATUS`

### Kode Program

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOVELETTER "LoveLetter.txt"
#define LOGFILE "ethereal.log"
#define PIDFILE "/tmp/angel.pid"

// 4 kalimat yang akan dipilih acak oleh fitur secret
char *messages[] = {
    "aku akan fokus pada diriku sendiri",
    "aku mencintaimu dari sekarang hingga selamanya",
    "aku akan menjauh darimu, hingga takdir mempertemukan kita di versi kita yang terbaik.",
    "kalau aku dilahirkan kembali, aku tetap akan terus menyayangimu"
};

// Ambil waktu sekarang dalam format dd:mm:yyyy]-[hh:mm:ss
void fetch_time(char *buf, int size) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, size, "%d:%m:%Y]-[%H:%M:%S", tm);
}

// Catat aktivitas ke ethereal.log
// Format: [dd:mm:yyyy]-[hh:mm:ss]_proses_STATUS
void append_log(char *process, char *status) {
    FILE *f = fopen(LOGFILE, "a");
    if (f) {
        char ts[64];
        fetch_time(ts, sizeof(ts));
        fprintf(f, "[%s]_%s_%s\n", ts, process, status);
        fclose(f);
    }
}

// Enkripsi teks ke Base64
void encode_base64(char *input, char *output) {
    const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0, j = 0;
    int len = strlen(input);
    unsigned char a, b, c;
    while (i < len) {
        a = i < len ? input[i++] : 0;
        b = i < len ? input[i++] : 0;
        c = i < len ? input[i++] : 0;
        output[j++] = table[a >> 2];
        output[j++] = table[((a & 3) << 4) | (b >> 4)];
        output[j++] = (i - 1 < len) ? table[((b & 15) << 2) | (c >> 6)] : '=';
        output[j++] = (i < len || (i-1 < len && i > 1)) ? table[c & 63] : '=';
    }
    output[j] = '\0';
}

// Dekripsi Base64 ke teks asli
void decode_base64(char *input, char *output) {
    const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0, j = 0;
    int len = strlen(input);
    unsigned char a, b, c, d;
    while (i < len) {
        a = strchr(table, input[i++]) - table;
        b = strchr(table, input[i++]) - table;
        c = input[i] == '=' ? (i++, 0) : strchr(table, input[i++]) - table;
        d = input[i] == '=' ? (i++, 0) : strchr(table, input[i++]) - table;
        output[j++] = (a << 2) | (b >> 4);
        if (input[i-2] != '=') output[j++] = ((b & 15) << 4) | (c >> 2);
        if (input[i-1] != '=') output[j++] = ((c & 3) << 6) | d;
    }
    output[j] = '\0';
}

// Handler saat daemon dimatikan dengan kill
void catch_signal(int sig) {
    append_log("kill", "SUCCESS");
    remove(PIDFILE);
    exit(0);
}

// Jadikan program sebagai daemon
void start_daemon(char *argv[]) {
    // Cek apakah daemon sudah berjalan
    FILE *pf = fopen(PIDFILE, "r");
    if (pf) {
        printf("Daemon sudah berjalan!\n");
        fclose(pf);
        exit(1);
    }

    append_log("daemon", "RUNNING");

    pid_t pid = fork();
    if (pid < 0) { append_log("daemon", "ERROR"); exit(1); }
    if (pid > 0) { append_log("daemon", "SUCCESS"); exit(0); } // parent exit

    umask(0);
    setsid(); // buat session baru

    // Ubah nama proses menjadi "maya"
    strncpy(argv[0], "maya", strlen(argv[0]));

    // Pindah ke folder soal_3
    char dir[256];
    snprintf(dir, sizeof(dir), "/home/%s/SISOP-2-2026-IT_5027251031/soal_3", getenv("USER"));
    chdir(dir);

    // Simpan PID ke file agar bisa di-kill nanti
    FILE *f = fopen(PIDFILE, "w");
    if (f) { fprintf(f, "%d\n", getpid()); fclose(f); }

    close(0); close(1); close(2);
    signal(SIGTERM, catch_signal);

    srand(time(NULL));
    int counter = 0;

    while (1) {
        sleep(1);
        counter++;

        if (counter % 10 == 0) {
            // FITUR SECRET: tulis kalimat acak ke LoveLetter.txt
            append_log("secret", "RUNNING");
            char *content = messages[rand() % 4];
            FILE *fl = fopen(LOVELETTER, "w");
            if (fl) { fprintf(fl, "%s\n", content); fclose(fl); }
            append_log("secret", "SUCCESS");

            // FITUR SURPRISE: enkripsi LoveLetter.txt dengan Base64
            append_log("surprise", "RUNNING");
            FILE *fr = fopen(LOVELETTER, "r");
            if (fr) {
                char buf[1024] = {0};
                fread(buf, 1, sizeof(buf)-1, fr);
                fclose(fr);
                char encoded[4096] = {0};
                encode_base64(buf, encoded);
                FILE *fw = fopen(LOVELETTER, "w");
                if (fw) { fprintf(fw, "%s", encoded); fclose(fw); }
            }
            append_log("surprise", "SUCCESS");
        }
    }
}

// Dekripsi LoveLetter.txt dan tampilkan isi aslinya
void run_decrypt() {
    append_log("decrypt", "RUNNING");
    FILE *f = fopen(LOVELETTER, "r");
    if (!f) {
        printf("Error: LoveLetter.txt tidak ditemukan!\n");
        append_log("decrypt", "ERROR");
        exit(1);
    }
    char buf[4096] = {0};
    fread(buf, 1, sizeof(buf)-1, f);
    fclose(f);
    char decoded[4096] = {0};
    decode_base64(buf, decoded);
    printf("%s\n", decoded);
    append_log("decrypt", "SUCCESS");
}

// Hentikan daemon menggunakan PID dari PIDFILE
void stop_daemon() {
    append_log("kill", "RUNNING");
    FILE *f = fopen(PIDFILE, "r");
    if (!f) {
        printf("Error: Daemon belum berjalan!\n");
        append_log("kill", "ERROR");
        exit(1);
    }
    int pid;
    fscanf(f, "%d", &pid);
    fclose(f);
    kill(pid, SIGTERM);
    printf("Daemon berhasil dihentikan!\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Penggunaan:\n");
        printf("  ./angel -daemon  : jalankan sebagai daemon (nama proses: maya)\n");
        printf("  ./angel -decrypt : decrypt LoveLetter.txt\n");
        printf("  ./angel -kill    : kill proses\n");
        return 0;
    }
    if (strcmp(argv[1], "-daemon") == 0) {
        start_daemon(argv);
    } else if (strcmp(argv[1], "-decrypt") == 0) {
        run_decrypt();
    } else if (strcmp(argv[1], "-kill") == 0) {
        stop_daemon();
    } else {
        printf("Penggunaan:\n");
        printf("  ./angel -daemon  : jalankan sebagai daemon (nama proses: maya)\n");
        printf("  ./angel -decrypt : decrypt LoveLetter.txt\n");
        printf("  ./angel -kill    : kill proses\n");
    }
    return 0;
}
```

### Cara Menjalankan
```bash
# Compile program
gcc angel.c -o angel

# Jalankan daemon (nama proses: maya)
./angel -daemon

# Cek daemon berjalan
ps aux | grep maya

# Decrypt LoveLetter.txt
./angel -decrypt

# Matikan daemon
./angel -kill
```

### Dokumentasi

**Screenshot 1 - Daemon berjalan dengan nama maya**

Setelah `./angel -daemon` dijalankan, `ps aux | grep maya` membuktikan daemon berjalan dengan nama proses `maya`.

![SS1](assets/soal_3/ss1.png)

**Screenshot 2 - Isi LoveLetter.txt terenkripsi**

Setelah 10 detik, fitur secret menulis kalimat acak dan fitur surprise langsung mengenkripsinya dengan Base64.

![SS2](assets/soal_3/ss2.png)

**Screenshot 3 - Hasil decrypt**

Perintah `./angel -decrypt` mendekripsi isi LoveLetter.txt dan menampilkan kalimat aslinya.

![SS3](assets/soal_3/ss3.png)

**Screenshot 4 - Isi ethereal.log**

File `ethereal.log` mencatat semua aktivitas daemon. Output panjang karena daemon lupa dimatikan dari sesi sebelumnya.

![SS4](assets/soal_3/ss4.png)

**Screenshot 5 - Matikan daemon**

Perintah `./angel -kill` menghentikan daemon. `kill_SUCCESS` muncul di baris terakhir ethereal.log.

![SS5](assets/soal_3/ss5.png)
