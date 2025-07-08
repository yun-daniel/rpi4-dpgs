CC = aarch64-linux-gnu-g++
CFLAGS = -I /usr/aarch64-linux-gnu/include -I ./include -D_REENTERANT
LDFLAGS = -L /usr/aarch64-linux-gnu/lib -lpthread

check_idpw:
	$(CC) check_idpw.cpp $(CFLAGS) $(LDFLAGS) -o $@.out
	scp check_idpw.out user_info dabee@192.168.0.67:/home/dabee/piece

clean:
	rm -f *.out

