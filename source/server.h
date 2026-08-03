#ifndef AE_SERVER_H
#define AE_SERVER_H

#include "common.h"
#include "socket.h"
#include "stream.h"

typedef struct ServerClient ServerClient;

struct ServerClient {
	int      relState;
	Socket*  relSock;
	uint64_t lastPing;
	uint16_t udpPort; // set to 0 if not used

	// state stuff
	uint16_t packetID;
	Stream   mapStream;

	// info
	char username[33];

	// link
	ServerClient* next;
	ServerClient* prev;
};

typedef struct {
	bool inet;
	int  inetPort;
	bool local;
	bool allowTCPOnly;
} ServerConfig;

typedef struct {
	bool  running;
	char* mapPath;

	// server sockets
	Socket* netSock;
	Socket* udpSock;
	Socket* localSock;

	// clients
	ServerClient* clients;
} Server;

extern Server       server;
extern ServerConfig serverConf;

bool Server_Start(void);
void Server_Free(void);
void Server_Update(void);
void Server_SetMap(const char* name);
void Server_SendMessage(const char* message);

#endif
