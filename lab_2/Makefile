CC=g++
CFLAGS= -g 


all: proxy

proxy: proxy.cpp
	$(CC) $(CFLAGS) -o proxy.o -c proxy.cpp
	$(CC) $(CFLAGS) -o proxy proxy.o

clean:
	rm -f proxy   *.o
	
tar:
	tar -cvzf 10010175.tar.gz proxy.cpp README Makefile
