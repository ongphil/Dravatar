#include "ax12.h"

int* direction = NULL;
int fd = 0;

int ax_init(int* direction_pin, int baud)
{
	// We set the enable pin
	direction = direction_pin;

	fd = serialOpen("/dev/ttyAMA0", baud);
	if( fd == -1 )
	{
		printf("Error: serialOpen\n");
		return -1;
	}

	return 0;
}
void ax_close()
{
	serialClose(fd);
}
int piPin_to_wiringPipin(int pin)
{
	int wiring_pin = -1;
	switch(pin)
	{
		case 2:
				wiring_pin = 8;
				break;
		case 3:
				wiring_pin = 9;
				break;
		case 4:
				wiring_pin = 4;
				break;
		case 5:
				wiring_pin = 21;
				break;
		case 6:
				wiring_pin = 22;
				break;
		case 7:
				wiring_pin = 11;
				break;
		case 8:
				wiring_pin = 10;
				break;
		case 9:
				wiring_pin = 13;
				break;
		case 10:
				wiring_pin = 12;
				break;
		case 11:
				wiring_pin = 14;
				break;
		case 12:
				wiring_pin = 26;
				break;
		case 13:
				wiring_pin = 23;
				break;
		case 14:
				wiring_pin = 15;
				break;
		case 15:
				wiring_pin = 16;
				break;
		case 16:
				wiring_pin = 27;
				break;
		case 17:
				wiring_pin = 0;
				break;
		case 18:
				wiring_pin = 1;
				break;
		case 19:
				wiring_pin = 24;
				break;
		case 20:
				wiring_pin = 28;
				break;
		case 21:
				wiring_pin = 29;
				break;
		case 22:
				wiring_pin = 3;
				break;
		case 23:
				wiring_pin = 4;
				break;
		case 24:
				wiring_pin = 5;
				break;
		case 25:
				wiring_pin = 6;
				break;
		case 26:
				wiring_pin = 25;
				break;
		case 27:
				wiring_pin = 2;
				break;
		default:
				wiring_pin = -1;
				break;
	}
	return wiring_pin;
}
// Calculate and return the checksum
char get_checksum(char* message)
{
	int i = 0;
	int sum = 0;
	char checksum = 0;
	int len = (int)message[3];
	for(i=0;i<(len-1+2);i++)
	{
		sum += message[2+i];
	}
	checksum = (char) (~sum) & 0xFF;
	return checksum;
}
int verify_checksum(char* message)
{
	if( (unsigned char)message[message[3]+3] != (unsigned char)get_checksum(message) )
		return -1;
	return 0;
}

int add_checksum(char* message)
{
	// If message is NULL
	if( message == NULL )
	{
		#ifdef DEBUG
		printf("add_checksum: Message NULL\n");
		#endif
		return -1;
	}
	// If header is not 0xFF 0xFF
	if( (unsigned char)message[0] != 0xFF || (unsigned char)message[1] != 0xFF )
	{
		#ifdef DEBUG
		printf("add_checksum: Wrong headers 0x%02x 0x%02x\n", (unsigned char)message[0], (unsigned char)message[1]);
		#endif
		return -1;
	}
	// If ID is not correct
	if( message[2] > 0xFE )
	{
		#ifdef DEBUG
		printf("add_checksum: Wrong ID, 0x%02x\n", message[2]);
		#endif
		return -1;
	}
	// If length is wrong
	if( message[3] <= 0 )
	{
		#ifdef DEBUG
		printf("add_checksum: Length <= 0, %d\n", (int)message[3]);
		#endif
		return -1;
	}

	message[message[3]+3] = get_checksum(message);//checksum;

	return 0;
}

int create_message(	char* message,
					char id,
					char instruction,
					char* param, 
					int param_len)
{
	int i = 0;
	
	message[0] = 0xFF;					// HEADER
	message[1] = 0xFF;
	message[2] = id;					// ID
	message[3] = (char)(param_len+2);	// LENGTH
	message[4] = instruction;			// INSTRUCTION
	for(i=0;i<param_len;i++)			// PARAMETER
		message[5+i] = param[i];
	if( add_checksum(message) == -1 )	// CHECKSUM
		return -1;
	return 0;
}


void display_message(char* message)
{
	int i = 0;
	printf("Message: ");
	if( message != NULL )
	{
		for(i=0;i<(message[3]+4);i++)
			printf("0x%02x ", (unsigned char)message[i]);
		printf("\n");
	}
}
// Error message from status packets
void display_status_error(char error)
{
	if( (error & 0x01) != 0 )
		printf("Status error: Input Voltage error\n");
	if( (error & 0x02) != 0 )
		printf("Status error: Angle Limit error\n");
	if( (error & 0x04) != 0 )
		printf("Status error: Overheating error\n");
	if( (error & 0x08) != 0 )
		printf("Status error: Range error\n");
	if( (error & 0x10) != 0 )
		printf("Status error: Checksum error\n");
	if( (error & 0x20) != 0 )
		printf("Status error: Overload error\n");
	if( (error & 0x40) != 0 )
		printf("Status error: Instruction error\n");
}

// Send instruction packet
int send_message(char* message)
{
	int i, size_message = message[3]+4;  //message size  +4 = number of bytes in message

	// We verify that the enable pin has been set by ax_init function
	if( direction == NULL )
	{
		printf("Error: enable pin not set!\n");
		return -1;
	}


	digitalWrite(*direction, HIGH);
	#ifdef DEBUG
		printf("Send message (type %02x) to ax12 %02x", message[4], message[2]);
	#endif
	
	for (i = 0 ; i< size_message ; i++) //for each byte in message
	{
		serialPutchar(fd, message[i]); //a byte is sent

		#ifdef DEBUG
			printf("char : 0x%02x sent\n", (unsigned char)message[i]);
		#endif
	}

	delay(4);
	digitalWrite(*direction, LOW);

	return 0;
}

// Receive status packet
int receive_message(int number_char, packet_t* packet) 
{
	int i = 0;
	char rec_message[50]; //temporary string

	//digitalWrite(*direction, LOW);
	delay(2);

	for(i = 0 ; i<number_char ; i++) //for each available bytes
	{
		rec_message[i] = serialGetchar(fd) ; //the byte is stored in the temporary string
		
		#ifdef DEBUG
			printf("char : %02x received\n", rec_message[i]);
		#endif
	}
	delay(2);

	//digitalWrite(*direction, HIGH);

	packet->id = rec_message[2]; //id is stored in packet structure
	packet->len = rec_message[3]; //length is stored in packet structure
	packet->error = rec_message[4]; //error is stored in packet structure

	#ifdef DEBUG
		printf("message received : id ax12 = %02x, length = %02x, error = %02x \n", packet->id, packet->len, packet->error);
	#endif

	for(i = 0 ; i<packet->len-2 ; i++) //for each content byte in the message minus checksum
	{
		packet->param[i]=rec_message[i+5]; //the content is moved into parameters
		#ifdef DEBUG
			printf("content byte number %d : %02x \n", i, packet->param[i]); //the content is moved into parameters
		#endif
	}

	if(packet->error != 0) return -1; //error in the received message
	return 0;  //else no error in the return message
}

int rpm_to_hex(float rpm)
{
	int hex = 0;
	hex = rpm / MIN_RPM;
	return hex;
}

float hex_to_rpm(int hex)
{
	float rpm = 0;
	rpm = (float)hex * MIN_RPM;
	return rpm;
}

// Convert degrees to hex value for AX12
int deg_to_hex(float deg)
{
	int hex = 0;
	hex = deg / MIN_ANGLE;
	return hex;
}

// Convert hex value to degrees
float hex_to_deg(int hex)
{
	float deg = 0;
	deg = (float)hex * MIN_ANGLE;
	return deg;
}

// Move the servo to pos
int set_goal_position(char id, int* pos, int* speed)
{
	char message[10];
	char param[5];
	int param_len = 3;

// Test
	if( pos ==  NULL )
	{
		#ifdef DEBUG
		printf("set_goal_position: position NULL\n");
		#endif
		return -1;
	}
	param[0] = POS_GOAL_POSITION;
	param[1] = (*pos) & 0xFF;
	param[2] = ( (*pos) >> 8 ) & 0xFF;

	// speed can be NULL
	if( speed != NULL )
	{
		param[3] = (*speed) & 0xFF;
		param[4] = ( (*speed) >> 8 ) & 0xFF;
		param_len += 2;
	}

// Message creation
	if( create_message(message, id, INST_WRITE_DATA, param, param_len) != 0 )
	{
		#ifdef DEBUG
		printf("set_goal_position: Message creation failed\n");
		#endif
		return -1;
	}

// Sending message
	send_message(message);

	#ifdef DEBUG
	printf("Goal position: set 0x%02x%02x\n", param[2], param[1]);
	if( param_len == 5 )
		printf("Moving speed: set 0x%02x%02x\n", param[4], param[3]);
	#endif

	return 0;
}

// get GOAL POSITION
int get_goal_position(char id)
{
	packet_t packet;
	char message[10];
	char param[2] = {POS_GOAL_POSITION, 0x02};
	int param_len = 2;
	int i = 0, data_size = 0;
	int hex = 0;

// Message creation
	if( create_message(message, id, INST_READ_DATA, param, param_len) != 0 )
	{
		#ifdef DEBUG
		printf("get_goal_position: Message creation failed\n");
		#endif
		return -1;
	}

// Sending message
	send_message(message);

// Reception status packet
	while( i < 10 )
	{
		data_size = serialDataAvail(fd);
		if( data_size > 0 )
		{
			if( receive_message(data_size, &packet) == 0 )
				i = 20;
		}
		i++;
	}

// Error
	// We didn't receive the Status message
	if( i < 20 )
	{
		#ifdef DEBUG
		printf("get_goal_position: No response\n");
		#endif
		return -1;
	}

	if( packet.error != 0 )
	{
		#ifdef DEBUG
		display_status_error(packet.error);
		#endif
		return -1;
	}

// Success
	hex = packet.param[1] << 8;
	hex |= packet.param[0];
	return hex;
}

// Change the speed of the servo
int set_moving_speed(char id, int* speed)
{
	char message[10];
	char param[3];
	int param_len = 3;
	int hex = 0;

// Test
	if( speed == NULL )
	{
		#ifdef DEBUG
		printf("set_moving_speed: speed NULL\n");
		#endif
		return -1;
	}

// Message creation
	hex = *speed;
	if( hex > MOVING_SPEED_HEX_MAX )
		hex = MOVING_SPEED_HEX_MAX;

	param[0] = POS_MOVING_SPEED;
	param[1] = hex & 0xFF;
	param[2] = ( hex >> 8 ) & 0xFF;

	

	if( create_message(message, id, INST_WRITE_DATA, param, param_len) != 0 )
	{
		#ifdef DEBUG
		printf("set_moving_speed: Message creation failed\n");
		#endif
		return -1;
	}

// Sending message
	send_message(message);

	#ifdef DEBUG
	printf("Moving Speed: set 0x%02x%02x\n", param[2], param[1]);
	#endif

	return 0;
}

// Get the speed of the servo
int get_moving_speed(char id)
{
	packet_t packet;
	char message[10];
	char param[2] = {POS_MOVING_SPEED, 0x02};
	int param_len = 2;
	int i = 0, data_size = 0;
	int hex = 0;

// Message creation
	if( create_message(message, id, INST_READ_DATA, param, param_len) != 0 )
	{
		#ifdef DEBUG
		printf("get_moving_speed: Message creation failed\n");
		#endif
		return -1;
	}

// Sending message
	send_message(message);

// Reception
	while( i < 10 )
	{
		data_size = serialDataAvail(fd);
		if( data_size > 0 )
		{
			if( receive_message(data_size, &packet) == 0 )
				i = 20;
		}
		i++;
	}

// Error
	// We didn't receive the Status message
	if( i < 20 )
	{
		#ifdef DEBUG
		printf("get_moving_speed: No response\n");
		#endif
		return -1;
	}

	if( packet.error != 0 )
	{
		#ifdef DEBUG
		display_status_error(packet.error);
		#endif
		return -1;
	}

// Success
	hex = packet.param[1] << 8;
	hex |= packet.param[0];
	return hex;
}
// Set the id of all the servos connected (!)
void set_id(char old_id, char new_id)
{
	char message[10];
	char param[2]={POS_ID, new_id};  //parameters, adress of id and new id
	if( create_message(message, old_id, INST_WRITE_DATA, param, 2) == 0 )
	{
		#ifdef DEBUG
			printf("message set_id created\n");
		#endif
		send_message(message);
	}
}

void set_baud_rate(char id, char baud_rate_hex) //baud rate in hexa 1 to 0xCF
{
	char message[10];
	char param[2]={POS_BAUD_RATE, baud_rate_hex};  //parameters, adress of baud rate id and new baud rate
	if( create_message(message, id, INST_WRITE_DATA, param, 2) == 0 )
	{
		#ifdef DEBUG
			printf("message set_baud_rate created\n");
		#endif
		send_message(message);
	}
}

void set_delay_time(char id, char delay_time_us) //delay time in us
{
	char message[10];
	delay_time_us=delay_time_us/2;
	char param[2]={POS_DELAY_TIME, delay_time_us};  //parameters, adress of delay time and new delay time
	if( create_message(message, id, INST_WRITE_DATA, param, 2) == 0 )
	{
		#ifdef DEBUG
			printf("message set_delay_time created\n");
		#endif
		send_message(message);
	}	
}

void set_limit_angle(char id, int cw_limit_angle, int ccw_limit_angle) // 2 bytes for an angle
{
	char message[10];

	char low_cw = (char) cw_limit_angle; //cast char to int
	char high_cw = (char) (cw_limit_angle >> 8) & 0xFF;  
	char low_ccw = (char) ccw_limit_angle; //same
	char high_ccw = (char) (ccw_limit_angle >> 8) & 0xFF; 

	char param[5]={ POS_CW_LIMIT_ANGLE, low_cw, high_cw, low_ccw, high_ccw};  //parameters, adress of cw limit angle time and new cw and ccw limit angle (2 x 2 bytes)
	
	if( create_message(message, id, INST_WRITE_DATA, param, 5) == 0 )
	{
		#ifdef DEBUG
			printf("message set_limit_angle created\n");
		#endif
		send_message(message);
	}
}

int ping(char id)
{
	packet_t packet;
	char message[10];
	char param[1]; //no parameter
	int i = 0, data_size = 0;
	

	if( create_message(message, id, INST_PING, param, 0) == 0 )
	{
		#ifdef DEBUG
			printf("message ping created\n");
		#endif
		send_message(message);  
	}
	// wait for return message

	// Reception
	while( i < 10 )
	{
		data_size = serialDataAvail(fd);
		if( data_size > 0 )
		{
			if( receive_message(data_size, &packet) == 0 )
				i = 20;
		}
		i++;
	}

	// Error
	// We didn't receive the Status message
	if( i < 20 )
	{
		#ifdef DEBUG
			printf("ping : No response\n");
		#endif
		return -1;
	}

	// Error
	if( packet.error != 0 )
	{
		#ifdef DEBUG
			display_status_error(packet.error);
		#endif
		return -1;
	}

// Success
	return 0;

}

void ax_led(char id, char state)  //state=0 to turn off, anything else for on
{
	char message[10];
	char param[2];
	param[0] = POS_LED;  

	if(state == 0) param[1] = 0x00 ; //led off
	else param[1] = 0x01; //led on

	if( create_message(message, id, INST_WRITE_DATA, param, 2) == 0 )
	{
		#ifdef DEBUG
			printf("message change_state_led created\n");
		#endif
		send_message(message);
	}	
}
