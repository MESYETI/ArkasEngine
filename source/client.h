#ifndef AE_CLIENT_H
#define AE_CLIENT_H

#include "socket.h"

typedef struct {
	Socket* relSocket;
} Client;

extern Client client;

bool Client_StartLocal(void);
bool Client_StartINet(const char* ip, uint16_t port);

#endif
