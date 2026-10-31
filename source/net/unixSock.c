#ifdef AE_NET_UNIX

#include <string.h>
#include <time.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include "../log.h"
#include "../data.h"
#include "../socket.h"

#ifdef PLATFORM_3DS
	#include <3ds.h>
	#include <malloc.h>

	#define SOC_ALIGN       0x1000
	#define SOC_BUFFERSIZE  0x100000

	static uint32_t* socBuf = NULL;
	
#endif

void NetSocket_Init(void) {
	#ifdef PLATFORM_3DS	
		socBuf = (uint32_t*) memalign(SOC_ALIGN, SOC_BUFFERSIZE);

		if (!socBuf) {
			Error("Failed to allocate SOC buffer");
		}

		int error = socInit(socBuf, SOC_BUFFERSIZE);

		if (error != 0) {
			Error("socInit failed: %.8x", (unsigned int) error);
		}
	#endif
}

void NetSocket_End(void) {
	#ifdef PLATFORM_3DS
		socExit();
	#endif
}

bool NetSocket_New(NetSocket* sock, int protocol) {
	int type = 0;

	if (protocol == SOCKET_PROTOCOL_TCP)      type = SOCK_STREAM;
	else if (protocol == SOCKET_PROTOCOL_UDP) type = SOCK_DGRAM;

	sock->fd       = socket(AF_INET, type, 0);
	sock->protocol = protocol;
	sock->sendLen  = 4;

	*((uint32_t*) sock->sendData) = 0;

	if (sock->fd == -1) {
		Log("Failed to create socket: %s", strerror(errno));
		return false;
	}

	int reuse = 1;
	int res = setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, (void*) &reuse, sizeof(reuse));

	if (res < 0) {
		Log("warning: Failed to set reuse address option");
	}

	return true;
}

bool NetSocket_Bind(NetSocket* sock, uint16_t port) {
	struct sockaddr_in addr;

	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(port);

	if (bind(sock->fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
		Log("bind failed: %s", strerror(errno));
		return false;
	}
	
	return true;
}

bool NetSocket_Listen(NetSocket* sock, int backlog) {
	if (sock->protocol == SOCKET_PROTOCOL_UDP) {
		Error("Called listen on UDP socket");
	}

	if (listen(sock->fd, backlog) < 0) {
		Log("listen failed: %s", strerror(errno));
		return false;
	}
	fcntl(sock->fd, F_SETFL, O_NONBLOCK);
	return true;
}

Socket* NetSocket_Accept(NetSocket* sock) {
	if (sock->protocol == SOCKET_PROTOCOL_UDP) {
		Error("Called accept on UDP socket");
	}

	struct sockaddr_storage addr;

	socklen_t len = sizeof(addr);

	int fd = accept(sock->fd, (struct sockaddr*) &addr, &len);

	if (fd < 0) {
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			return NULL;
		}

		Error("accept error: %s", strerror(errno));
	}

	Socket* ret                  = Socket_AllocSocket();
	ret->value.type              = sock->type;
	ret->value.net.fd            = fd;
	ret->value.net.protocol      = sock->protocol;
	ret->value.net.addr.addr     = addr;
	ret->value.net.addr.addrLen  = (size_t) len;

	fcntl(fd, F_SETFL, O_NONBLOCK);
	return ret;
}

bool NetSocket_Connect(NetSocket* sock, const char* ip, uint16_t port) {
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

	if (connect(sock->fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
		Log("Connection failed: %s", strerror(errno));
		return false;
	}

	fcntl(sock->fd, F_SETFL, O_NONBLOCK);

	return true;
}

size_t NetSocket_DataAvailable(NetSocket* sock) {
	if (sock->protocol == SOCKET_PROTOCOL_UDP) {
		Error("Called DataAvailable on UDP socket");
	}

	#ifdef PLATFORM_3DS
		// slow and bad. TODO: add a buffering system to NetSocket

		if (!NetSocket_IsDataAvailable(sock)) {
			return 0;
		}

		static uint8_t buf[1024];

		ssize_t count = recv(sock->fd, buf, sizeof(buf), MSG_PEEK);

		if (count < 0) {
			return 0;
		}
		else {
			return (size_t) count;
		}
	#else
		int count;
		ioctl(sock->fd, FIONREAD, &count);

		return (size_t) count;
	#endif
}

bool NetSocket_IsDataAvailable(NetSocket* sock) {
	struct pollfd pollfd;

	pollfd.fd     = sock->fd;
	pollfd.events = POLLIN;

	int res = poll(&pollfd, 1, 0);

	if (res < 0) {
		Error("ppoll failed: %s", strerror(errno));
	}

	return (pollfd.revents & POLLIN)? true : false;
}

size_t NetSocket_Receive(NetSocket* sock, void* buf, size_t size) {
	if (sock->protocol == SOCKET_PROTOCOL_UDP) {
		Error("Called receive on UDP socket");
	}

	ssize_t ret = recv(sock->fd, buf, size, 0);
	if (ret <= 0) return 0;
	return (size_t) ret;
}

size_t NetSocket_ReceiveUDP(NetSocket* sock, void* buf, size_t size, NetSocketAddr* addr) {
	addr->addrLen = sizeof(addr->addr);

	ssize_t dgSize = recvfrom(
		sock->fd, buf, size, 0, (struct sockaddr*) &addr->addr, &addr->addrLen
	);

	if (dgSize < 0) {
		Error("recvfrom failed: %s", strerror(errno));
	}

	return (size_t) dgSize;
}

static ssize_t SendUDP(NetSocket* sock) {
	ssize_t ret = send(sock->fd, sock->sendData, sock->sendLen, 0);

	if (ret < 0) {
		Error("sendto failed: %s", strerror(errno));
	}

	sock->sendLen = 4;

	uint32_t sessionID = AE_SWAP_32(sock->sessionID);
	memcpy(sock->sendData, &sessionID, sizeof(sessionID));

	return ret;
}

size_t NetSocket_Send(NetSocket* sock, void* buf, size_t size) {
	ssize_t ret;

	if (sock->protocol == SOCKET_PROTOCOL_UDP) {
		if (sock->sendLen + size >= sizeof(sock->sendData)) {
			SendUDP(sock);
		}

		memcpy(&sock->sendData[sock->sendLen], buf, size);
		sock->sendLen += size;

		if (sock->sendLen == sizeof(sock->sendData)) {
			SendUDP(sock);
		}
	}
	else {
		ret = send(sock->fd, buf, size, 0);
	}

	if (ret <= 0) return 0;
	return (size_t) ret;
}

void NetSocket_Flush(NetSocket* sock) {
	SendUDP(sock);
}

void NetSocket_Close(NetSocket* sock) {
	close(sock->fd);
}

void NetSocket_StringAddr(NetSocket* sock, char* dest, size_t size) {
	inet_ntop(AF_INET, &(((struct sockaddr_in *) &sock->addr)->sin_addr), dest, size);
}

bool NetSocket_GetAddr(NetSocket* sock, NetSocketAddr* out) {
	out->addrLen = sizeof(out->addr);
	return getsockname(sock->fd, (struct sockaddr*) &out->addr, &out->addrLen) == 0;
}

void NetSocket_SetSessionID(NetSocket* sock, uint32_t sessionID) {
	sock->sessionID = sessionID;

	sessionID = AE_SWAP_32(sessionID);
	memcpy(&sock->sendData, &sessionID, sizeof(sessionID));
}

uint16_t NetSocketAddr_Port(NetSocketAddr* addr) {
	if (addr->addr.ss_family == AF_INET) {
		struct sockaddr_in* addr2 = (struct sockaddr_in*) &addr->addr;

		return ntohs(addr2->sin_port);
	}
	#ifdef AE_IPV6
		else if (addr->addr.ss_family == AF_INET6) {
			struct sockaddr_in6* addr2 = (struct sockaddr_in6*) &addr->addr;

			return ntohs(addr2->sin6_port);
		}
	#endif

	abort();
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
		#ifdef AE_IPV6
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
		#endif
	#else
		Error("Network sockets not available");
	#endif

	return false;
}

bool NetSocketAddr_CompareAPort(NetSocketAddr* a, NetSocketAddr* b, uint16_t port) {
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
			if (ntohs(aIn->sin_port) != port) {
				return false;
			}

			return true;
		}
		#ifdef AE_IPV6
			else if (a->addr.ss_family == AF_INET6) {
				struct sockaddr_in6* aIn = (struct sockaddr_in6*) &a->addr;
				struct sockaddr_in6* bIn = (struct sockaddr_in6*) &b->addr;

				if (memcmp(
					aIn->sin6_addr.s6_addr, bIn->sin6_addr.s6_addr, sizeof(aIn->sin6_addr.s6_addr)
				) != 0) {
					return false;
				}

				if (ntohs(aIn->sin6_port) != port) {
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
		#endif
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

#endif
