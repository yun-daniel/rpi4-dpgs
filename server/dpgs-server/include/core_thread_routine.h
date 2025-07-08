#ifndef CORE_THREAD_ROUTINE_H
#define CORE_THREAD_ROUTINE_H

#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <pthread.h>
#include <errno.h>

void * core_thread_routine1 (void *arg);
void * core_thread_routine2 (void *arg);
void * core_thread_routine3 (void *arg);
void * core_thread_routine4 (void *arg);
void * core_thread_routine5 (void *arg);

#endif  // CORE_THREAD_ROUTINE_H