#include <string.h>
#include "mem.h"
#include "util.h"
#include "server.h"
#include "resources.h"

Server server = {
	.running   = false,
	.mapPath   = NULL,
	.netSock   = NULL,
	.localSock = NULL
};

ServerConfig serverConf = {
	.inet     = false,
	.inetPort = 2025,
	.local    = false
};

bool Server_Start(void) {
	server.running   = true;
	server.clients   = NULL;
	server.clientNum = 0;

	Log("SERVER CONFIG");
	Log("=============");
	Log("Internet:      %s", serverConf.inet? "enabled" : "disabled");
	if (serverConf.inet) {
		Log("Internet port: %d", serverConf.inetPort);
	}
	Log("Local:         %s", serverConf.local? "enabled" : "disabled");
	Log("=============");

	if (serverConf.inet) {
		Log("Opening internet socket...");
		server.netSock = Socket_New(SOCKET_TYPE_NET, SOCKET_PROTOCOL_TCP);

		if (!server.netSock) {
			Log("Failed to open internet socket");
			return false;
		}

		if (!Socket_Bind(server.netSock, (uint16_t) serverConf.inetPort)) {
			Log("Failed to bind internet socket");
			return false;
		}

		if (!Socket_Listen(server.netSock, 10)) {
			Log("Failed to listen on internet socket");
			return false;
		}

		Log("Listening on internet socket");
	}

	if (serverConf.local) {
		Log("Opening local socket...");
		server.localSock = Socket_New(SOCKET_TYPE_LOCAL, 0);

		if (!server.localSock) {
			Log("Failed to open local socket");
			return false;
		}

		if (!Socket_Bind(server.localSock, 0)) {
			Log("Failed to bind local socket");
			return false;
		}

		if (!Socket_Listen(server.localSock, 10)) {
			Log("Failed to listen on local socket");
			return false;
		}
	}

	Log("Server running");
	return true;
}

void Server_Free(void) {
	if (server.netSock)   Socket_Close(server.netSock);
	if (server.localSock) Socket_Close(server.localSock);

	for (size_t i = 0; i < server.clientNum; ++ i) {
		Socket_Close(server.clients[i].relSock);
	}
	free(server.clients);
}

enum {
	SC_WAITING = 0, // waiting for a new packet
	SC_PACKET,      // waiting for the current packet to be finished
	SC_MAP,
};

static bool ClientSendMap(ServerClient* this) {
	uint16_t id = 0x01;
	Socket_Send(this->relSock, &id, sizeof(id));

	char mapName[64];
	strncpy(mapName, server.mapPath, 64);
	Socket_Send(this->relSock, mapName, sizeof(mapName));

	bool success;
	this->mapStream = Resources_Open(server.mapPath, &success);

	if (!success) {
		Log("Failed to open map");
		return false;
	}

	uint32_t mapSize = (uint32_t) Stream_Size(&this->mapStream);
	Socket_Send(this->relSock, &mapSize, sizeof(mapSize));

	this->relState = SC_MAP;
	return true;
}

static bool ClientWorker(ServerClient* this) {
	switch (this->relState) {
		case SC_WAITING: {
			if (Socket_DataAvailable(this->relSock) < 2) break;

			Socket_Receive(this->relSock, &this->packetID, 2);
			this->relState = SC_PACKET;
			break;
		}
		case SC_PACKET: {
			size_t available = Socket_DataAvailable(this->relSock);

			switch (this->packetID) {
				case 0x00: {
					size_t size = 32 + 2;

					if (size != available) break;

					uint16_t version;
					Socket_Receive(this->relSock, &version, 2);

					if (version != 0) {
						Log("server: Client is on incompatible version");
						return false;
					}

					char username[33];
					username[32] = 0;
					Socket_Receive(this->relSock, &username, 32);
					Log("server: %s has joined the server", username);

					strcpy(this->username, username);

					// now send response
					uint16_t id             = 3;
					char     serverName[32] = "Arkas Engine Server";

					Socket_Send(this->relSock, &id, sizeof(id));
					Socket_Send(this->relSock, serverName, sizeof(serverName));

					// start sending map
					if (server.mapPath == NULL) {
						Log("server: warning: No map to send");
						this->relState = SC_WAITING;
					}
					else {
						if (!ClientSendMap(this)) {
							Log("server: Failed to send map");
							this->relState = SC_WAITING;
						}
					}
					break;
				}
				default: {
					Log("server: Client sent invalid packet ID: %.4x", this->packetID);
					return false;
				}
			}
			break;
		}
		case SC_MAP: {
			uint8_t chunk[1024];
			size_t  size;

			size = Stream_Read(&this->mapStream, 1024, &chunk);

			Socket_Send(this->relSock, chunk, size);

			if (size != 1024) {
				Log("server: Finished sending map");
				this->relState = SC_WAITING;
			}
			break;
		}
	}

	return true;
}

void Server_Update(void) {
	Socket* newClient = NULL;

	if (server.netSock) {
		newClient = Socket_Accept(server.netSock);
	}
	if (!newClient && server.localSock) {
		newClient = Socket_Accept(server.localSock);
	}

	if (newClient) {
		char addr[64];
		Socket_StringAddr(newClient, addr, sizeof(addr));
		Log("server: Client connected (%s)", addr);

		++ server.clientNum;
		server.clients = SafeRealloc(
			server.clients, server.clientNum * sizeof(ServerClient)
		);

		ServerClient* client = &server.clients[server.clientNum - 1];

		client->relState = SC_WAITING;
		client->relSock  = newClient;
	}

	for (size_t i = 0; i < server.clientNum; ++ i) {
		ServerClient* client = &server.clients[i];
		ClientWorker(client);
	}
}

void Server_SetMap(const char* name) {
	if (server.mapPath) {
		free(server.mapPath);
	}

	char* path     = ConcatString("maps:", name);
	server.mapPath = ConcatString(path, ".arm");
	free(path);
}
