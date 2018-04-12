#include "dravatar.h"

dravatar_pin_t pins;

int robot_init()
{
	wiringPiSetup();
	int temp_pin = 0;

	temp_pin = piPin_to_wiringPipin(ENABLE_PIN);
	if( temp_pin == -1 )
	{
		#ifdef DEBUG
			printf("Enable pin error!\n");
		#endif
		return -1;
	}
	pins.enable = temp_pin;

	temp_pin = piPin_to_wiringPipin(LED_PIN);
	if( temp_pin == -1 )
	{
		#ifdef DEBUG
			printf("Led pin error!\n");
		#endif
		return -1;
	}
	pins.led = temp_pin;

	temp_pin = piPin_to_wiringPipin(LEFT_WHEELS_FORWARD_PIN);
	if( temp_pin == -1 )
	{
		#ifdef DEBUG
			printf("Left Forward Wheels pin error!\n");
		#endif
		return -1;
	}
	pins.left_wheels_forward = temp_pin;

	temp_pin = piPin_to_wiringPipin(LEFT_WHEELS_BACKWARD_PIN);
	if( temp_pin == -1 )
	{
		#ifdef DEBUG
			printf("Left Backward Wheels pin error!\n");
		#endif
		return -1;
	}
	pins.left_wheels_backward = temp_pin;

	temp_pin = piPin_to_wiringPipin(RIGHT_WHEELS_FORWARD_PIN);
	if( temp_pin == -1 )
	{
		#ifdef DEBUG
			printf("Right Forward Wheels pin error!\n");
		#endif
		return -1;
	}
	pins.right_wheels_forward = temp_pin;

	temp_pin = piPin_to_wiringPipin(RIGHT_WHEELS_BACKWARD_PIN);
	if( temp_pin == -1 )
	{
		#ifdef DEBUG
			printf("Right Backward Wheels pin error!\n");
		#endif
		return -1;
	}
	pins.right_wheels_backward = temp_pin;


	pinMode(pins.led, OUTPUT);

	pinMode(pins.enable, OUTPUT);

	pinMode(pins.left_wheels_forward, OUTPUT);
	pinMode(pins.left_wheels_backward, OUTPUT);
	pinMode(pins.right_wheels_forward, OUTPUT);
	pinMode(pins.right_wheels_backward, OUTPUT);

	if( ax_init(&(pins.enable), BAUD) == -1 )
	{
		#ifdef DEBUG
			printf("Error: ax_init return -1\n");
		#endif
		return -1;
	}

	digitalWrite(pins.led, HIGH);

	digitalWrite(pins.left_wheels_forward, LOW);
	digitalWrite(pins.left_wheels_backward, LOW);
	digitalWrite(pins.right_wheels_forward, LOW);
	digitalWrite(pins.right_wheels_backward, LOW);

	return 0;
}

int rotate(char id, int angle_degree, int speed)
{
	int pos_hex = 0;
	int speed_hex = 0;
	int result = -1;

	if( angle_degree < 0 )
		return -1;

	pos_hex = deg_to_hex((int)angle_degree);
	if( pos_hex > GOAL_POSITION_HEX_MAX )
		pos_hex = GOAL_POSITION_HEX_MAX;

	speed_hex = rpm_to_hex(speed);
	if( speed_hex > MOVING_SPEED_HEX_MAX )
		speed_hex = MOVING_SPEED_HEX_MAX;

	if( speed > 0 )
		result = set_goal_position(id, &pos_hex, &speed_hex);
	else
		result = set_goal_position(id, &pos_hex, NULL);

	return result;
}
int change_speed(char id, int speed)
{
	int hex = 0;
	int result = -1;

	if( speed < 0 )
		return -1;

	if( speed > MOVING_SPEED_MAX_RPM )
		speed = MOVING_SPEED_MAX_RPM;
	hex = rpm_to_hex(speed);

	if( hex > MOVING_SPEED_HEX_MAX )
		hex = MOVING_SPEED_HEX_MAX;

	result = set_moving_speed(id, &hex);

	return result;
}

int led(char id, char state)
{
	if( id <= 0 || id > ID_ALL )
		return -1;
	ax_led(id, state);
	return 0;
}

int calculate_speed_linear_by_time(int positionA, int positionB)
{
	int distance = abs( positionB - positionA );

	int speed =  500 * distance / (3 * SOCKET_RECEPTION_TIME);

	//speed = log((double)speed) / 2;
	if( speed > MOVING_SPEED_MAX_RPM )
		speed = MOVING_SPEED_MAX_RPM;

	return speed;
}


int move_robot(int move_instruction)
{
	if (move_instruction == MOVE_STOP ) // stop robot
	{
		digitalWrite( pins.left_wheels_forward, LOW );
		digitalWrite( pins.right_wheels_forward, LOW );
		digitalWrite( pins.left_wheels_backward, LOW );
		digitalWrite( pins.right_wheels_backward, LOW );
		return 0;
	}

	else if (move_instruction == MOVE_FORWARD) // move robot forward
	{
		digitalWrite( pins.left_wheels_forward, HIGH );
		digitalWrite( pins.right_wheels_forward, HIGH );
		digitalWrite( pins.left_wheels_backward, LOW ); 
		digitalWrite( pins.right_wheels_backward, LOW );
		return 0;		
	}

	else if (move_instruction == MOVE_BACKWARD) // move robot backward
	{
		digitalWrite( pins.left_wheels_forward, LOW );
		digitalWrite( pins.right_wheels_forward, LOW );
		digitalWrite( pins.left_wheels_backward, HIGH );
		digitalWrite( pins.right_wheels_backward, HIGH );
		return 0;
	}

	else if (move_instruction == MOVE_LEFT) // rotate robot left
	{
		digitalWrite( pins.left_wheels_forward, LOW );
		digitalWrite( pins.left_wheels_backward, HIGH );
		digitalWrite( pins.right_wheels_forward, HIGH );
		digitalWrite( pins.right_wheels_backward, LOW );
		return 0;
	}

	else if (move_instruction == MOVE_RIGHT) // rotate robot right
	{
		digitalWrite( pins.left_wheels_forward, HIGH );
		digitalWrite( pins.left_wheels_backward, LOW );
		digitalWrite( pins.right_wheels_forward, LOW );
		digitalWrite( pins.right_wheels_backward, HIGH );
		return 0;
	}

	return -1;
}

int blink_led(int blink_loop)
{
	int i = 0;

	if( blink_led <= 0 )
	{
		#ifdef DEBUG
			printf("Led_blink: Cannot put less than 1 blink_loop\n");
			return -1;
		#endif
	}

	// Blink part
	for(i=0;i<blink_loop;i++)
	{
		digitalWrite(pins.led, LOW);
		delay(BLINK_TIME);	// dravatar.h
		digitalWrite(pins.led, HIGH);
		delay(BLINK_TIME);
	}

	return 0;
}

void dravatar_close()
{
	// Put all the pins to LOW state
	digitalWrite(pins.led, LOW);
	digitalWrite(pins.left_wheels_forward, LOW);
	digitalWrite(pins.left_wheels_backward, LOW);
	digitalWrite(pins.right_wheels_forward, LOW);
	digitalWrite(pins.right_wheels_backward, LOW);

	// Close the ax12
	ax_close();
}