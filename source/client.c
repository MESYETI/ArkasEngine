#include "mem.h"
#include "map.h"
#include "vfs.h"
#include "chat.h"
#include "data.h"
#include "util.h"
#include "event.h"
#include "client.h"
#include "entity.h"
#include "server.h"
#include "player.h"
#include "ramDrive.h"
#include "platform.h"

enum {
	C_IDENT = 0,  // about to identify
	C_WAITING,    // waiting for a packet
	C_PACKET,     // waiting for packet to finish sending
	C_FILE,       // receiving a file
	C_STOP,       // client stopped
	C_NEW_ENTITY, // new entity data available
	C_UPDATE_COMP // entity component data available 
};

Client client = {
	.running     = false,
	.state       = C_WAITING,
	.relSock     = NULL,
	.udpSock     = NULL,
	.downloading = false
};

void Client_Init(void) {
	strcpy(client.name, "Player");

	char digits[5];
	digits[4] = 0;

	for (int i = 0; i < sizeof(digits) - 1; ++ i) {
		digits[i] = (rand() % 10) + '0';
	}

	strcat(client.name, digits);
}

static void StartClient(void) {
	if (VFS_DriveExists("net")) {
		VFS_DeleteDrive("net");
	}
	VFS_AddDrive(NewRamDrive(), "net");

	Chat_Free();
	Chat_Init();

	client.sessionID = 0;
	client.reportPos = false;
}

bool Client_StartLocal(void) {
	if (!server.running || !server.localSock) {
		Log("client: Server not running or does not have local socket");
		return false;
	}

	client.relSock = Socket_New(SOCKET_TYPE_LOCAL, 0);
	client.udpSock = NULL;
	client.state   = C_IDENT;

	if (!client.relSock) {
		Log("client: Failed to open socket");
		return false;
	}

	if (!Socket_ConnectLocal(client.relSock, server.localSock)) {
		Log("client: Failed to connect");
		Socket_Close(client.relSock);
		client.relSock = NULL;
		return false;
	}

	Log("client: Client connected to local server");
	client.running = true;
	StartClient();
	return true;
}

bool Client_StartINet(const char* ip, uint16_t port) {
	client.relSock = Socket_New(SOCKET_TYPE_NET, SOCKET_PROTOCOL_TCP);
	client.udpSock = Socket_New(SOCKET_TYPE_NET, SOCKET_PROTOCOL_UDP);
	client.state   = C_IDENT;

	if (!client.relSock) {
		Log("client: Failed to open socket");
		return false;
	}

	if (!Socket_ConnectNet(client.relSock, ip, port)) {
		Log("client: Failed to connect");
		Socket_Close(client.relSock);
		client.relSock = NULL;
		return false;
	}

	if (!Socket_ConnectNet(client.udpSock, ip, port)) {
		Log("client: Failed to connect");
		Socket_Close(client.udpSock);
		client.udpSock = NULL;
		return false;
	}

	Log("client: Client connected to internet server");
	client.running = true;
	StartClient();

	client.lastPing = Platform_GetTime();
	return true;
}

bool Client_IsMultiplayer(void) {
	if (!client.relSock) {
		return false;
	}

	return client.relSock->value.type == SOCKET_TYPE_NET;
}

void Client_Stop(void) {
	if (client.relSock) {
		Socket_Close(client.relSock);
		client.relSock = NULL;
	}

	if (client.udpSock) {
		Socket_Close(client.udpSock);
		client.udpSock = NULL;
	}

	client.running = false;

	Log("Client stopped");
}

static void KickClient(const char* reason) {
	Log("client: %s", reason);
	Log("client: Stopping client");

	Event e;
	e.type = AE_EVENT_CLIENT_KICKED;

	strncpy(e.clientKick.message, reason, sizeof(e.clientKick.message) - 1);
	e.clientKick.message[sizeof(e.clientKick.message) - 1] = 0;

	Event_Add(&e);

	client.state = C_STOP;
}

void Client_Update(void) {
	if (!Socket_Connected(client.relSock)) {
		Log("client: Client sock not connected yet, waiting");
		return;
	}

	if (
		(client.relSock->value.type == SOCKET_TYPE_NET) &&
		(Platform_GetTime() - client.lastPing > 15000000)
	) {
		// send a ping
		uint16_t id = 0x03;

		Socket_Send(client.relSock, &id, sizeof(id));

		client.lastPing = Platform_GetTime();
	}

	switch (client.state) {
		case C_IDENT: {
			Socket_Send16(client.relSock, 0);
			Socket_Send16(client.relSock, 0);

			Socket_Send(client.relSock, client.name, sizeof(client.name));

			client.state = C_WAITING;
			Log("client: Sent identification");
			break;
		}
		case C_WAITING: {
			if (Socket_DataAvailable(client.relSock) < 2) break;

			client.packetID = Socket_Receive16(client.relSock);
			client.state    = C_PACKET;
			break;
		}
		case C_PACKET: {
			size_t available = Socket_DataAvailable(client.relSock);

			switch (client.packetID) {
				case 0x00: { // identification
					size_t size = 32 + 4;

					if (available < size) break;

					client.sessionID = Socket_Receive32(client.relSock);

					Log("client: ID: %u", client.sessionID);

					if (client.udpSock) {
						Socket_SetSessionID(client.udpSock, client.sessionID);
						Log("Set session ID on socket");
					}

					char name[33];
					name[32] = 0;
					Socket_Receive(client.relSock, &name, 32);
					Log("client: Server identifies as '%s'", name);

					client.state = C_WAITING;
					break;
				}
				case 0x01: { // receive file
					size_t size = 64 + 4;

					if (available < size) break;

					client.fileName[64] = 0;
					Socket_Receive(client.relSock, &client.fileName, 64);

					client.dataSize = Socket_Receive32(client.relSock);

					if (client.dataSize > 16777216) {
						KickClient("Server attempted to send a file size above the limit");
						break;
					}

					client.data     = SafeMalloc(client.dataSize);
					client.dataRead = 0;
					client.state    = C_FILE;

					Log("Receiving '%s' from server...", client.fileName);
					break;
				}
				case 0x02: { // chat
					size_t size = 64;

					if (available < size) break;

					char message[65];
					message[64] = 0;
					Socket_Receive(client.relSock, &message, 64);

					Log("MSG: %s", message);
					Chat_Add(message);

					client.state = C_WAITING;
					break;
				}
				case 0x03: { // ping
					client.state = C_WAITING;
					break;
				}
				case 0x04: { // kicked
					if (available < 256) break;

					char message[257];
					message[256] = 0;
					Socket_Receive(client.relSock, &message, 256);

					Event e;
					e.type = AE_EVENT_CLIENT_KICKED;
					strcpy(e.clientKick.message, message);
					Event_Add(&e);

					client.state = C_STOP;
					break;
				}
				case 0x05: { // new entity
					if (available < 4 + 2) break;

					uint32_t entityID = Socket_Receive32(client.relSock);
					uint16_t dataSize = Socket_Receive16(client.relSock);

					if (entityID >= entities.size) {
						if (entityID >= AE_MAX_ENTITIES) {
							KickClient("Server used entity ID above limit");
							break;
						}

						entities.pool = SafeRealloc(
							entities.pool, (entityID + 1) * sizeof(Entity)
						);
					}

					if (dataSize > AE_MAX_ENTITY_SIZE) {
						KickClient("Server sent entity size above limit");
						break;
					}

					entities.pool[entityID].state |= AE_ENT_USED;

					client.entityID = entityID;
					client.dataSize = dataSize;
					client.dataRead = 0;
					client.data     = SafeMalloc(dataSize);
					client.state    = C_NEW_ENTITY;
					break;
				}
				case 0x06: { // update entity component
					if (available < 4 + 4 + 2) break;

					uint32_t entityID = Socket_Receive32(client.relSock);
					uint32_t compID   = Socket_Receive32(client.relSock);
					uint16_t dataSize = Socket_Receive16(client.relSock);

					if (entityID >= entities.size) {
						KickClient("Server sent invalid entity ID");
						break;
					}
					if (!Entities_CompIDValid(compID)) {
						KickClient("Server sent invalid entity component ID");
						break;
					}
					if (dataSize > AE_MAX_COMP_SIZE) {
						KickClient("Server sent entity component size above limit");
						break;
					}

					client.state    = C_UPDATE_COMP;
					client.entityID = entityID;
					client.dataSize = dataSize;
					client.dataRead = 0;
					client.data     = SafeMalloc(dataSize);
					break;
				}
				case 0x08: { // update entity position
					if (available < 4 + 1 + (4 * 6)) break;

					uint32_t entityID  = Socket_Receive32(client.relSock);
					uint8_t  flags     = Socket_Receive8(client.relSock);
					float    x         = Socket_ReceiveFloat(client.relSock);
					float    y         = Socket_ReceiveFloat(client.relSock);
					float    z         = Socket_ReceiveFloat(client.relSock);
					float    yaw       = Socket_ReceiveFloat(client.relSock);
					float    pitch     = Socket_ReceiveFloat(client.relSock);
					uint32_t sectorIdx = Socket_Receive32(client.relSock);

					enum {
						FLAG_X      = 1,
						FLAG_Y      = 2,
						FLAG_Z      = 4,
						FLAG_YAW    = 8,
						FLAG_PITCH  = 16,
						FLAG_SECTOR = 32
					};

					if (entityID >= entities.size) {
						KickClient("Server sent invalid entity ID");
						break;
					}
					if ((flags & FLAG_SECTOR) && (map.active || (sectorIdx >= map.sectorsLen))) {
						KickClient("Server sent invalid sector ID");
						break;
					}

					Entity* ent = &entities.pool[entityID];

					if (flags & FLAG_X) ent->pos.x = x;
					if (flags & FLAG_Y) ent->pos.y = y;
					if (flags & FLAG_Z) ent->pos.z = z;

					if (flags & FLAG_YAW)   ent->dir.yaw   = yaw;
					if (flags & FLAG_PITCH) ent->dir.pitch = pitch;

					if (flags & FLAG_SECTOR) {
						Map_DetachEntity(entityID);
						ent->sector = &map.sectors[sectorIdx];
						Map_AddEntity(entityID);
					}
					break;
				}
				default: {
					Log("client: Server sent invalid packet ID: %.4x", client.packetID);
				}
			}

			break;
		}
		case C_FILE:
		case C_NEW_ENTITY:
		case C_UPDATE_COMP: {
			size_t remaining = ((size_t) client.dataSize) - client.dataRead;
			size_t chunk     = (remaining > 1024)? 1024 : remaining;

			// if (Socket_DataAvailable(client.relSock) == 0) break;
			if (Socket_DataAvailable(client.relSock) < chunk) break;

			Socket_Receive(client.relSock, &client.data[client.dataRead], chunk);
			client.dataRead += chunk;

			if (client.dataRead != client.dataSize) break;

			switch (client.state) {
				case C_FILE: {
					char* path = ConcatString("net:", client.fileName);

					if (!VFS_WriteFile(path, client.data, client.dataSize)) {
						Error("Failed to write net file");
						break;
					}
					Log("Downloaded %s", client.fileName);
					free(path);

					if (strcmp(client.fileName, "map.arm") == 0) {
						bool   success;
						Stream file = VFS_Open("net:map.arm", &success, false);

						if (!success) {
							Error("Failed to open server map");
							break;
						}

						Map_LoadFile(&file, "net:map.arm", true);
						Stream_Close(&file);

						// send map loaded event
						Event e;
						e.type = AE_EVENT_CLIENT_MAP_LOADED;
						Event_Add(&e);
					}

					client.state = C_WAITING;
					break;
				}
				case C_NEW_ENTITY: {
					Entity* ent = &entities.pool[client.entityID];

					if (!Entities_Deserialise(client.data, client.dataSize, ent)) {
						KickClient("Failed to deserialise entity");
						break;
					}

					ent->state |= AE_ENT_ACTIVE;

					client.state = C_WAITING;
					break;
				}
				case C_UPDATE_COMP: {
					Entity*          ent  = &entities.pool[client.entityID];
					EntityComponent* comp = Entities_GetComponent(client.compID);

					assert(comp->read);

					if (comp->read(ent, client.data, client.dataSize) == ((size_t) -1)) {
						KickClient("Failed to deserialise entity component");
						break;
					}

					client.state = C_WAITING;
					break;
				}
			}
			break;
		}
		case C_STOP: break; // nothing to do
	}

	// send position to server
	if (map.active && client.reportPos && client.sessionID && (client.state == C_WAITING)) {
		uint8_t packet[30];
		packet[0] = 0x00;
		packet[1] = 0xFF;

		++ client.movement;

		Entity* ent = Player_GetEntity();

		Data_Write32(&packet[2], client.movement);
		Data_Write32(&packet[6], (uint32_t) (ent->sector - map.sectors));
		Data_WriteFloat(&packet[10], ent->pos.x);
		Data_WriteFloat(&packet[14], ent->pos.y);
		Data_WriteFloat(&packet[18], ent->pos.z);
		Data_WriteFloat(&packet[22], ent->dir.yaw);
		Data_WriteFloat(&packet[26], ent->dir.pitch);

		if (client.udpSock) {
			Socket_Send(client.udpSock, packet, sizeof(packet));
			Socket_Flush(client.udpSock);
		}
		else {
			Socket_Send(client.relSock, packet, sizeof(packet));
		}
	}
}

void Client_SendMessage(const char* message) {
	uint8_t data[130];
	data[0] = 0x02;
	data[1] = 0;

	strncpy((char*) &data[2], message, 128);
	Socket_Send(client.relSock, data, sizeof(data));
}
