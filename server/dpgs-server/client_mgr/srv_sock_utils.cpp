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
int recv_bytes(SSL * ssl, void * buf, size_t len){
    int ret, err;
    size_t readbytes;

    if ((ret = SSL_read_ex(ssl, buf, len, &readbytes)) == 0) {
        err = SSL_get_error(ssl, ret);
        if (err == SSL_ERROR_SSL) {
            return -2;
        }
        return -1;
    }

    return readbytes;
}

/* 
 * Return byte of sent.
 * If failed return -1.
 */
int send_bytes(SSL * ssl, void * buf, size_t len) {
    int ret, err;
    size_t written;

    if ((ret = SSL_write_ex(ssl, buf, len, &written)) == 0) {
        err = SSL_get_error(ssl, ret);
        if (err == SSL_ERROR_SSL) {
            return -2;
        }
        return -1;
    }

    return written;
}

/*
 * Check the id and pw that received from client
 * Sends a one-byte reply to the client (1 = authenticated, 0 = authentication failed).
 * Returns 0 on success, 1 on failure.
 */
int check_idpw (SSL * ssl) {
    char id[16], pw[16];
    char reply = '0';
    int ret, err;
    size_t readbytes;
    
    // Receive id/pw from client
    if (recv_bytes(ssl, id, sizeof(id)) < 0) {
        return 1;
    }
    if (recv_bytes(ssl, pw, sizeof(pw)) < 0) {
        return 1;
    }

    printf("%s\n%s\n", id, pw);


    // Check id/pw
    char * id_ptr, * pw_ptr;
    char buffer[128], filename[128];
    FILE * fp;

    strcpy(filename, USER_INFO_FILE_PATH);
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
    printf("%c\n", reply);
    if (send_bytes(ssl, &reply, 1) < 0) {
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
int recv_msg (int clnt_sock, SSL * ssl, StreamingModule* sm_ptr) {
    char logout_msg = 0x0;
    int ret;

    while (logout_msg != '0') {
        // Receive logout_msg from client
        if ((ret = recv_bytes(ssl, &logout_msg, 1)) < 0) {
            if (ret == -2) {
                fprintf(stderr, "Warning: %d is closed\n", clnt_sock);
            }
            return 1;
        }

        printf("recv msg: %c\n", logout_msg);

        if (logout_msg == '1') {
	    sm_ptr->update(1);
        }
        else if (logout_msg == '2') {
	    sm_ptr->update(2);
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
