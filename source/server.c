#include "util.h"
#include "server.h"

Server server = {
	.running   = false,
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

void Server_Update(void) {
	Socket* newClient = NULL;

	if (server.netSock) {
		newClient = Socket_Accept(server.netSock);
	}
	if (!newClient && server.localSock) {
		newClient = Socket_Accept(server.localSock);
	}

	if (newClient) {
		Log("Client connected");
		Socket_Close(newClient);
	}
}
