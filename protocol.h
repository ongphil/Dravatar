#ifndef PROTOCOL_INCLUDE_H
#define PROTOCOL_INCLUDE_H

#include <stdio.h>
#include <stdlib.h>

#define POSITION 	0x01
#define MOVE 		0x02

#define MSG_POSITION_LEN 10
#define MSG_MOVE_LEN 10

#define MSG_RECEIVED_MAX 10


typedef struct position
{
	int x;
	int y;
	int z;
}position_t;

// Display the values in a poistion_t variable
void display_position(position_t pos);


// Verify the value of the checksum
// Return 0 if OK else -1
int check_checksum(char* message);

// Look for different messages in an accumulated messages in a string
// and fill "list" all the messages as an array of char
// Return the number of messages found or -1 if error
// read_size is the length of messages
int divide_messages(char* messages,
					int read_size,
					char list[MSG_RECEIVED_MAX][MSG_POSITION_LEN]);

// Fill a position_t with values in a POSITION message
// Return 0 else return -1 if error
int parse_message_position(char* message, position_t* pos);
int parse_message_move(char* message, int* move_instruction);

#endif