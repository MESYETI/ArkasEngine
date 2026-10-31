#include <sys/socket.h>

typedef struct {
	struct sockaddr_storage addr;
	socklen_t               addrLen;
} NetSocketAddr;

typedef struct {
	int type;

	int      fd;
	int      protocol;
	uint16_t ident;

	NetSocketAddr addr;

	// only for UDP sockets
	uint8_t  sendData[SOCKET_UDP_DATA_SIZE];
	size_t   sendLen;
	uint32_t sessionID;
} NetSocket;
