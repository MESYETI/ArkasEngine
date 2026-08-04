#include <string.h>
#include "mem.h"
#include "util.h"
#include "socket.h"

#ifdef AE_NET_SOCKET
	#include <time.h>
	#include <poll.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
#endif

Socket* sockets = NULL;

static Socket* AllocSocket(void) {
	Socket* socket = SafeMalloc(sizeof(Socket));
	socket->prev   = NULL;
	socket->next   = sockets;

	if (sockets) {
		sockets->prev = socket;
	}

	sockets = socket;

	return sockets;
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
				int type = 0;

				if (protocol == SOCKET_PROTOCOL_TCP)      type = SOCK_STREAM;
				else if (protocol == SOCKET_PROTOCOL_UDP) type = SOCK_DGRAM;

				ret.net.fd       = socket(AF_INET, type, 0);
				ret.net.protocol = protocol;

				if (ret.net.fd == -1) {
					Log("Failed to create socket: %s", strerror(errno));
					return NULL;
				}

				int reuse = 1;
				int res = setsockopt(
					ret.net.fd, SOL_SOCKET, SO_REUSEADDR, (void*) &reuse, sizeof(reuse)
				);

				if (res < 0) {
					Log("warning: Failed to set reuse address option");
				}
			#else
				Error("Network sockets not available");
			#endif

			break;
		}
	}

	Socket* sock = AllocSocket();
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
				struct sockaddr_in addr;

				addr.sin_family      = AF_INET;
				addr.sin_addr.s_addr = INADDR_ANY;
				addr.sin_port        = htons(port);

				if (bind(sock->value.net.fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
					Log("bind failed: %s", strerror(errno));
					return false;
				}
				
				return true;
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
				if (sock->value.net.protocol == SOCKET_PROTOCOL_UDP) {
					Error("Called listen on UDP socket");
				}

				if (listen(sock->value.net.fd, backlog) < 0) {
					Log("listen failed: %s", strerror(errno));
					return false;
				}
				fcntl(sock->value.net.fd, F_SETFL, O_NONBLOCK);
				return true;
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
				if (sock->value.net.protocol == SOCKET_PROTOCOL_UDP) {
					Error("Called accept on UDP socket");
				}

				struct sockaddr_storage addr;

				socklen_t len = sizeof(addr);

				int fd = accept(sock->value.net.fd, (struct sockaddr*) &addr, &len);

				if (fd < 0) {
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
						return NULL;
					}

					Error("accept error: %s", strerror(errno));
				}

				Socket* ret                  = AllocSocket();
				ret->value.type              = sock->value.type;
				ret->value.net.fd            = fd;
				ret->value.net.protocol      = sock->value.net.protocol;
				ret->value.net.addr.addr     = addr;
				ret->value.net.addr.addrLen  = (size_t) len;

				fcntl(fd, F_SETFL, O_NONBLOCK);
				return ret;
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

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(port);

		if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
			if (inet_pton(AF_INET6, ip, &addr.sin_addr) <= 0) {
				Log("Invalid address: %s", ip);
				return false;
			}
			else {
				addr.sin_family = AF_INET6;
			}
		}

		if (connect(sock->value.net.fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
			Log("Connection failed: %s", strerror(errno));
			return false;
		}

		fcntl(sock->value.net.fd, F_SETFL, O_NONBLOCK);

		return true;
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
				if (sock->value.net.protocol == SOCKET_PROTOCOL_UDP) {
					Error("Called DataAvailable on UDP socket");
				}

				int count;
				ioctl(sock->value.net.fd, FIONREAD, &count);

				return (size_t) count;
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
				struct pollfd pollfd;

				pollfd.fd     = sock->value.net.fd;
				pollfd.events = POLLIN;

				struct timespec time;
				time.tv_sec  = 0;
				time.tv_nsec = 5000;

				int res = ppoll(&pollfd, 1, &time, NULL);

				if (res < 0) {
					Error("ppoll failed: %s", strerror(errno));
				}

				return (pollfd.revents & POLLIN)? true : false;
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
				if (sock->value.net.protocol == SOCKET_PROTOCOL_UDP) {
					Error("Called receive on UDP socket");
				}

				ssize_t ret = recv(sock->value.net.fd, buf, size, 0);
				if (ret <= 0) return 0;
				return (size_t) ret;
			#else
				Error("Network sockets not available");
			#endif
		}
	}

	assert(0);
}

size_t Socket_ReceiveUDP(Socket* sock, void* buf, size_t size, NetSocketAddr* addr) {
	#ifdef AE_NET_SOCKET
		if (sock->value.type != SOCKET_TYPE_NET) {
			Error("Called ReceiveUDP on non-network socket");
		}

		if (sock->value.net.protocol != SOCKET_PROTOCOL_UDP) {
			Error("Called ReceiveUDP on non-UDP socket");
		}

		uint8_t dg[2 + SOCKET_UDP_DATA_SIZE];

		ssize_t dgSize = recvfrom(
			sock->value.net.fd, dg, sizeof(dg), 0, (struct sockaddr*) &addr->addr, &addr->addrLen
		);
		if (dgSize < 4) return 0;

		uint16_t dataSize = *((uint16_t*) &dg[0]);

		memcpy(buf, &dg[2], size < dataSize? size : dataSize);

		return (size_t) dataSize;
	#else
		Error("Network sockets not available");
		return (size_t) -1;
	#endif
}

static ssize_t SendUDP(Socket* sock) {
	uint16_t len = (uint16_t) sock->value.net.sendLen;

	uint8_t data[2 + SOCKET_UDP_DATA_SIZE];
	data[0] = (uint8_t) (len & 0xFF);
	data[1] = (uint8_t) ((len & 0xFF00) >> 8);

	memcpy(&data[2], sock->value.net.sendData, SOCKET_UDP_DATA_SIZE);

	ssize_t ret = sendto(
		sock->value.net.fd, data, sizeof(data), 0, (struct sockaddr*) &sock->value.net.addr.addr,
		sock->value.net.addr.addrLen
	);

	sock->value.net.sendLen = 0;
	return ret;
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
				ssize_t ret;

				if (sock->value.net.protocol == SOCKET_PROTOCOL_UDP) {
					if (sock->value.net.sendLen + size > 504) {
						SendUDP(sock);
					}

					memcpy(&sock->value.net.sendData[sock->value.net.sendLen], buf, size);
					sock->value.net.sendLen += size;

					if (sock->value.net.sendLen == 504) {
						SendUDP(sock);
					}
				}
				else {
					ret = send(sock->value.net.fd, buf, size, 0);
				}

				if (ret <= 0) return 0;
				return (size_t) ret;
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

	SendUDP(sock);
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
				close(sock->value.net.fd);
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
				inet_ntop(
					AF_INET, &(((struct sockaddr_in *) &sock->value.net.addr)->sin_addr),
					dest, size
				);
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
				out->addrLen = sizeof(out->addr);
				return getsockname(
					sock->value.net.fd, (struct sockaddr*) &out->addr, &out->addrLen
				) == 0;
			#else
				Error("Network sockets not available");
			#endif
		}
	}

	return false;
}

uint16_t NetSocketAddr_Port(NetSocketAddr* addr) {
	#ifdef AE_NET_SOCKET
		if (addr->addr.ss_family == AF_INET) {
			struct sockaddr_in* addr2 = (struct sockaddr_in*) &addr->addr;

			return ntohs(addr2->sin_port);
		}
		else if (addr->addr.ss_family == AF_INET6) {
			struct sockaddr_in6* addr2 = (struct sockaddr_in6*) &addr->addr;

			return ntohs(addr2->sin6_port);
		}
	#else
		Error("Network sockets not available");
	#endif

	return 0;
}

bool NetSocketAddr_Compare(NetSocketAddr* a, NetSocketAddr* b) {
	#ifdef AE_NET_SOCKET
		if (a->addr.ss_family != b->addr.ss_family) {
			return false;
		}

		if (a->addr.ss_family == AF_INET) {
			struct sockaddr_in* aIn = (struct sockaddr_in*) &a->addr;
			struct sockaddr_in* bIn = (struct sockaddr_in*) &b->addr;

			if (ntohl(aIn->sin_addr.s_addr) != ntohl(bIn->sin_addr.s_addr)) {
				return false;
			}
			if (ntohs(aIn->sin_port) != ntohs(bIn->sin_port)) {
				return false;
			}

			return true;
		}
		else if (a->addr.ss_family == AF_INET6) {
			struct sockaddr_in6* aIn = (struct sockaddr_in6*) &a->addr;
			struct sockaddr_in6* bIn = (struct sockaddr_in6*) &b->addr;

			if (memcmp(
				aIn->sin6_addr.s6_addr, bIn->sin6_addr.s6_addr, sizeof(aIn->sin6_addr.s6_addr)
			) != 0) {
				return false;
			}

			if (ntohs(aIn->sin6_port) != ntohs(bIn->sin6_port)) {
				return false;
			}

			if (aIn->sin6_flowinfo != bIn->sin6_flowinfo) {
				return false;
			}

			if (aIn->sin6_scope_id != bIn->sin6_scope_id) {
				return false;
			}

			return true;
		}
	#else
		Error("Network sockets not available");
	#endif

	return false;
}

void NetSocketAddr_StringAddr(NetSocketAddr* addr, char* dest, size_t size) {
	#ifdef AE_NET_SOCKET
		inet_ntop(AF_INET, &(((struct sockaddr_in *) &addr->addr)->sin_addr), dest, size);
	#else
		Error("Network sockets not available");
	#endif
}
