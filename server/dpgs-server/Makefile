CC = aarch64-linux-gnu-g++
CFLAGS = -I /usr/aarch64-linux-gnu/include -I ./include -D_REENTERANT -c
LDFLAGS = -L /usr/aarch64-linux-gnu/lib -lpthread

CORE_DIR := ./core

dpgs-server: dpgs-server.o core_thread_manager.o
	$(CC) -o $@ $^ $(LDFLAGS)

dpgs-server.o: main.cpp
	$(CC) $(CFLAGS) $< -o $@

# ------------ CORE Compile ------------
core_thread_manager.o: $(CORE_DIR)/core_thread_manager.cpp
	$(CC) $(CFLAGS) $< -o $@

valgrind: dpgs-server
	valgrind --leak-check=full --leak-resolution=high --show-reachable=yes ./$<

clean: 
	rm -f dpgs-server *.o

