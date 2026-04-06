#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

#define FILE_KONTRAK "contract.txt"
#define FILE_LOG "work.log"

void catat_log(char *pesan) {
    FILE *f = fopen(FILE_LOG, "a");
    if (f) { fprintf(f, "%s\n", pesan); fclose(f); }
}

char* ambil_waktu() {
    static char buf[64];
    time_t t = time(NULL);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buf;
}

void tulis_kontrak(int dipulihkan) {
    FILE *f = fopen(FILE_KONTRAK, "w");
    if (f) {
        fprintf(f, "\"A promise to keep going, even when unseen.\"\n\n");
        fprintf(f, "%s at: %s\n", dipulihkan ? "restored" : "created", ambil_waktu());
        fclose(f);
    }
}

char isi_lama[1024];

void simpan_isi_kontrak() {
    FILE *f = fopen(FILE_KONTRAK, "r");
    if (f) {
        int n = fread(isi_lama, 1, sizeof(isi_lama)-1, f);
        isi_lama[n] = '\0';
        fclose(f);
    }
}

void tangkap_sigterm(int sig) {
    catat_log("We really weren't meant to be together");
    exit(0);
}

int main() {
    // Jadikan daemon
    pid_t pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);

    umask(0);
    setsid();
    chdir("/home/piramidiana/SISOP-2-2026-IT_5027251031/soal_2");
    close(0); close(1); close(2);

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
            tulis_kontrak(1);
            simpan_isi_kontrak();
        } else {
            // Cek apakah isinya berubah
            char isi_baru[1024];
            FILE *f = fopen(FILE_KONTRAK, "r");
            if (f) {
                int n = fread(isi_baru, 1, sizeof(isi_baru)-1, f);
                isi_baru[n] = '\0';
                fclose(f);
            }
            if (strcmp(isi_lama, isi_baru) != 0) {
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
