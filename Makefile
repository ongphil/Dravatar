TARGET=dravatar
CC=gcc
FLAGS=-Wall -lwiringPi -lm 
END_FLAGS=-lpthread


all: dravatar ax12 main status_packet protocol
	$(CC) $(FLAGS) -o $(TARGET) main.o ax12.o dravatar.o status_packet.o protocol.o $(END_FLAGS)

run: all
	./$(TARGET)

main:
	$(CC) $(FLAGS) -c main.c $(END_FLAGS)

ax12:
	$(CC) $(FLAGS) -c ax12.c

dravatar:
	$(CC) $(FLAGS) -c dravatar.c

status_packet:
	$(CC) $(FLAGS) -c status_packet.c

protocol:
	$(CC) $(FLAGS) -c protocol.c


clean:
	rm *.o $(TARGET)
