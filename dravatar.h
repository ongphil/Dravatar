#ifndef DRAVATAR_INCLUDE_H
#define DRAVATAR_INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <math.h>
#include "status_packet.h"
#include "ax12.h"
#include "dravatar_pin.h"

#define BAUD 						57600
#define SOCKET_RECEPTION_TIME 		50 // (ms)
#define BLINK_TIME					50 // (ms)

#define LEFT_WHEELS_FORWARD_PIN 	20 
#define LEFT_WHEELS_BACKWARD_PIN 	16
#define RIGHT_WHEELS_FORWARD_PIN 	26
#define RIGHT_WHEELS_BACKWARD_PIN 	19

#define LED_PIN 					17

#define ENABLE_PIN					4

#define ID_LEFT_RIGHT				12
#define ID_UP_DOWN					11


#define MOVE_STOP					0
#define MOVE_FORWARD				1
#define MOVE_BACKWARD				2
#define MOVE_LEFT					3
#define MOVE_RIGHT					4

#define RETURN_SPEED				20

// Init the Robot (pins + AX12 baud rate)
// Initialize the pins to 0 for the wheels
// The pins and the baud rate are defined in this file:
//		- LEFT_WHEELS_FORWARD_PIN
//		- LEFT_WHEELS_BACKWARD_PIN
//		- RIGHT_WHEELS_FORWARD_PIN
//		- RIGHT_WHEELS_BACKWARD_PIN
//		- ENABLE_PIN
//		- BAUD
// Return 0 if no error
int robot_init();

// Rotate the head
// char id : id of the AX12
// int angle_degree : angle to to move (in degree)
// int speed : the speed to use (in RPM) (0 tu use the previous speed)
// Return 0 if no error
int rotate(char id, int angle_degree, int speed);

// Change the speed to use
// char id : id of the AX12
// int speed : angle to to move (in RPM)
// Return 0 if no error
int change_speed(char id, int speed);

// Turn ON/OFF the led
// char id : id of the AX12
// char state : state of the led (0 for OFF, else ON)
// Return 0 if no error
int led(char id, char state);

// Calculate the speed to take to go to a position A to position B
// in a limit of SOCKET_RECEPTION_TIME ms
// In this algorithm, 
// Return the speed in RPM
int calculate_speed_linear_by_time(int positionA, int positionB);


// Move the robot by turning the wheels
// int move_instructions :
//							- MOVE_STOP: to stop
//							- MOVE_FORWARD: to go forward
//							- MOVE_BACKWARD: to go backward
//							- MOVE_LEFT: to turn to left
//							- MOVE_RIGHT: to turn to right
// Return 0 if no error
int move_robot(int move_instruction);

// Blink the led (pin: LED_PIN) blink_loop times
// int blink_loop : Number of times the led will blink
// Return 0 if no error
int blink_led(int blink_loop);

// Put all the pins used to LOW et call ax_close()
// cf ax12.h
void dravatar_close();
#endif