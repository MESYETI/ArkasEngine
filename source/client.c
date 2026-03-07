#include "util.h"
#include "client.h"
#include "server.h"

Client client = {
	.relSocket = NULL
};

bool Client_StartLocal(void) {
	if (!server.running || !server.localSock) {
		Log("client: Server not running or does not have local socket");
		return false;
	}

	client.relSocket = Socket_New(SOCKET_TYPE_LOCAL, 0);

	if (!client.relSocket) {
		Log("client: Failed to open socket");
		return false;
	}

	if (!Socket_ConnectLocal(client.relSocket, server.localSock)) {
		Log("client: Failed to connect");
		Socket_Close(client.relSocket);
		client.relSocket = NULL;
		return false;
	}

	Log("client: Client connected to local server");
	return true;
}

bool Client_StartINet(const char* ip, uint16_t port) {
	client.relSocket = Socket_New(SOCKET_TYPE_NET, SOCKET_PROTOCOL_TCP);

	if (!client.relSocket) {
		Log("client: Failed to open socket");
		return false;
	}

	if (!Socket_ConnectNet(client.relSocket, ip, port)) {
		Log("client: Failed to connect");
		Socket_Close(client.relSocket);
		client.relSocket = NULL;
		return false;
	}

	Log("client: Client connected to internet server");
	return true;
}
