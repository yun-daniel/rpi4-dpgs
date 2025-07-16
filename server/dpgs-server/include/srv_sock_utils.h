#ifndef __SRV_SOCK_UTILS_H__
#define __SRV_SOCK_UTILS_H__

#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#include "streaming_module.h"


void unlock_mutex (void * arg);
int recv_bytes(int fd, void * buf, size_t len);
int send_bytes(int fd, void * buf, size_t len);
int check_idpw (int clnt_sock);
int recv_msg (int clnt_sock, StreamingModule* sm_ptr);


#endif  // __SRV_SOCK_UTILS_H__
