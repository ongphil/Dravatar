#include <stdio.h>
#include <stdlib.h>
#include "ax12.h"

#define ENABLE_PIN 		4
#define COMMUNICATION_BAUD_RATE 1000000
#define DRAVATAR_BAUD_RATE	57600

#define ID_OF_AX_TO_SET		0xFE
#define ID_UP_DOWN		11
#define ID_LEFT_RIGHT		12


int choice = -1;
int new_id = -1;
int baud_rate[9] = {1000000, 500000, 400000, 250000, 200000, 115200, 57600, 19200, 9600};


int main()
{
	int enable = piPin_to_wiringPipin(ENABLE_PIN);
	int i = 0;
	printf("Choose which AX12 to set: \n");
	printf("1) AX12: Move UP-DOWN\n");
	printf("2) AX12: Move LEFT-RIGHT\n");
	printf("0) Quit\n");

	while( choice != 1 && choice != 2 && choice != 0 )
	{
		printf("Choice: ");
		scanf("%d", &choice);
		fflush(stdin);
		printf("\n");
	}

	if( choice == 1 )
		new_id = ID_UP_DOWN;	
	else if( choice == 2 )
		new_id = ID_LEFT_RIGHT;
	else
	{
		printf("Quit\n");
		return 0;
	}

	
	for(i=0;i<9;i++)
	{
		//ax_init(&enable, COMMUNICATION_BAUD_RATE);
		ax_init(&enable, baud_rate[i]);
		ax_led(new_id, 0);
	
	
		set_id(ID_OF_AX_TO_SET, new_id);
		ax_led(new_id, 1);
		set_baud_rate(ID_OF_AX_TO_SET, 0x22);// 0x22 = 57600
		ax_close();
		delay(50);
	}
	printf("ID set to %d\nBaud rate set to %d\n", new_id, DRAVATAR_BAUD_RATE); 
	printf("If the led of the AX12 is ON, then the setting succeeds\n");
	return 0;
}
