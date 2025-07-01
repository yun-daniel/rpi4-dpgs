CC = aarch64-linux-gnu-g++
CFLAGS = -I /usr/aarch64-linux-gnu/include -D_REENTERANT -c
LDFLAGS = -L /usr/aarch64-linux-gnu/lib -lpthread


dpgs-server: dpgs-server.o
	$(CC) -o $@ $< $(LDFLAGS)

dpgs-server.o: skeleton.cpp
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -f dpgs-server*

