#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void run(char *path, char *argv[]) {
pid_t pid = fork ();
if (pid < 0) {
printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
exit(1);
}
if (pid == 0) {
execv(path, argv);
printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
exit(1);
}
int status;
waitpid(pid, &status, 0);
if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
exit(1);
}
}
int main () {
char *a1[] = {"/bin/mkdir", "-p", "brankas_kedai", NULL};
run("/bin/mkdir",a1);

char *a2[] = {"/bin/cp", "buku_hutang.csv", "brankas_kedai/", NULL};
run("/bin/cp", a2);

pid_t pid = fork ();
if (pid < 0) {
printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
exit(1);
}

if (pid == 0) {
execlp("bash", "bash", "-c",
	"grep \"Belum Lunas\" brankas_kedai/buku_hutang.csv > brankas_kedai/daftar_penunggak.txt", NULL);
printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
exit(1);
}

int status;
waitpid(pid, &status, 0);
if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
exit(1);
}

char *a4[] = {"/usr/bin/zip", "-r", "rahasia_muthu.zip", "brankas_kedai", NULL};
run("/usr/bin/zip", a4);

printf ("[INFO] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.\n");
return 0;
} 
