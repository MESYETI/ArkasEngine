#ifndef AE_SOCKET_H
#define AE_SOCKET_H

#include <sys/socket.h>
#include "common.h"

#define SOCKET_UDP_DATA_SIZE 508

enum {
	SOCKET_TYPE_LOCAL = 0,
	SOCKET_TYPE_NET
};

enum {
	SOCKET_PROTOCOL_TCP = 0,
	SOCKET_PROTOCOL_UDP
};

typedef struct LocalSocket LocalSocket;
typedef struct Socket      Socket;

struct LocalSocket {
	int type;

	LocalSocket*  other;
	uint8_t*      buf;
	size_t        cap;
	size_t        len;

	bool     server;
	Socket** connections;
	size_t   connectionsLen;
};

typedef struct {
	#ifdef AE_NET_SOCKET
		struct sockaddr_storage addr;
		socklen_t               addrLen;
	#else
		int nothing;
	#endif
} NetSocketAddr;

typedef struct {
	int type;

	#ifdef AE_NET_SOCKET
		int      fd;
		int      protocol;
		uint16_t ident;

		NetSocketAddr addr;

		// only for UDP sockets
		uint8_t sendData[SOCKET_UDP_DATA_SIZE - 2];
		size_t  sendLen;
	#endif
} NetSocket;

typedef union {
	int type;

	LocalSocket local;
	NetSocket   net;
} SocketValue;

struct Socket {
	SocketValue value;
	Socket*     next;
	Socket*     prev;
};

Socket*  Socket_New(int type, int protocol);
bool     Socket_Bind(Socket* sock, uint16_t port);
bool     Socket_Listen(Socket* sock, int backlog);
Socket*  Socket_Accept(Socket* sock);
bool     Socket_ConnectNet(Socket* sock, const char* ip, uint16_t port);
bool     Socket_ConnectLocal(Socket* sock, Socket* to);
size_t   Socket_DataAvailable(Socket* sock);
bool     Socket_IsDataAvailable(Socket* sock);
size_t   Socket_Receive(Socket* sock, void* buf, size_t size);
size_t   Socket_ReceiveUDP(Socket* sock, void* buf, size_t size, NetSocketAddr* addr);
size_t   Socket_Send(Socket* sock, void* buf, size_t size);
void     Socket_Flush(Socket* sock);
void     Socket_Close(Socket* sock);
void     Socket_StringAddr(Socket* sock, char* dest, size_t size);
bool     Socket_Connected(Socket* sock);
bool     Socket_GetAddr(Socket* sock, NetSocketAddr* out);

uint16_t NetSocketAddr_Port(NetSocketAddr* addr);
bool     NetSocketAddr_Compare(NetSocketAddr* a, NetSocketAddr* b);
bool     NetSocketAddr_CompareAPort(NetSocketAddr* a, NetSocketAddr* b, uint16_t port);
void     NetSocketAddr_StringAddr(NetSocketAddr* addr, char* dest, size_t size);

#endif
