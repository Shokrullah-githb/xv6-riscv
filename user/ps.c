#include "kernel/types.h"
#include "user/user.h"

#define MAX_PROCS 64

char *
state_name(int state)
{
  switch (state) {
  case 1: return "USED";
  case 2: return "SLEEPING";
  case 3: return "RUNNABLE";
  case 4: return "RUNNING";
  case 5: return "ZOMBIE";
  default: return "UNKNOWN";
  }
}

int
main(void)
{
  struct pinfo info[MAX_PROCS];

  int count = getprocs(info, MAX_PROCS);

  if (count < 0) {
    printf("ps: getprocs failed\n");
    exit(1);
  }

  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");

  for (int i = 0; i < count; i++) {
    printf("%d\t%d\t%s\t%ld\t%s\n",
           info[i].pid,
           info[i].ppid,
           state_name(info[i].state),
           info[i].sz,
           info[i].name);
  }

  exit(0);
}
