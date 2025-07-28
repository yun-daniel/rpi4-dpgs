#ifndef SRV_SOCK_UTILS_H
#define SRV_SOCK_UTILS_H

#include "config.h"

#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "streaming_module.h"

void unlock_mutex (void * arg);
int recv_bytes(SSL * ssl, void * buf, size_t len);
int send_bytes(SSL * ssl, void * buf, size_t len);
int check_idpw (SSL * ssl);
int recv_msg (int clnt_sock, SSL * ssl, StreamingModule* sm_ptr);

#endif  // SRV_SOCK_UTILS_H
