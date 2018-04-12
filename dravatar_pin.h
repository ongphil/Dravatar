#ifndef DRAVATAR_PIN_INCLUDE_H
#define DRAVATAR_PIN_INCLUDE_H

#include <stdio.h>

typedef struct pin
{
	int enable;
	int led;
	int left_wheels_forward;
	int left_wheels_backward;
	int right_wheels_forward;
	int right_wheels_backward;
}dravatar_pin_t;

#endif