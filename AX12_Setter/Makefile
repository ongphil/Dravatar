
FLAGS=-Wall -lwiringPi -lm 


run: all
	./ax_setter

all: main ax12 status_packet
	gcc  $(FLAGS) -o ax_setter main.o ax12.o status_packet.o

main:
	gcc $(FLAGS) -c main.c

ax12:
	gcc $(FLAGS) -c ax12.c

status_packet:
	gcc $(FLAGS) -c status_packet.c

clean:
	rm *.o ax_setter
