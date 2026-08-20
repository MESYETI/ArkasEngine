#ifndef AE_SERVER_H
#define AE_SERVER_H

#include "common.h"
#include "socket.h"
#include "stream.h"

typedef struct ServerClient ServerClient;

struct ServerClient {
	// network
	int      relState;
	Socket*  relSock;
	uint64_t lastPing;
	uint16_t udpPort; // set to 0 if not used
	uint32_t sessionID;

	// position
	FVec3  pos;
	float  yaw;
	float  pitch;
	size_t sector;

	// state stuff
	uint16_t    packetID;
	Stream      mapStream;
	bool        kickFlag;
	const char* kickReason;
	uint32_t    movement;

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
	size_t        clientsNum;

	// callbacks
	// must not be NULL if the engine is compiled with AE_NO_BASIC_PLAYER_ENT
	size_t (*createPlayerEntity)(ServerClient* client);
} Server;

extern Server       server;
extern ServerConfig serverConf;

bool     Server_Start(void);
void     Server_Free(void);
void     Server_Update(void);
uint32_t Server_GenID(void);
void     Server_SetMap(const char* name);
void     Server_SendMessage(const char* message);

#endif
