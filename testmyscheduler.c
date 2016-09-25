#include "types.h"
#include "user.h"

void
cpuintensive(int index)
{
  int stime = uptime();
  int temp = 0;
  for (int i = 0; i < (1<<29); ++i)
    temp += i;

  int ttime = uptime() - stime;
    printf(1, "child %d finished cpu intensive work in %d ticks, calculated %d\n", index, ttime, temp);
}

void
sleeping(int index)
{
  int stime = uptime();
  for(int i = 0; i < 100; ++i)
    sleep(10);
  int ttime = uptime() - stime;
    printf(1, "child %d finished sleeping work in %d ticks\n", index, ttime);
}

int
main(int argc, char *argv[])
{
  //Test Case 1
  printf(1, "\nTest Case 1 ...\n");
  printf(1, "Forking a cpu intensive processes ...\n");
  printf(1, "Main process also do same amount of processing ...\n");
  
  int pid = fork();
  int stime, ttime;
  int temp = 0;
  int prio = 3;

  if(pid < 0)
  {
    printf(1, "fork failed!\n");
    exit();
  }

  stime = uptime();

  if(pid != 0)
  {
    setprio(prio);
  }

  if(pid == 0)
  {
    printf(1, "child  priority : %d\n", getprio());
  }
  else
  {
    printf(1, "parent priority : %d\n", getprio());
    printf(1, "expected ratio of finishing times : %d/%d\n", 2*prio, prio + 1);
  }


  for (int j = 0; j < 5; ++j)
    for (int i = 0; i < (1<<29); ++i)
      temp += i;

  ttime = uptime() - stime;
  if(pid == 0)
  {
    printf(1, "child finished after %d ticks, calculated %d\n", ttime, temp);
    exit();
  }
  else
  {
    printf(1, "parent finished after %d ticks, calculated %d\n", ttime, temp);
    wait();
  }

  // TestCase 2
  printf(1, "\nTest Case 2 Running ...\n");
  printf(1, "Creating 5 cpu intensive process with priorities 1, 2, 3, 4 and 5 ...\n");
  printf(1, "Main process waits for these to terminate ...\n");

  
  for(int i = 0; i < 5; i++)
  {
    pid = fork();
    if(pid < 0)
    {
      printf(1, "fork failed!\n");
      exit();
    }
    if(pid == 0)
    {
      setprio(i + 1);
      cpuintensive(i);
      exit();
    }
  }

  while(wait() > 0);

  // TestCase 3
  printf(1, "\nTest Case 3 Running ...\n");
  printf(1, "Creating 8 cpu intensive process with priority 10 and \n");
  printf(1, "one sleeping process with priority 1 and another one with priority 100..\n");

  
  for(int i = 0; i < 10; i++)
  {
    pid = fork();
    if(pid < 0)
    {
      printf(1, "fork failed!\n");
      exit();
    }
    if(pid == 0)
    {
      if(i == 0)
      {
        setprio(100);
        sleeping(i);
      }
      else if(i == 1)
      {
        setprio(1);
        sleeping(i);
      }
      else
      {
        setprio(10);
        cpuintensive(i);
      }
      exit();
    }
  }

  while(wait() > 0);

  exit();
}