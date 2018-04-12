#ifndef AX12_INCLUDE_H
#define AX12_INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "status_packet.h"


//#define DEBUG



// ************************ LIMIT VALUES *********************
#define MOVING_SPEED_HEX_MAX 	0x3FF
#define GOAL_POSITION_HEX_MAX 	0x3FF

#define MOVING_SPEED_MAX_RPM	114

// ***************************PINS****************************
#define RX 15
#define TX 14

// ************************ INSTRUCTIONS ************************
#define INST_PING 		0x01
#define INST_READ_DATA 	0x02
#define INST_WRITE_DATA 0x03
#define INST_RESET 		0x06

// ************************ AX12 MEMORY ************************
#define POS_ID 				0x03
#define POS_BAUD_RATE		0x04
#define POS_DELAY_TIME		0x05
#define POS_CW_LIMIT_ANGLE  0X06
#define POS_CCW_LIMIT_ANGLE 0X08
#define POS_LED				0x19
#define POS_GOAL_POSITION	0x1E
#define POS_MOVING_SPEED	0x20

// ************************* AX12 ID ***************************
#define ID_ALL  0xFE

// ***************** MINIMUM ANGLE *******************************
#define MIN_ANGLE 	0.29
#define MIN_RPM 	0.11


// Initialyze the diretion pin for the half UART communication of the ax12
// direction_pin is the number of the raspberry pin in wiringPi table (use piPin_to_wiringPiPin)
// Return 0 if the pin is recognize, -1 if not
int ax_init(int* direction_pin, int baud);

// Close connection to serial port and end ax12
void ax_close();

// Convert raspberry pin number to wiringPi lib pin number
// Return the wiringPi pin number or -1 if no match
int piPin_to_wiringPipin(int pin);

// Convert speed value rpm to hexadecimal value for ax12
// return the HEX value
int rpm_to_hex(float rpm);

// Convert the hexadecimal speed value of ax12 to rpm value
// Return the value in rpm
float hex_to_rpm(int hex);


// Convert an angle in degrees to its hexadecimal value for goal position
// Return the hexadecimal value
int deg_to_hex(float deg);

// Convert an hexadecimal to its value in degree (position goal)
// Return the angle in degree
float hex_to_deg(int hex);

// Calculate and return the checksum of message
char get_checksum(char* message);

// Calculate and add the checksum at the end of message
// Return 0 if the checksum is added, -1 if the message is not well created
int add_checksum(char* message);

// Verify the checksum of the message
// Return 0 if the checksums match, -1 if not
int verify_checksum(char* message);

// Create the message and put it in message
// Return 0 if the message is successfully created, -1 if not
int create_message(	char* message,
					char id,
					char instruction,
					char* param, 
					int param_len);

// Display the message, based on the content of message(length)
void display_message(char* message);

//Send the instruction message given in parameter
int send_message(char *message);

//Receive message and store it in the *packet
int receive_message(int number_char, packet_t *packet);

// Print the type of error in a status message (packet_t.error)
void display_status_error(char error);

// Send ping message and receive status message
// Return 0 if success, -1 if no response or error
int ping(char id);


// Move the servo to the *pos
// If speed is NULL, we use the previous speed
// pos and speed are in hexadecimal
// Return 0 if no error
int set_goal_position(	char id,
						int* pos,
						int* speed);

// Return the current position of the servo (goal position)
// The returned value is in hexadecimal
int get_goal_position(char id);

//Set id of connected ax12
void set_id(char old_id, char new_id);

//Set baud rate of connected ax12
void set_baud_rate(char id, char baud_rate_hex);

//Set delay time of connected ax12
void set_delay_time(char id, char delay_time_us);

//Set limit angles of connected ax12
void set_limit_angle(char id, int cw_limit_angle, int ccw_limit_angle);

// Set the moving speed of the servo (moving speed)
// speed is in hexadecimal
// Return 0 if no error occurs
int set_moving_speed(char id, int* speed);

// Return the moving speed value in hexadecimal of the servo (moving speed)
int get_moving_speed(char id);

//change led state, 0 off
void ax_led(char id, char state);
#endif