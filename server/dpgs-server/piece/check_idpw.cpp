#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <errno.h>

#include <unistd.h>

/* This will be replaced as member variable of class */
int listen_fd;
struct sockaddr_in address; 
int addrlen = sizeof(address); 

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
 * Make new socket to listen and connect with port.
 * Returns 0 on success, 1 on failure. 
 */
int make_connection (int port) {
    listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : "); 
		return 1;
	}

    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        return 1;
    }
	
	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(port); 
	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		return 1;
	} 

    if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
        perror("listen failed : "); 
        return 1;
    } 

    return 0;
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
    char * filename, * id_ptr, * pw_ptr;
    char buffer[128];
    FILE * fp;

    strcpy(filename, "user_info");
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

    return 0;
}

/*
 * Detect logout from client.
 * If logout_msg is 1, the client has disconnected. 
 * Returns 0 on success, 1 on failure.
 */
int detect_logout (int clnt_sock) {
    char logout_msg;

    // Receive logout_msg from client
    if (recv_bytes(clnt_sock, &logout_msg, 1) < 0) {
        return 1;
    }

    printf("recv msg: %c\n", logout_msg);

    if (logout_msg == '1') {
        printf("pthread_exit and call cleanup handler\n");
    }
    else {
        return 1;
    }


    return 0;
}

int main (void) {
    
    if (make_connection(9999) == 1) {
        fprintf(stderr, "Error: Make listen_fd failed\n");
        exit(EXIT_FAILURE);
    }

    printf("listen fd: %d\n", listen_fd);
    printf("listen ok\n");

    /* This piece will be inserted in while(1) later. */
    int clnt_sock;
    if ((clnt_sock = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen)) < 0) {
        perror("Error: accept"); 
        exit(EXIT_FAILURE);     // It will be replaced as return value or other thing in while(1). Maybe..?
    }

    printf("clnt_sock : %d\n", clnt_sock);

    if (check_idpw(clnt_sock) == 1) {
        fprintf(stderr, "Error: check_idpw failed\n");
        exit(EXIT_FAILURE);
    }

    if (detect_logout(clnt_sock) == 1) {
        fprintf(stderr, "Error: detect_logout failed\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
