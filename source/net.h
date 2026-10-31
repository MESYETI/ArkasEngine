#ifndef AE_NET_H
#define AE_NET_H

#define SOCKET_UDP_DATA_SIZE 508

// defines NetSocket and NetSocketAddr
#ifdef AE_NET_UNIX
	#include "net/unixSock.h"
#elif defined(AE_NET_STUB)
	#include "net/stub.h"
#else
	#error "no net socket implementation selected"
#endif

void     NetSocket_Init(void);
void     NetSocket_End(void);
bool     NetSocket_New(NetSocket* sock, int protocol);
bool     NetSocket_Bind(NetSocket* sock, uint16_t port);
bool     NetSocket_Listen(NetSocket* sock, int backlog);
Socket*  NetSocket_Accept(NetSocket* sock);
bool     NetSocket_Connect(NetSocket* sock, const char* ip, uint16_t port);
size_t   NetSocket_DataAvailable(NetSocket* sock);
bool     NetSocket_IsDataAvailable(NetSocket* sock);
size_t   NetSocket_Receive(NetSocket* sock, void* buf, size_t size);
size_t   NetSocket_ReceiveUDP(NetSocket* sock, void* buf, size_t size, NetSocketAddr* addr);
size_t   NetSocket_Send(NetSocket* sock, void* buf, size_t size);
void     NetSocket_Flush(NetSocket* sock);
void     NetSocket_Close(NetSocket* sock);
void     NetSocket_StringAddr(NetSocket* sock, char* dest, size_t size);
bool     NetSocket_Connected(NetSocket* sock);
bool     NetSocket_GetAddr(NetSocket* sock, NetSocketAddr* out);
void     NetSocket_SetSessionID(NetSocket* sock, uint32_t sessionID);
uint16_t NetSocketAddr_Port(NetSocketAddr* addr);
bool     NetSocketAddr_Compare(NetSocketAddr* a, NetSocketAddr* b);
bool     NetSocketAddr_CompareAPort(NetSocketAddr* a, NetSocketAddr* b, uint16_t port);
void     NetSocketAddr_StringAddr(NetSocketAddr* addr, char* dest, size_t size);

#endif
