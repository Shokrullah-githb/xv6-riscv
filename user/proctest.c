#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int pid;

  printf("Creating child processes...\n");

  // Create three child processes.
  for (int i = 0; i < 3; i++) {
    pid = fork();

    if (pid < 0) {
      printf("fork failed\n");
      exit(1);
    }

    if (pid == 0) {
      sleep(100);
      exit(0);
    }
  }

  // Wait before collecting process information.
  sleep(10);

  struct pinfo info[64];

  // Test 1: Normal case (max = 64).
  int count = getprocs(info, 64);

  printf("\nTest 1: max = 64\n");
  printf("Processes found: %d\n", count);
  printf("PID\tPPID\tSTATE\tNAME\n");

  for (int i = 0; i < count; i++) {
    printf("%d\t%d\t%d\t%s\n",
           info[i].pid,
           info[i].ppid,
           info[i].state,
           info[i].name);
  }

  // Test 2: Small buffer (max = 2).
  int small_count = getprocs(info, 2);

  printf("\nTest 2: max = 2\n");
  printf("Processes found: %d\n", small_count);

  // Test 3: Invalid max value.
  int invalid_max = getprocs(info, 0);

  printf("\nTest 3: max = 0\n");
  printf("Return value: %d\n", invalid_max);

  // Test 4: Invalid buffer address.
  int invalid_buffer = getprocs((struct pinfo *)0, 64);

  printf("\nTest 4: buffer = 0\n");
  printf("Return value: %d\n", invalid_buffer);

  // Wait for all child processes.
  for (int i = 0; i < 3; i++) {
    wait(0);
  }

  exit(0);
}
