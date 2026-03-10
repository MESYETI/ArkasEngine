#include "mem.h"
#include "map.h"
#include "util.h"
#include "client.h"
#include "server.h"
#include "ramDrive.h"
#include "resources.h"

enum {
	C_IDENT = 0, // about to identify
	C_WAITING,   // waiting for a packet
	C_PACKET,    // waiting for packet to finish sending
	C_FILE       // receiving a file
};

Client client = {
	.running     = false,
	.relSock     = NULL,
	.state       = C_WAITING,
	.downloading = false
};

static void StartClient(void) {
	if (Resources_DriveExists("net")) {
		Resources_DeleteDrive("net");
	}
	Resources_AddDrive(NewRamDrive(), "net");
}

bool Client_StartLocal(void) {
	if (!server.running || !server.localSock) {
		Log("client: Server not running or does not have local socket");
		return false;
	}

	client.relSock = Socket_New(SOCKET_TYPE_LOCAL, 0);
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

	Log("client: Client connected to internet server");
	client.running = true;
	StartClient();
	return true;
}

void Client_Update(void) {
	if (!Socket_Connected(client.relSock)) {
		Log("client: Client sock not connected yet, waiting");
		return;
	}

	switch (client.state) {
		case C_IDENT: {
			uint16_t id = 0;
			Socket_Send(client.relSock, &id, sizeof(id));

			uint16_t ver = 0;
			Socket_Send(client.relSock, &ver, sizeof(ver));

			char name[32] = "player";
			Socket_Send(client.relSock, name, sizeof(name));

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
				case 0x00: {
					size_t size = 32;

					if (size != available) break;

					char name[33];
					name[32] = 0;
					Socket_Receive(client.relSock, &name, 32);
					Log("client: Server identifies as '%s'", name);

					client.state = C_WAITING;
					break;
				}
				case 0x01: {
					size_t size = 64 + 4;

					if (size != available) break;

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
					Stream file = Resources_Open("net:map.arm", &success);

					if (!success) {
						Error("Failed to open server map");
						break;
					}

					Map_LoadFile(&file, "net:map.arm");
					client.state = C_WAITING;
				}
			}
			break;
		}
	}
}
