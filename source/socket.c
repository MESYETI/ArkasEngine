#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include "mem.h"
#include "util.h"
#include "socket.h"

Socket* sockets = NULL;

static Socket* AllocSocket(void) {
	if (sockets == NULL) {
		sockets = SafeMalloc(sizeof(Socket));
		sockets->prev = NULL;
		sockets->next = NULL;
	}
	else {
		Socket* socket = SafeMalloc(sizeof(Socket));
		socket->prev   = NULL;
		socket->next   = sockets;
		sockets        = socket;
	}

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
				return NULL;
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
				break;
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
				if (listen(sock->value.net.fd, backlog) < 0) {
					Log("listen failed: %s", strerror(errno));
					return false;
				}
				fcntl(sock->value.net.fd, F_SETFL, O_NONBLOCK);
				return true;
			#else
				break;
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
				struct sockaddr addr;
				socklen_t len;

				int fd = accept(sock->value.net.fd, &addr, &len);

				if (fd < 0) return NULL; // TODO: check for actual errors

				Socket* ret             = AllocSocket();
				ret->value.type         = sock->value.type;
				ret->value.net.fd       = fd;
				ret->value.net.protocol = sock->value.net.protocol;
				ret->value.net.addr     = addr;
				ret->value.net.addrLen  = (size_t) len;

				fcntl(fd, F_SETFL, O_NONBLOCK);
				return ret;
			#else
				break;
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
		assert(0);
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
				int count;
				ioctl(sock->value.net.fd, FIONREAD, &count);

				return (size_t) count;
			#else
				break;
			#endif
		}
	}

	assert(0);
}

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
				ssize_t ret = recv(sock->value.net.fd, buf, size, 0);
				if (ret <= 0) return 0;
				return (size_t) ret;
			#else
				break;
			#endif
		}
	}

	assert(0);
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
				ssize_t ret = send(sock->value.net.fd, buf, size, 0);
				if (ret <= 0) return 0;
				return (size_t) ret;
			#else
				break;
			#endif
		}
	}

	assert(0);
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
		sock->next->prev = sock->prev;
	}
	free(sock);
}

void Socket_StringAddr(Socket* sock, char* dest, size_t size) {
	switch (sock->value.type) {
		case SOCKET_TYPE_LOCAL: {
			snprintf(dest, size, "%p", sock);
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
