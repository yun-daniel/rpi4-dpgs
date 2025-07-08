CC = aarch64-linux-gnu-g++
CFLAGS = -I /usr/aarch64-linux-gnu/include -I ./include -D_REENTERANT -c
LDFLAGS = -L /usr/aarch64-linux-gnu/lib -lpthread

CORE_DIR := ./core
OBJS := dpgs-server.o core_thread_manager.o client_thread_manager.o core_thread_routine1.o 

dpgs-server: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

dpgs-server.o: main.cpp
	$(CC) $(CFLAGS) $< -o $@

# ------------ CORE Compile ------------
core_thread_manager.o: $(CORE_DIR)/core_thread_manager.cpp
	$(CC) $(CFLAGS) $< -o $@

client_thread_manager.o: $(CORE_DIR)/client_thread_manager.cpp
	$(CC) $(CFLAGS) $< -o $@

core_thread_routine1.o : $(CORE_DIR)/core_thread_routine1.cpp
	$(CC) $(CFLAGS) $< -o $@

# ------------ Memory Check ------------- 
valgrind: dpgs-server
	valgrind --leak-check=full --leak-resolution=high --show-reachable=yes ./$<

clean: 
	rm -f dpgs-server *.o

