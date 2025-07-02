CC = aarch64-linux-gnu-g++
CFLAGS = -I /usr/aarch64-linux-gnu/include -D_REENTERANT -c
LDFLAGS = -L /usr/aarch64-linux-gnu/lib -lpthread


dpgs-server: dpgs-server.o
	$(CC) -o $@ $< $(LDFLAGS)

dpgs-server.o: skeleton.cpp
	$(CC) $(CFLAGS) $< -o $@

valgrind:
	valgrind --leak-check=full --leak-resolution=high --show-reachable=yes ./dpgs-server

clean: 
	rm -f dpgs-server*

