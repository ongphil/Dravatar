#include "protocol.h"

void display_position(position_t pos)
{
	printf("Position:\n");
	printf("x = %d\ny = %d\nz = %d\n\n", pos.x, pos.y, pos.z);
}


//verify the checksum
int check_checksum(char* message)
{
	int i = 0;
	int len = 0; 
	int sum = 0;
	char result = 0;
	int success = 0;

	if( message[2] == POSITION ) 	//if message's type is position 
		len = 7;					//length is always 7
	else if( message[2] ==  MOVE)
		len = 2;

	if( len == 0 )
		return -1;

	for(i=0;i<len;i++)
	{
		sum += message[i+2];		//compute the checksum
	}

	result = (char)sum & 0xFF;

	if( result == message[i+2] )     	//compare sum and checksum to ensure message integrity
		success = 0;
	else
		success = -1;

	return success;
}

//divide the received socket into differrent messages
int divide_messages(char* messages,
					int read_size,
					char list[MSG_RECEIVED_MAX][MSG_POSITION_LEN])
{
	char message[20];
	int i = 0, j = 0, i_list = 0;
	
	if( messages == NULL )
		return -1; 

	if( read_size == 0 )
		return -1;


	for(i=0;i<read_size;i++)								//for each charachter in the received socket
	{
		if( messages[i] == 0xFF && messages[i+1] == 0xFF)  	//first to character of a message
		{
			switch(messages[i+2])  							//test for types
			{
				case POSITION: 								//for position type
					for(j=0;j<MSG_POSITION_LEN;j++)
						message[j] = messages[i+j];

					if( check_checksum(message) == 0 )		//if checksum is correct
					{
						for(j=0;j<MSG_POSITION_LEN;j++)
							list[i_list][j] = message[j];	//separate each message
						i_list++;
					}
					break;

				case MOVE:
					for(j=0;j<MSG_MOVE_LEN;j++)
						message[j] = messages[i+j];

					if( check_checksum(message) == 0 )		//if checksum is correct
					{
						for(j=0;j<MSG_MOVE_LEN;j++)
							list[i_list][j] = message[j];	//separate each message
						i_list++;
					}


				default:break;
			}
		}
	}

	return i_list;
}

int parse_message_position(char* message, position_t* pos)
{
	if( message == NULL )
		return -1;

	if( pos == NULL )
		return -1;

	if( message[0] != 0xFF && message[1] != 0xFF )
		return -1;

	if( message[2] != POSITION )  
		return -1;

	//if message is correct do :

	pos->x = (message[3] << 8) | message[4] ;  //cast the two first bytes into an integer
	pos->y = (message[5] << 8) | message[6];  //cast the two second bytes into an integer
	pos->z = (message[7] << 8) | message[8];  //cast the two third bytes into an integer

	return 0;
}

int parse_message_move(char* message, int* move_instruction)
{
	if( message == NULL )
		return -1;

	if( message[0] != 0xFF && message[1] != 0xFF )
		return -1;

	if( message[2] != MOVE )  
		return -1;

	//if message is correct do :

	*move_instruction = message[3];

	return 0;
}
