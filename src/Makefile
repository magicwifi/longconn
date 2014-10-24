longconn: main.o fetch_thread.o  utils.o 
	$(CC) $(LDFLAGS) main.o fetch_thread.o utils.o -o longconn -lpthread

fetch_thread.o: fetch_thread.c
	$(CC) $(CFLAGS) -c fetch_thread.c 

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c 

main.o: main.c
	$(CC) $(CFLAGS) -c main.c 

clean:
	rm -rf *.o longconn
