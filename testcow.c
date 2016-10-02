#include "types.h"
#include "user.h"

int x = 1;

void test1()
{
  printf(1, "Free pages before fork : %d, x : %d\n", getNumFreePages(), x);
  if(fork() == 0)
  {
    printf(1, "Free pages after fork : %d, x : %d\n", getNumFreePages(), x);
    x = 2;
    printf(1, "Free pages after updating x in child : %d, x : %d\n", getNumFreePages(), x);
    exit();
  }
  else
  {
    wait();
    printf(1, "Free pages after reaping child : %d, x : %d\n", getNumFreePages(), x);
  }
}

void test2()
{
  printf(1, "Free pages before fork : %d, x : %d\n", getNumFreePages(), x);
  if(fork() == 0)
  {
    printf(1, "Free pages after fork 1 : %d, x : %d\n", getNumFreePages(), x);
    if(fork() == 0)
    {
      printf(1, "Free pages after fork 2 : %d, x : %d\n", getNumFreePages(), x);
      x = 2;
      printf(1, "Free pages after updating x in child 2 : %d, x : %d\n", getNumFreePages(), x);
      exit();
    }
    else
    {
      wait();
      printf(1, "Free pages after reaping child 2 : %d, x : %d\n", getNumFreePages(), x);
      x = 2;
      printf(1, "Free pages after updating x in child 1: %d, x : %d\n", getNumFreePages(), x);
    }
    exit();
  }
  else
  {
    wait();
    printf(1, "Free pages after reaping both child : %d, x : %d\n", getNumFreePages(), x);
  }
}

void test3()
{
  printf(1, "Free pages before fork : %d, x : %d\n", getNumFreePages(), x);
  if(fork() == 0)
  {
    printf(1, "Free pages after fork : %d, x : %d\n", getNumFreePages(), x);
    exit();
  }
  else
  {
    wait();
    printf(1, "Free pages after reaping child : %d, x : %d\n", getNumFreePages(), x);
    x = 2;
    printf(1, "Free pages after updating x in parent: %d, x : %d\n", getNumFreePages(), x);
  }
}

int main()
{   
  printf(1, "Test Case 1 ...\n");
  test1();
  printf(1, "------------------------\n");

  printf(1, "Test Case 2 ...\n");
  test2();
  printf(1, "------------------------\n");    

  printf(1, "Test Case 3 ...\n");
  test();
  printf(1, "------------------------\n");

  exit();
}