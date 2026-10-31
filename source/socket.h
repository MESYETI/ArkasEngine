#ifndef AE_SOCKET_H
#define AE_SOCKET_H

#include "common.h"

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

#include "net.h"

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

Socket* Socket_AllocSocket(void);

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
void     Socket_SetSessionID(Socket* sock, uint32_t sessionID);

#endif
