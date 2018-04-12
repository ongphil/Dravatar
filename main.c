#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;


#include "ax12.h"
#include "dravatar.h"
#include "protocol.h"

#define MSG_SIZE_MAX 1000

int PORT = 8888;
char id_ud = ID_UP_DOWN;
char id_lr = ID_LEFT_RIGHT;


pthread_t thread_listener;
pthread_t thread_wheels;
pthread_t thread_head;

int head_socket = -2;//-1;
int wheels_socket = -2;//-1;

pthread_mutex_t head_socket_mutex;
pthread_mutex_t wheels_socket_mutex;

// Function waiting the connection of a client
// int my_socket : the socket we have to listen
// Return the socket value of the client connected
int listening_to_client_connection(int my_socket)
{
	int sockaddr_size = 0;
	int client_socket = -1;
	SOCKADDR_IN client;
	#ifdef DEBUG
		puts("Listening ...");
	#endif
	listen(my_socket, 2);
	sockaddr_size = sizeof(struct sockaddr_in);

	client_socket = accept(my_socket, (struct sockaddr *)&client,
			(socklen_t*)&sockaddr_size);
	if (client_socket < 0)
	{
		perror("accept failed");
		return -1;
	}
	#ifdef DEBUG
		puts("Connection accepted");
	#endif
	return client_socket;
}

// Function thread to control the head of the robot (AX12)
// void* data : unused
void* thread_move_head(void* data)
{
	int read_size = -1;
	char message[MSG_SIZE_MAX] = {'\0'};
	char list_messages[MSG_RECEIVED_MAX][MSG_POSITION_LEN];
	int nb_messages = -1;
	int i = 0;
	position_t position;
	position_t last_position;

	int start_x = 0;
	int start_z = 0;
	int sumx = 0;
	int sumz = 0;
	int first_receptions = 0;

	char end = 0;

	while( end == 0 )
	{
		// Vérify if the socket is available
		if( pthread_mutex_lock(&head_socket_mutex) == 0 )
		{
			if( head_socket != -1 )
			{

				rotate(id_lr, 150, RETURN_SPEED);
				rotate(id_ud, 150, RETURN_SPEED);
				last_position.x = 150;
				last_position.y = 0;
				last_position.z = 150;

				first_receptions = 0;
				sumx = 0;
				sumz = 0;
				#ifdef DEBUG
					printf("Please keep the head straight\nCalibrating...\n");
				#endif

				while( first_receptions < 100 )
				{
					read_size = recv(head_socket , message , MSG_SIZE_MAX, 0);
					if( (nb_messages = divide_messages(message, read_size, list_messages)) > 0 )
					{
						for(i=0;i<nb_messages;i++)
						{
							if( parse_message_position(list_messages[i], &position) == 0 )
							{
								sumx += position.x;
								sumz += position.z;
								first_receptions++;
							}
						}
					}
				}
				pthread_mutex_unlock(&head_socket_mutex);
				start_x = sumx/first_receptions;
				start_z = sumz/first_receptions;

				while( read_size != 0 )
				{
					if( pthread_mutex_lock(&head_socket_mutex) == 0 )
					{
						read_size = recv(head_socket , message , MSG_SIZE_MAX, 0);
						pthread_mutex_unlock(&head_socket_mutex);
					}
					
					if( read_size > 0 )
					{
						int nb_messages = divide_messages(message, read_size, list_messages);
						if( nb_messages > 0 )
						{
							if( parse_message_position(list_messages[nb_messages-1], &position) == 0 )
							{
								// Calibration of x
								position.x = position.x - start_x + 150;
								if( position.x < 0 )
									position.x += 360;
								else if( position.x > 360 )
									position.x -= 360;

								if( position.x > 300 )
									position.x = 300;
								else if( position.x < 0 )
									position.x = 0;
								// Invert sens for x
								position.x = 300 - position.x;


								// Calibration of z
								position.z = position.z - start_z + 150;
								if( position.z < 0 )
									position.z += 360;
								else if( position.z > 360 )
									position.z -= 360;

								if( position.z > 300 )
									position.z = 300;
								else if( position.z < 0 )
									position.z = 0;


								// Limitation of X
								if( position.x > 210 )
									position.x = 210;
								else if( position.x < 90 )
									position.x = 90;

								// Limitation of Z
								if( position.z < 60 )
									position.z = 60;
								else if( position.z >180 )
									position.z = 180;

/*
								// Calculation of the speed
								float speedX = calculate_speed_linear_by_time(last_position.x, position.x);
								//float speedY = calculate_speed_linear_by_time(last_position.y, position.y);
								float speedZ = calculate_speed_linear_by_time(last_position.z, position.z);
*/
								float speedX = 20;
								float speedZ = 25;
								

								#ifdef DEBUG
									display_position(position);
								#endif

								// Rotation of the AX12
								rotate(ID_LEFT_RIGHT, position.x, speedX);
								rotate(ID_UP_DOWN, position.z, speedZ);

								// Last position updated
								last_position.x = position.x;
								last_position.y = position.y;
								last_position.z = position.z;
							}
						}
					}
				}

				if(read_size == 0)
			    {
					puts("Client disconnected");
					fflush(stdout);
					if( pthread_mutex_lock(&head_socket_mutex) == 0 )
					{
						head_socket = -1;// reset the socket
						pthread_mutex_unlock(&head_socket_mutex);
					}
					end = 1;
				} 
				else if(read_size == -1)
				{
					perror("recv failed");
				}
			}
			pthread_mutex_unlock(&head_socket_mutex);
		}
	}
	return NULL;
}

void* thread_wheels_timer(void* data)
{
	clock_t start = clock(), time = start;
	while( time - start < (CLOCKS_PER_SEC) )
		time = clock();
	move_robot(MOVE_STOP);
	return NULL;
}

// Function of the thread that control the wheels
// void* data : unused
void* thread_move_wheels(void* data)
{
	int read_size = -1;
	char move_message[MSG_SIZE_MAX] = {'\0'};
	char list_move[MSG_RECEIVED_MAX][MSG_MOVE_LEN];
	int i = 0;
	int move_instruction = 0;
	char end = 0;


	while( end == 0 )
	{
		if( pthread_mutex_lock(&wheels_socket_mutex) == 0 )
		{
			//if( wheels_socket != -1 )
			if( wheels_socket >= 0 )
			{
				read_size = recv(wheels_socket , move_message , MSG_SIZE_MAX, 0);
				pthread_mutex_unlock(&wheels_socket_mutex);
				if( read_size > 0 )
				{
					int nb_messages = divide_messages(move_message, read_size, list_move);
					if( nb_messages > 0 )
					{
						for(i=0;i<nb_messages;i++)
						{
							if( parse_message_move(list_move[i], &move_instruction) == 0 )
							{
								// Change the state of the wheels
								move_robot(move_instruction);
							}
						}
					}
				}
				else if(read_size == 0)
			    {
					puts("Client disconnected");
					fflush(stdout);
					wheels_socket = -1;// reset the socket
					end = 1;
					move_robot(MOVE_STOP);
				} 
				else if(read_size == -1)
				{
					perror("recv failed");
				}
			}
			pthread_mutex_unlock(&wheels_socket_mutex);
		}
	}
	return NULL;
}

// Function of the thread waiting for connections
// void* data : address of the socket we are listening
void* thread_listening(void* data)
{
	int my_socket = *(int*)data;
	int client_socket = -1;
	char first_message[MSG_SIZE_MAX] = {'\0'};
	char list[MSG_RECEIVED_MAX][MSG_POSITION_LEN];
	int read_size = -1;
	int nb_messages = -1;
	char client_is_identified = 0;
	char type_message = -1;
	int i = 0;
	//clock_t start, timer;
	while(1)
	{
		client_is_identified = 0;
		type_message = -1;
		client_socket = listening_to_client_connection(my_socket);
		if( client_socket != -1 )
		{
			while( client_is_identified == 0 )
			{
				// Receive a first message for identification
				read_size = recv(client_socket , first_message , MSG_SIZE_MAX, 0);
				if( (nb_messages = divide_messages(first_message, read_size, list)) > 0 )
				{
					for(i=0;i<nb_messages;i++)
					{
						type_message = list[i][2];
						if( type_message ==  POSITION ||
							type_message == MOVE )
						{
							client_is_identified = 1;
							break;
						}
					}
				}
				if( read_size == 0 )
				{
					client_is_identified = -1;
					type_message = -1;
				}
			}
				
			switch(type_message)
			{
				case POSITION:	// The new client wants to control the head
								// Utilization of mutex for head
								if( pthread_mutex_lock(&head_socket_mutex) == 0 )
								{
									// Verify which socket is available
									if( head_socket < 0 )
									{
										// = -2 no thread activated
										// = -1 thread waiting for a join
										if( head_socket == -1 )
										{
											if( pthread_join(thread_head, NULL) != 0 )
												pthread_cancel(thread_head);
										}
										if( pthread_create(&thread_head, NULL, thread_move_head, NULL) != 0 )
										{
											head_socket = -2;
											#ifdef DEBUG
												puts("Error: Thread for head creation fail !\n");
											#endif
										}
										else
										{
											head_socket = client_socket;
											blink_led(5);
										}
									}
									// Head already controlled
									else
									{
										closesocket(client_socket);
									}
									


									pthread_mutex_unlock(&head_socket_mutex);
								}
								break;

				case MOVE:		// The new client wants to control the wheels
								// Utilization of mutex for wheels
								if( pthread_mutex_lock(&wheels_socket_mutex) == 0 )
								{
									// Verify which socket is available
									//if( wheels_socket == -1 )
									if( wheels_socket < 0 )
									{
										// = -2 no thread activated
										// = -1 thread waiting for a join
										if( wheels_socket == -1 )
										{
											if( pthread_join(thread_wheels, NULL) != 0 )
												pthread_cancel(thread_wheels);
										}
										if( pthread_create(&thread_wheels, NULL, thread_move_wheels, NULL) != 0 )
										{
											wheels_socket = -2;
											#ifdef DEBUG
												puts("Error: Thread for wheels creation fail !\n");
											#endif
										}
										else
										{
											wheels_socket = client_socket;
											blink_led(5);
										}
									}
									// Refuse if already used
									// Head already controlled
									else
									{
										closesocket(client_socket);
									}

									pthread_mutex_unlock(&wheels_socket_mutex);
								}
								break;
				default:
								closesocket(client_socket);
								break;
			}
			client_socket = -1;
		}
	}
}


int main(int argc, char* argv[])
{
	int my_socket = -1;
	SOCKADDR_IN server;

	char end_string[100];

	// Robot initialization
	if( robot_init() == -1 )
	{
		#ifdef DEBUG
		printf("Error: Robot_init failed\n");
		#endif
		return -1;
	}

	// Creation of the mutex for the head socket
	if( pthread_mutex_init(&head_socket_mutex, NULL) != 0 )
	{
		#ifdef DEBUG
			puts("Error : Mutex_init for head failed");
		#endif
		return -1;
	}

	// Creation of the mutex for the wheels
	if( pthread_mutex_init(&wheels_socket_mutex, NULL) != 0 )
	{
		pthread_mutex_destroy(&head_socket_mutex);
		#ifdef DEBUG
			puts("Error : Mutex_init for wheels failed");
		#endif
		return -1;
	}

	// Creation of the socket for the users
	my_socket = socket(AF_INET , SOCK_STREAM , 0);
	if( my_socket == -1 )
	{
		#ifdef DEBUG
		printf("Error: Socket creation failed\n");
		#endif
		return -1;
	}
	#ifdef DEBUG
	puts("Socket creation: OK");
	#endif
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );
	if( bind(my_socket,(struct sockaddr *)&server , sizeof(server)) < 0) 
	{
		printf("Error: Bind failed\n");
		return 1;
	}
	puts("Bind: OK");

	// Thread waiting for connections
	// It will create the 2 others thread when needed (head + wheels)
	if( pthread_create(&thread_listener, NULL, thread_listening, &my_socket) != 0 )
	{
		pthread_mutex_destroy(&head_socket_mutex);
		pthread_mutex_destroy(&wheels_socket_mutex);
		#ifdef DEBUG
			puts("Error: Thread listener creation fail !\n");
		#endif
		return -1;
	}

	// Initialization of the position
	rotate(id_lr, 150, RETURN_SPEED);
	rotate(id_ud, 150, RETURN_SPEED);

	// Blink the led 5 times then stay ON
	blink_led(5);

	// Wait the word "end" to close the program
	while(strcmp(end_string,"end")!=0)
	{
		printf("Type to \"end\" quit program properly\n");
		scanf("%s", end_string);
		fflush(stdin);
	}

	// Back to the initial position
	rotate(id_lr, 150, RETURN_SPEED);
	rotate(id_ud, 150, RETURN_SPEED);
	move_robot(MOVE_STOP);

	// Free memory correctly
	pthread_mutex_destroy(&head_socket_mutex);
	pthread_mutex_destroy(&wheels_socket_mutex);
	pthread_cancel(thread_listener);
	pthread_cancel(thread_wheels);
	pthread_cancel(thread_head);
	closesocket(head_socket);
	closesocket(wheels_socket);
	dravatar_close();

	return 0;
}
