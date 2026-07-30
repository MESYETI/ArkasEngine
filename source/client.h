#ifndef AE_CLIENT_H
#define AE_CLIENT_H

#include "socket.h"

typedef struct {
	bool    running;
	int     state;
	Socket* relSock;
	Socket* udpSock;
	char    name[32];

	// state stuff
	uint16_t packetID;
	char     fileName[65];
	uint32_t fileSize;
	uint8_t* fileContents;
	size_t   fileRead;
	bool     downloading;
	uint64_t lastPing;
} Client;

extern Client client;

bool Client_StartLocal(void);
bool Client_StartINet(const char* ip, uint16_t port);
void Client_Stop(void);
void Client_Update(void);
void Client_SendMessage(const char* message);

#endif
