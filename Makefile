CC=g++

default: sendfile recvfile routfile

all: sendfile recvfile routfile

sender.o: src/sender.cpp src/utility.h src/send_window.h
	$(CC) -c src/sender.cpp

router.o: src/router.cpp src/utility.h src/send_window.h
	$(CC) -c -pthread src/router.cpp

utility.o: src/utility.cpp src/utility.h
	$(CC) -c src/utility.cpp

receiver.o: src/receiver.cpp src/utility.h
	$(CC) -c src/receiver.cpp

sendfile: sender.o utility.o
	$(CC) -o sendfile sender.o utility.o

recvfile: receiver.o utility.o
	$(CC) -o recvfile receiver.o utility.o

routfile: router.o utility.o
	$(CC) -pthread -o routfile router.o utility.o

clean:
	rm -f sendfile recvfile routfile sender.o utility.o receiver.o router.o
