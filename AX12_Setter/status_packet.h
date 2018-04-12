#ifndef STATUS_PACKET_INCLUDE_H
#define STATUS_PACKET_INCLUDE_H

typedef struct status_packet
{
	char id;
	char len;
	char error;
	char param[20];
}packet_t;



#endif