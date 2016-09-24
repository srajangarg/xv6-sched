#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "forking ...\n");

  int pid = fork();
  int stime, ttime;
  int temp = 0;
  int prio = 3;

  if(pid < 0) {
    printf(1, "fork failed!\n");
    exit();
  }

  stime = uptime();

  if(pid != 0) {
    setprio(prio);
  }

  if(pid == 0) {
    printf(1, "child  priority : %d\n", getprio());
  } else {
    printf(1, "parent priority : %d\n", getprio());
    printf(1, "expected ratio of finishing times : %d/%d\n", 2*prio, prio + 1);
  }


  for (int j = 0; j < 5; ++j)
    for (int i = 0; i < (1<<29); ++i)
    	temp += i;

  ttime = uptime() - stime;
  if(pid == 0) {
    printf(1, "child finsihed after %d ticks, got %d\n", ttime, temp);
  } else {
    printf(1, "parent finsihed after %d ticks, got %d\n", ttime, temp);
    wait();
  }

  exit();
}