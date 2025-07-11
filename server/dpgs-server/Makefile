CC = aarch64-linux-gnu-g++
CFLAGS = -I /usr/aarch64-linux-gnu/include -I ./include -D_REENTERANT
LDFLAGS = -L /usr/aarch64-linux-gnu/lib -lpthread

temp: 
	$(CC) $(CFLAGS) $(LDFLAGS) temp.cpp ./core/client_manager.cpp ./core/client_info.cpp -o $@

# ------------ Memory Check ------------- 
valgrind: 
	valgrind --leak-check=full --leak-resolution=high --show-reachable=yes ./temp

clean: 
	rm -f temp

