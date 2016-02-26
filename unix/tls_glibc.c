#define _GNU_SOURCE

#include <asm/prctl.h>
#include <sys/prctl.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* FS Support */
int arch_prctl(int code, unsigned long addr);

void set_fs(void * addr)
{
  if (arch_prctl(ARCH_SET_FS, (unsigned long) addr)) {
    perror("arch_prctl(ARCH_SET_FS)");
    exit(EXIT_FAILURE);
  }
}

unsigned long get_fs(void)
{
  unsigned long fs_addr;
  if (arch_prctl(ARCH_GET_FS, (unsigned long) &fs_addr)) {
    perror("arch_prctl(ARCH_GET_FS)");
    exit(EXIT_FAILURE);
  }
  return fs_addr;
}

/* glibc / pthread */
int new_thread(void* (*fn)(void*))
{
  pthread_t pt;
  if (pthread_create(&pt, NULL, fn, NULL)) {
    perror("pthread_create");
    exit(EXIT_FAILURE);
  }
  pthread_detach(pt);
  return 0;
}

/* glibc TLS */
__thread int thread_i;

void print_id(char *who, int tid, int * addr)
{
  *addr = tid;
  printf("[%s] thread_i: %i (%p)\n", who, *addr, addr);
}

void* child(void * arg)
{
  printf("[child] started\n");
  printf("[child] tls fs: %p\n", (void*) get_fs());
  print_id("child", 0, &thread_i);
  usleep(500000);
  return NULL;
}

int main(void)
{
  int tid, status;

  printf("[main_] started\n");
  printf("[main_] tls fs: %p\n", (void*) get_fs());

  tid = new_thread(&child);
  printf("[main_] child launched: %d\n", tid);

  print_id("main_", 1, &thread_i);
  usleep(500000);

  return EXIT_SUCCESS;
}
