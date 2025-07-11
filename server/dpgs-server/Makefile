CC = aarch64-linux-gnu-g++
CFLAGS = -I /usr/aarch64-linux-gnu/include -I ./include -D_REENTERANT
LDFLAGS = -L /usr/aarch64-linux-gnu/lib -lpthread

SRC := temp.cpp ./core/client_manager.cpp ./core/client_info.cpp ./core/srv_sock_utils.cpp

temp: 
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $@
	$(CC) $(CFLAGS) $(LDFLAGS) ./core/client.cpp -o client

# ------------ Memory Check ------------- 
valgrind: 
	valgrind --leak-check=full --leak-resolution=high --show-reachable=yes ./temp

clean: 
	rm -f temp client

