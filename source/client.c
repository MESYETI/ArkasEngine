#include "mem.h"
#include "map.h"
#include "chat.h"
#include "data.h"
#include "util.h"
#include "event.h"
#include "client.h"
#include "server.h"
#include "player.h"
#include "ramDrive.h"
#include "platform.h"
#include "resources.h"

enum {
	C_IDENT = 0, // about to identify
	C_WAITING,   // waiting for a packet
	C_PACKET,    // waiting for packet to finish sending
	C_FILE       // receiving a file
};

Client client = {
	.running     = false,
	.state       = C_WAITING,
	.relSock     = NULL,
	.udpSock     = NULL,
	.downloading = false
};

static void StartClient(void) {
	if (Resources_DriveExists("net")) {
		Resources_DeleteDrive("net");
	}
	Resources_AddDrive(NewRamDrive(), "net");

	Chat_Free();
	Chat_Init();
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
			uint16_t id = 0;
			Socket_Send(client.relSock, &id, sizeof(id));

			uint16_t ver = 0;
			Socket_Send(client.relSock, &ver, sizeof(ver));

			uint16_t port = 0;

			if (client.udpSock) {
				NetSocketAddr addr;

				if (!Socket_GetAddr(client.relSock, &addr)) {
					Error("client: Failed to get address of UDP socket");
				}

				port = NetSocketAddr_Port(&addr);
			}

			Socket_Send(client.relSock, &port, sizeof(port));

			Socket_Send(client.relSock, client.name, sizeof(client.name));

			client.state = C_WAITING;
			Log("client: Sent identification");
			break;
		}
		case C_WAITING: {
			if (Socket_DataAvailable(client.relSock) < 2) break;

			Socket_Receive(client.relSock, &client.packetID, 2);
			client.state = C_PACKET;
			break;
		}
		case C_PACKET: {
			size_t available = Socket_DataAvailable(client.relSock);

			switch (client.packetID) {
				case 0x00: { // identification
					size_t size = 34;

					if (available < size) break;

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
					Socket_Receive(client.relSock, &client.fileSize, 4);

					if (client.fileSize > 16777216) {
						client.running = false;
						Error("Server attempted to send %d byte file", client.fileSize);
					}

					client.fileContents = SafeMalloc(client.fileSize);
					client.fileRead     = 0;
					client.state        = C_FILE;

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

					Log("Kicked: %s", message);

					Event e;
					e.type = AE_EVENT_CLIENT_KICKED;
					strcpy(e.clientKick.message, message);
					Event_Add(&e);
					break;
				}
				default: {
					Log("client: Server sent invalid packet ID: %.4x", client.packetID);
				}
			}

			break;
		}
		case C_FILE: {
			size_t remaining = ((size_t) client.fileSize) - client.fileRead;
			size_t chunk     = (remaining > 1024)? 1024 : remaining;

			if (Socket_DataAvailable(client.relSock) == 0) break;

			Socket_Receive(
				client.relSock, &client.fileContents[client.fileRead], chunk
			);
			client.fileRead += chunk;

			if (client.fileRead == client.fileSize) {
				char* path = ConcatString("net:", client.fileName);

				if (!Resources_WriteFile(path, client.fileContents, client.fileSize)) {
					Error("Failed to write net file");
					break;
				}
				Log("Downloaded %s", client.fileName);
				free(path);

				if (strcmp(client.fileName, "map.arm") == 0) {
					bool   success;
					Stream file = Resources_Open("net:map.arm", &success, false);

					if (!success) {
						Error("Failed to open server map");
						break;
					}

					Map_LoadFile(&file, "net:map.arm");
					Stream_Close(&file);
					client.state = C_WAITING;

					// send map loaded event
					Event e;
					e.type = AE_EVENT_CLIENT_MAP_LOADED;
					Event_Add(&e);
				}
			}
			break;
		}
	}

	// send position to server
	if (map.active) {
		uint8_t packet[30];
		packet[0] = 0x00;
		packet[1] = 0xFF;

		++ client.movement;

		Data_Write32(&packet[2], client.movement);
		Data_Write32(&packet[6], (uint32_t) (player.sector - map.sectors));
		Data_WriteFloat(&packet[10], player.pos.x);
		Data_WriteFloat(&packet[14], player.pos.y);
		Data_WriteFloat(&packet[18], player.pos.z);
		Data_WriteFloat(&packet[22], player.yaw);
		Data_WriteFloat(&packet[26], player.pitch);

		if (client.udpSock) {
			Socket_Send(client.udpSock, packet, sizeof(packet));
			Socket_Flush(client.udpSock);

			puts("Sent position over UDP");
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
