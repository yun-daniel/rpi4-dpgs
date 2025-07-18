#include "srv_sock_utils.h"

void unlock_mutex (void * arg) {
    pthread_mutex_t * m = static_cast<pthread_mutex_t *>(arg);
    pthread_mutex_unlock(m);
}

/* 
 * Return byte of received.
 * If failed return -1.
 * If fd is closed return -2.
 */
int recv_bytes(int fd, void * buf, size_t len){
    char * p = (char *)buf ;
    size_t acc = 0 ;

    while (acc < len) {
        size_t recved ;
        recved = recv(fd, p, len - acc, 0) ;
        if (recved == -1)
            return -1 ;
        else if (recved == 0) {
            return -2 ;
        }
        p += recved ;
        acc += recved ;
    }
    return acc ;
}

/* 
 * Return byte of sent.
 * If failed return -1.
 */
int send_bytes(int fd, void * buf, size_t len){
    char * p = (char *)buf ;
    size_t acc = 0 ;

    while (acc < len) {
        size_t sent ;
        sent = send(fd, p, len - acc, MSG_NOSIGNAL) ;
        if (sent == -1) {
            return -1 ;
        }
        p += sent ;
        acc += sent ;
    }
    return acc ;
}

/*
 * Check the id and pw that received from client
 * Sends a one-byte reply to the client (1 = authenticated, 0 = authentication failed).
 * Returns 0 on success, 1 on failure.
 */
int check_idpw (int clnt_sock) {
    char id[16], pw[16];
    char reply = '0';
    
    // Receive id/pw from client
    if (recv_bytes(clnt_sock, id, sizeof(id)) < 0) {
        return 1;
    }
    if (recv_bytes(clnt_sock, pw, sizeof(pw)) < 0) {
        return 1;
    }

    printf("%s\n%s\n", id, pw);


    // Check id/pw
    char * id_ptr, * pw_ptr;
    char buffer[128], filename[128];
    FILE * fp;

    strcpy(filename, "./user_info");
    if ((fp = fopen(filename, "r")) == NULL) {
        return 1;
    }

    while (fgets(buffer, 128, fp) != NULL) {
        id_ptr = strtok(buffer, ":");
        pw_ptr = strtok(NULL, "\n");
        printf("%s\t%s\n", id_ptr, pw_ptr);
        
        if ((strcmp(id, id_ptr) == 0) && (strcmp(pw, pw_ptr) == 0)) {
            reply = '1';
            break;
        } 
    }

    // Send reply to client
    // printf("%c\n", reply);
    if (send_bytes(clnt_sock, &reply, 1) < 0) {
        return 1;
    }

    fclose (fp);

    return 0;
}

/*
 * Detect client msg
 * 0:logout, 1:change cam1, 2:change cam2
 * Returns 0 on success, 1 on failure.
 */
//StreamingModule * stm_ptr
int recv_msg (int clnt_sock, int * cam_rq, pthread_t * tid_arr, pthread_mutex_t * m_ptr) {
    char logout_msg = 0x0;
    int ret;

    while (logout_msg != '0') {
        // Receive logout_msg from client
        if ((ret = recv_bytes(clnt_sock, &logout_msg, 1)) < 0) {
            if (ret == -2) {
                fprintf(stderr, "Warning: %d is closed\n", clnt_sock);
            }
            return 1;
        }

        printf("recv msg: %c\n", logout_msg);

        if (logout_msg == '1') {
            pthread_mutex_lock(m_ptr); 
                pthread_cleanup_push(unlock_mutex, (void *)m_ptr);
                *cam_rq = 1;
            pthread_cleanup_pop(1);
            pthread_kill(tid_arr[1], SIGUSR1);
        }
        else if (logout_msg == '2') {
            pthread_mutex_lock(m_ptr); 
                pthread_cleanup_push(unlock_mutex, (void *)m_ptr);
                *cam_rq = 2;
            pthread_cleanup_pop(1);
            pthread_kill(tid_arr[1], SIGUSR1);
        }
        else {
            if (logout_msg == '0') {
                printf("Client %x is logout\n", clnt_sock);
            }
            else {
                fprintf(stderr, "Warning: recv msg is undefined\n");
            }
        }
    }

    return 0;
}
