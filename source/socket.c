#include <string.h>
#include "mem.h"
#include "data.h"
#include "util.h"
#include "socket.h"

Socket* sockets = NULL;

Socket* Socket_AllocSocket(void) {
	Socket* socket = SafeMalloc(sizeof(Socket));
	socket->prev   = NULL;
	socket->next   = sockets;

	if (sockets) {
		sockets->prev = socket;
	}

	sockets = socket;

	return sockets;
}

void Socket_Init(void) {
	#ifdef AE_NET_SOCKET
		NetSocket_Init();
	#endif
}

void Socket_End(void) {
	#ifdef AE_NET_SOCKET
		NetSocket_End();
	#endif
}

Socket* Socket_New(int type, int protocol) {
	SocketValue ret;
	ret.type = type;

	switch (type) {
		case SOCKET_TYPE_LOCAL: {
			ret.local.other          = NULL;
			ret.local.buf            = NULL;
			ret.local.cap            = 0;
			ret.local.len            = 0;
			ret.local.server         = false;
			ret.local.connections    = NULL;
			ret.local.connectionsLen = 0;
			break;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				NetSocket_New(&ret.net, protocol);
			#else
				Error("Network sockets not available");
			#endif

			break;
		}
	}

	Socket* sock = Socket_AllocSocket();
	sock->value  = ret;
	return sock;
}

bool Socket_Bind(Socket* sock, uint16_t port) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			sock->value.local.server         = true;
			sock->value.local.connections    = NULL;
			sock->value.local.connectionsLen = 0;
			return true;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				return NetSocket_Bind(&sock->value.net, port);
			#else
				Error("Network sockets not available");
			#endif
		}
	}

	assert(0);
}

bool Socket_Listen(Socket* sock, int backlog) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: return true; // no need to do anything
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				return NetSocket_Listen(&sock->value.net, backlog);
			#else
				Error("Network sockets not available");
			#endif
		}
	}

	assert(0);
}

Socket* Socket_Accept(Socket* sock) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			if (!sock->value.local.server) {
				Error("Tried to accept a connection on a non-server socket");
			}
			if (sock->value.local.connectionsLen == 0) {
				return NULL;
			}

			Socket* other            = sock->value.local.connections[0];
			Socket* ret              = Socket_New(SOCKET_TYPE_LOCAL, 0);
			ret->value.local.other   = &other->value.local;
			other->value.local.other = &ret->value.local;

			-- sock->value.local.connectionsLen;
			memmove(
				&sock->value.local.connections[0],
				&sock->value.local.connections[1],
				sock->value.local.connectionsLen * sizeof(Socket*)
			);

			sock->value.local.connections = SafeRealloc(
				sock->value.local.connections,
				sock->value.local.connectionsLen * sizeof(Socket*)
			);
			return ret;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				return NetSocket_Accept(&sock->value.net);
			#else
				Error("Network sockets not available");
			#endif
		}
	}

	assert(0);
}

bool Socket_ConnectNet(Socket* sock, const char* ip, uint16_t port) {
	#ifdef AE_NET_SOCKET
		if (sock->value.type == SOCKET_TYPE_LOCAL) {
			Error("Called Socket_ConnectNet on local socket");
		}

		return NetSocket_Connect(&sock->value.net, ip, port);
	#else
		Error("Network sockets not available");
	#endif
}

bool Socket_ConnectLocal(Socket* sock, Socket* to) {
	if (sock->value.type != SOCKET_TYPE_LOCAL) {
		Error("Called Socket_ConnectLocal on non-local socket");
	}

	if (!to->value.local.server) {
		Error("Attempted to connect to non-server socket");
	}

	++ to->value.local.connectionsLen;
	to->value.local.connections = SafeRealloc(
		to->value.local.connections, to->value.local.connectionsLen * sizeof(Socket)
	);

	to->value.local.connections[to->value.local.connectionsLen - 1] = sock;
	return true;
}

size_t Socket_DataAvailable(Socket* sock) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			return sock->value.local.len;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				return NetSocket_DataAvailable(&sock->value.net);
			#else
				Error("Network sockets not available");
			#endif
		}
	}

	assert(0);
}

bool Socket_IsDataAvailable(Socket* sock) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			return sock->value.local.len > 0;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				return NetSocket_IsDataAvailable(&sock->value.net);
			#else
				Error("Network sockets not available");
			#endif
		}
	}

	assert(0);
}

// bool Socket_ReceiveUDP(Socket* sock) {
// 	if (
// 		(sock->value.type != SOCKET_TYPE_NET) ||
// 		(sock->value.net.protocol != SOCKET_PROTOCOL_UDP)
// 	) {
// 		Error("Called ReceiveUDP on non-network or non-UDP socket");
// 	}
// 
// 	size_t          remaining = 506 - sock->value.net.recvLen;
// 	struct sockaddr addr;
// 	socklen_t       addrLen;
// 	uint8_t*        buf = &sock->value.net.recvData[sock->value.net.recvLen];
// 
// 	ssize_t len = recvfrom(sock->value.net.fd, buf, remaining, 0, &addr, &addrLen);
// 
// 	if (len <= 0) {
// 		return false;
// 	}
// 
// 	sock->value.net.recvLen += (size_t) len;
// 
// 	bool ret = sock->value.net.recvLen >= 506;
// 
// 	if (ret) {
// 		sock->value.net.recvLen = 0;
// 	}
// 
// 	return ret;
// }

size_t Socket_Receive(Socket* sock, void* buf, size_t size) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			size_t ret = size;

			if (size > sock->value.local.len) {
				ret = sock->value.local.len;
			}
			if (ret == 0) return 0;

			size_t remaining = sock->value.local.len - ret;
			memcpy(buf, sock->value.local.buf, ret);
			memmove(sock->value.local.buf, &sock->value.local.buf[ret], remaining);

			sock->value.local.len -= ret;

			size_t newSize = sock->value.local.cap;
			while (remaining < newSize / 2) {
				newSize /= 2;
			}
			
			if (newSize != sock->value.local.cap) {
				sock->value.local.buf = SafeRealloc(
					sock->value.local.buf, newSize
				);
				sock->value.local.cap = newSize;
			}
			return ret;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				return NetSocket_Receive(&sock->value.net, buf, size);
			#else
				Error("Network sockets not available");
			#endif
		}
	}

	assert(0);
}

size_t Socket_ReceiveUDP(Socket* sock, void* buf, size_t size, NetSocketAddr* addr) {
	if (sock->value.type != SOCKET_TYPE_NET) {
		Error("Called ReceiveUDP on non-network socket");
	}

	if (sock->value.net.protocol != SOCKET_PROTOCOL_UDP) {
		Error("Called ReceiveUDP on non-UDP socket");
	}

	#ifdef AE_NET_SOCKET
		return NetSocket_ReceiveUDP(&sock->value.net, buf, size, addr);
	#else
		Error("Network sockets not available");
		return (size_t) -1;
	#endif
}

size_t Socket_Send(Socket* sock, void* buf, size_t size) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			LocalSocket* other  = sock->value.local.other;
			size_t       newLen = other->len + size;
			size_t       newCap = other->cap;

			if (newCap == 0) {
				++ newCap;
			}

			while (newLen > newCap) {
				newCap *= 2;
			}

			if (newCap != other->cap) {
				other->buf = SafeRealloc(other->buf, newCap);
				other->cap = newCap;
			}

			memcpy(&other->buf[other->len], buf, size);
			other->len += size;
			return size;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				return NetSocket_Send(&sock->value.net, buf, size);
			#else
				break;
			#endif
		}
	}

	assert(0);
}

void Socket_Flush(Socket* sock) {
	if (sock->value.type         != SOCKET_TYPE_NET)     return;
	if (sock->value.net.protocol != SOCKET_PROTOCOL_UDP) return;

	NetSocket_Flush(&sock->value.net);
}

void Socket_Close(Socket* sock) {
	if (sockets == sock) {
		sockets = sock->next;
	}

	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			if (sock->value.local.other) {
				sock->value.local.other->other = NULL;
			}
			if (sock->value.local.buf) {
				free(sock->value.local.buf);
			}
			break;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				NetSocket_Close(&sock->value.net);
				break;
			#else
				assert(0);
			#endif
		}
	}

	if (sock->prev) {
		sock->prev->next = sock->next;
	}
	if (sock->next) {
		sock->next->prev = sock->prev; // use after free
	}
	free(sock); // freed here
}

void Socket_StringAddr(Socket* sock, char* dest, size_t size) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			snprintf(dest, size, "%p", (void*) sock);
			break;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				NetSocket_StringAddr(&sock->value.net, dest, size);
				break;
			#else
				assert(0);
			#endif
		}
	}
}

bool Socket_Connected(Socket* sock) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: return sock->value.local.other? true : false;
		case SOCKET_TYPE_NET:   return true;
	}

	assert(0);
}

bool Socket_GetAddr(Socket* sock, NetSocketAddr* out) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			Error("GetAddr not available on local sockets");
			break;
		}
		case SOCKET_TYPE_NET: {
			#ifdef AE_NET_SOCKET
				return NetSocket_GetAddr(&sock->value.net, out);
			#else
				Error("Network sockets not available");
			#endif
		}
	}

	return false;
}

void Socket_SetSessionID(Socket* sock, uint32_t sessionID) {
	if (sock->value.type != SOCKET_TYPE_NET) {
		Log("SetSessionID can only be called on network UDP sockets");
	}
	if (sock->value.net.protocol != SOCKET_PROTOCOL_UDP) {
		Log("SetSessionID can only be called on network UDP sockets");
	}

	NetSocket_SetSessionID(&sock->value.net, sessionID);
}
