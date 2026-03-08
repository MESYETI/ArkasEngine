#ifndef AE_CLIENT_H
#define AE_CLIENT_H

#include "socket.h"

typedef struct {
	bool    running;
	int     state;
	Socket* relSock;

	// state stuff
	uint16_t packetID;
} Client;

extern Client client;

bool Client_StartLocal(void);
bool Client_StartINet(const char* ip, uint16_t port);
void Client_Update(void);

#endif
