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

char *messages[] = {
    "aku akan fokus pada diriku sendiri",
    "aku mencintaimu dari sekarang hingga selamanya",
    "aku akan menjauh darimu, hingga takdir mempertemukan kita di versi kita yang terbaik.",
    "kalau aku dilahirkan kembali, aku tetap akan terus menyayangimu"
};

void fetch_time(char *buf, int size) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, size, "%d:%m:%Y]-[%H:%M:%S", tm);
}

void append_log(char *process, char *status) {
    FILE *f = fopen(LOGFILE, "a");
    if (f) {
        char ts[64];
        fetch_time(ts, sizeof(ts));
        fprintf(f, "[%s]_%s_%s\n", ts, process, status);
        fclose(f);
    }
}

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

void catch_signal(int sig) {
    append_log("kill", "SUCCESS");
    remove(PIDFILE);
    exit(0);
}

void start_daemon(char *argv[]) {
    FILE *pf = fopen(PIDFILE, "r");
    if (pf) {
        printf("Daemon sudah berjalan!\n");
        fclose(pf);
        exit(1);
    }

    append_log("daemon", "RUNNING");

    pid_t pid = fork();
    if (pid < 0) { append_log("daemon", "ERROR"); exit(1); }
    if (pid > 0) { append_log("daemon", "SUCCESS"); exit(0); }

    umask(0);
    setsid();
    strncpy(argv[0], "maya", strlen(argv[0]));


    char dir[256];
    snprintf(dir, sizeof(dir), "/home/%s/SISOP-2-2026-IT_5027251031/soal_3", getenv("USER"));
    chdir(dir);

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
            // Fitur secret
            append_log("secret", "RUNNING");
            char *content = messages[rand() % 4];
            FILE *fl = fopen(LOVELETTER, "w");
            if (fl) { fprintf(fl, "%s\n", content); fclose(fl); }
            append_log("secret", "SUCCESS");

            // Fitur surprise
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
