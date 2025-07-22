CC = aarch64-linux-gnu-g++
CFLAGS = -I /usr/aarch64-linux-gnu/include -I ./include -D_REENTERANT -g
LDFLAGS = -L /usr/aarch64-linux-gnu/lib -lpthread

SRC := main.cpp ./core/client_manager.cpp ./core/connection_manager.cpp ./core/client_info.cpp ./core/srv_sock_utils.cpp ./core/map_monitor.cpp

# $(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $@
main: 
	$(CC) -I ./include -lpthread $(SRC) -o $@ 
	$(CC) $(CFLAGS) $(LDFLAGS) ./core/client.cpp -o client

# ------------ Memory Check ------------- 
valgrind: 
	valgrind --leak-check=full --leak-resolution=high --show-reachable=yes ./main

clean: 
	rm -f temp client main

