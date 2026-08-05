#include <string.h>
#include "map.h"
#include "mem.h"
#include "data.h"
#include "util.h"
#include "random.h"
#include "server.h"
#include "platform.h"
#include "resources.h"

Server server = {
	.running   = false,
	.mapPath   = NULL,
	.netSock   = NULL,
	.udpSock   = NULL,
	.localSock = NULL
};

ServerConfig serverConf = {
	.inet         = false,
	.inetPort     = 2025,
	.local        = false,
	.allowTCPOnly = false
};

bool Server_Start(void) {
	server.running    = true;
	server.clients    = NULL;
	server.clientsNum = 0;

	// commented until it actually gets used
	// #ifdef AE_RANDOM_STD
	// 	Log("##### WARNING: Using insecure random number generator");
	// #endif

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
			Log("Failed to open TCP internet socket");
			return false;
		}

		if (!Socket_Bind(server.netSock, (uint16_t) serverConf.inetPort)) {
			Log("Failed to bind TCP internet socket");
			return false;
		}

		if (!Socket_Listen(server.netSock, 10)) {
			Log("Failed to listen on TCP internet socket");
			return false;
		}

		server.udpSock = Socket_New(SOCKET_TYPE_NET, SOCKET_PROTOCOL_UDP);

		if (!server.udpSock) {
			Log("Failed to open UDP internet socket");
			return false;
		}

		if (!Socket_Bind(server.udpSock, (uint16_t) serverConf.inetPort)) {
			Log("Failed to bind UDP internet socket");
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
	if (server.netSock) {
		Socket_Close(server.netSock);
		server.netSock = NULL;
	}

	if (server.localSock) {
		Socket_Close(server.localSock);
		server.localSock = NULL;
	}

	if (server.clients) {
		ServerClient* client = server.clients;

		while (client) {
			ServerClient* next = client->next;

			Socket_Close(client->relSock);
			free(client);

			client = next;
		}

		server.clients = NULL;
	}

	if (server.mapPath) {
		free(server.mapPath);
		server.mapPath = NULL;
	}

	server.clientsNum = 0;
	server.running    = false;
	server.mapPath    = NULL;
}

enum {
	SC_WAITING = 0, // waiting for a new packet
	SC_PACKET,      // waiting for the current packet to be finished
	SC_MAP
};

static bool ClientSendMap(ServerClient* this) {
	uint16_t id = 0x01;
	Socket_Send(this->relSock, &id, sizeof(id));

	char mapName[64] = "map.arm";
	Socket_Send(this->relSock, mapName, sizeof(mapName));

	bool success;
	this->mapStream = Resources_Open(server.mapPath, &success, false);

	if (!success) {
		Log("Failed to open map");
		return false;
	}

	uint32_t mapSize = (uint32_t) Stream_Size(&this->mapStream);
	Socket_Send(this->relSock, &mapSize, sizeof(mapSize));

	this->relState = SC_MAP;

	Log("server: Begin sending map");
	return true;
}

static void KickClient(ServerClient* this, const char* message) {
	char packet[258];
	packet[0] = 0x04; // packet ID
	packet[1] = 0;
	strncpy(&packet[2], message, 256);
	Socket_Send(this->relSock, packet, sizeof(packet));
}

static bool HandleSharedPacket(ServerClient* this, uint8_t* packet, size_t sz) {
	if (sz < 2) return true;

	uint16_t id = Data_Read16(packet);

	switch (id) {
		case 0xFF00: { // my current position
			if (sz < 30) {
				return true;
			}

			uint32_t movement = Data_Read32(&packet[2]);

			if (movement < this->movement) {
				break;
			}
			this->movement = movement;

			uint32_t sector = Data_Read32(&packet[6]);
			float    x      = Data_ReadFloat(&packet[10]);
			float    y      = Data_ReadFloat(&packet[14]);
			float    z      = Data_ReadFloat(&packet[18]);
			float    yaw    = Data_ReadFloat(&packet[22]);
			float    pitch  = Data_ReadFloat(&packet[26]);

			if (sector >= map.sectorsLen) {
				this->kickFlag   = true;
				this->kickReason = "Broken movement packet"; // TODO: implement
				Log("server: Kicking '%s' for broken movement packet", this->username);
				return false;
			}

			this->pos.x  = x;
			this->pos.y  = y;
			this->pos.z  = z;
			this->yaw    = yaw;
			this->pitch  = pitch;
			this->sector = sector;

			Log("server: %s is at %g,%g,%g", this->username, x, y, z);
			return true;
		}
		default: {
			abort();
		}
	}

	return true;
}

static bool ClientWorker(ServerClient* this) {
	if (
		(this->relSock->value.type == SOCKET_TYPE_NET) &&
		(Platform_GetTime() - this->lastPing > 30000000)
	) {
		// send kick message in case they are still connected, which probably won't
		// be the case
		KickClient(this, "Automatically disconnected - no ping for over 30 seconds");
		return false;
	}

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
				case 0x00: { // identification
					size_t size = 32 + 2 + 2;

					if (available < size) break;

					uint16_t version;
					Socket_Receive(this->relSock, &version, 2);

					if (version != 0) {
						Log("server: Client is on incompatible version");
						return false;
					}

					Socket_Receive(this->relSock, &this->udpPort, 2);

					char username[33];
					username[32] = 0;
					Socket_Receive(this->relSock, &username, 32);
					Log("server: %s (%d) has joined the server", username, (int) this->udpPort);

					strcpy(this->username, username);

					// now send response
					uint16_t id             = 0; // packet id
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
				case 0x02: { // chat
					size_t size = 128;

					if (available < size) break;

					char message[129];
					message[128] = 0;
					Socket_Receive(this->relSock, &message, 128);

					char* msg = Format("%s: %s", this->username, message);
					Server_SendMessage(msg);
					free(msg);

					this->relState = SC_WAITING;
					break;
				}
				case 0x03: { // ping
					this->lastPing = Platform_GetTime();
					this->relState = SC_WAITING;
					break;
				}
				case 0xFF00: { // my current position
					size_t size = 28;

					if (available < size) break;

					uint8_t packet[30];
					packet[0] = 0x00;
					packet[1] = 0xFF;

					Socket_Receive(this->relSock, &packet[2], 28);
					this->relState = SC_WAITING;

					return HandleSharedPacket(this, packet, sizeof(packet));
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

			Log("server: Sent %d bytes to client", (int) size);

			Socket_Send(this->relSock, chunk, size);

			if (size != 1024) {
				Log("server: Finished sending map");
				Stream_Close(&this->mapStream);
				this->relState = SC_WAITING;
			}
			break;
		}
	}

	return true;
}

static void RemoveClient(ServerClient* this) {
	if (this->prev) this->prev->next = this->next;
	if (this->next) this->next->prev = this->prev;

	Socket_Close(this->relSock);
	free(this);

	-- server.clientsNum;
}

static void HandleUDP(ServerClient* this, uint8_t* packet, size_t sz) {
	if (sz < 2) return;

	uint16_t id = Data_Read16(packet);

	switch (id) {
		case 0xFF00: { // my current position
			HandleSharedPacket(this, packet, sz);
			break;
		}
		default: {
			this->kickFlag   = true;
			this->kickReason = "Invalid packet ID";

			Log("server: %s sent invalid packet ID %.2x", id);
			break;
		}
	}
}

void Server_Update(void) {
	if (server.udpSock) {
		for (size_t i = 0; i < server.clientsNum + 1; ++ i) {
			if (!Socket_IsDataAvailable(server.udpSock)) break;

			uint8_t       packet[SOCKET_UDP_DATA_SIZE];
			NetSocketAddr addr;

			size_t sz = Socket_ReceiveUDP(server.udpSock, packet, sizeof(packet), &addr);

			if (sz == 0) {
				Log("server: Failed to read from UDP socket");
			}
			if (sz < 2) {
				break;
			}
			size_t dataSize = Data_Read16(packet);

			ServerClient* client = server.clients;

			while (client) {
				if (client->relSock->value.type != SOCKET_TYPE_NET) {
					client = client->next;
					continue;
				}

				NetSocketAddr addr2;

				// TODO: this is very wrong
				if (!Socket_GetAddr(client->relSock, &addr2)) {
					Log("Failed to get %s's address", client->username);
				}

				ServerClient* next = client->next;

				if (NetSocketAddr_Compare(&addr, &addr2)) {
					HandleUDP(client, packet, sz < dataSize? sz : dataSize);

					if (client->kickFlag) {
						RemoveClient(client);
					}

					break;
				}

				client = next;
			}
		}
	}

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

		ServerClient* client = SafeMalloc(sizeof(ServerClient));
		client->prev         = NULL;

		if (server.clients) {
			client->next         = server.clients;
			server.clients->prev = client;
			server.clients       = client;
		}
		else {
			server.clients = client;
			client->next   = NULL;
		}

		client->relState = SC_WAITING;
		client->relSock  = newClient;
		client->lastPing = Platform_GetTime();
		client->kickFlag = false;
		client->movement = 0;

		++ server.clientsNum;
	}

	ServerClient* client = server.clients;
	while (client) {
		if (ClientWorker(client)) {
			client = client->next;
		}
		else {
			char ip[64];
			Socket_StringAddr(client->relSock, ip, sizeof(ip));

			Log("Client %s disconnected", ip);

			ServerClient* removed = client;

			client = client->next;

			if (server.clients == removed) {
				server.clients = removed->next;
			}

			if (removed->prev) removed->prev->next = removed->next;
			if (removed->next) removed->next->prev = removed->prev;

			Socket_Close(removed->relSock);
			free(removed);

			-- server.clientsNum;
		}
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

static void MsgPacket(const char* data) {
	uint8_t packet[66];
	packet[0] = 0x02;
	packet[1] = 0;
	strncpy((char*) &packet[2], data, 64);

	ServerClient* client = server.clients;
	while (client) {
		Socket_Send(client->relSock, packet, sizeof(packet));

		client = client->next;
	}
}

void Server_SendMessage(const char* message) {
	size_t len   = strlen(message);
	size_t times = (len / 64) + ((len % 64)? 1 : 0);

	for (size_t i = 0; i < times; ++ i, message += 64) {
		MsgPacket(message);
	}
}
