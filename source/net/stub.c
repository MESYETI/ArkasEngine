#ifdef AE_NET_STUB

#include "../socket.h"

void NetSocket_Init(void) {
	abort();
}

void NetSocket_End(void) {
	abort();
}

void NetSocket_New(NetSocket* sock, int protocol) {
	abort();
}

bool NetSocket_Bind(NetSocket* sock, uint16_t port) {
	abort();
}

bool NetSocket_Listen(NetSocket* sock, int backlog) {
	abort();
}

Socket NetSocket_Accept(NetSocket* sock) {
	abort();
}

bool NetSocket_Connect(NetSocket* sock, const char* ip, uint16_t port) {
	abort();
}

size_t NetSocket_DataAvailable(NetSocket* sock) {
	abort();
}

bool NetSocket_IsDataAvailable(NetSocket* sock) {
	abort();
}

size_t NetSocket_Receive(NetSocket* sock, void* buf, size_t size) {
	abort();
}

size_t NetSocket_ReceiveUDP(NetSocket* sock, void* buf, size_t size, NetSocketAddr* addr) {
	abort();
}

size_t NetSocket_Send(NetSocket* sock, void* buf, size_t size) {
	abort();
}

void NetSocket_Close(NetSocket* sock) {
	abort();
}

void NetSocket_StringAddr(NetSocket* sock, char* dest, size_t size) {
	abort();
}

bool NetSocket_GetAddr(NetSocket* sock, NetSocketAddr* out) {
	abort();
}

void NetSocket_SetSessionID(NetSocket* sock, uint32_t sessionID) {
	abort();
}

uint16_t NetSocketAddr_Port(NetSocketAddr* addr) {
	abort();
}

bool NetSocketAddr_Compare(NetSocketAddr* a, NetSocketAddr* b) {
	abort();
}

bool NetSocketAddr_CompareAPort(NetSocketAddr* a, NetSocketAddr* b, uint16_t port) {
	abort();
}

void NetSocketAddr_StringAddr(NetSocketAddr* addr, char* dest, size_t size) {
	abort();
}

#endif
