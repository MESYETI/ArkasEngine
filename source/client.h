#ifndef AE_CLIENT_H
#define AE_CLIENT_H

#include "socket.h"

typedef struct {
	bool     running;
	int      state;
	Socket*  relSock;
	Socket*  udpSock;
	char     name[32];
	uint32_t sessionID;

	// state stuff
	uint16_t packetID;
	uint32_t dataSize;
	size_t   dataRead;
	uint8_t* data;
	char     fileName[65];
	bool     downloading;
	uint64_t lastPing;
	uint32_t movement;
	bool     reportPos;
	uint32_t entityID;
	uint32_t compID;
} Client;

extern Client client;

void Client_Init(void);
bool Client_StartLocal(void);
bool Client_StartINet(const char* ip, uint16_t port);
bool Client_IsMultiplayer(void);
void Client_Stop(void);
void Client_Update(void);
void Client_SendMessage(const char* message);

#endif
