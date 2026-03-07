#ifndef AE_SERVER_H
#define AE_SERVER_H

#include "common.h"
#include "socket.h"

typedef struct {
	Socket* relSocket;
} ServerClient;

typedef struct {
	bool inet;
	int  inetPort;
	bool local;
} ServerConfig;

typedef struct {
	bool running;

	// server sockets
	Socket* netSock;
	Socket* localSock;

	// clients
	ServerClient* clients;
	size_t        clientNum;
} Server;

extern Server       server;
extern ServerConfig serverConf;

bool Server_Start(void);
void Server_Update(void);

#endif
