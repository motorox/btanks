#include "udp_socket.h"
#include "ioexception.h"

#ifdef WIN32
#	include "Winsock2.h"
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <netinet/ip.h> /* superset of previous */
#	include <arpa/inet.h>
#	include <netdb.h>
#endif              

#ifdef WIN32
#	ifndef socklen_t 
#		define socklen_t int
#	endif

#	ifndef in_addr_t
#		define in_addr_t unsigned long
#	endif
#endif


using namespace mrt;

void UDPSocket::listen(const std::string &bindaddr, const unsigned port, const bool reuse) {
	create();

	int on = 1;
	if (reuse)
		setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (!bindaddr.empty()) {
		addr.sin_addr.s_addr = inet_addr(bindaddr.c_str());
	}
	
	if (bind(_sock, (const sockaddr *)&addr, sizeof(addr)) == -1)
		throw_io(("bind"));
	
//	if (::listen(_sock, 10) == -1)
//		throw_io(("listen"));	
}

void UDPSocket::connect(const std::string &host, const int port) {
	create();
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(host.c_str());

	if (addr.sin_addr.s_addr == (in_addr_t)-1) {
		//try to resolve host
		hostent *he = gethostbyname(host.c_str());
		if (he == NULL)
			throw_ex(("host '%s' was not found", host.c_str()));
		addr.sin_addr = *(struct in_addr*)(he->h_addr_list[0]);
	}
	
	LOG_DEBUG(("connect %s:%d", inet_ntoa(addr.sin_addr), port));
	if (::connect(_sock, (const struct sockaddr*)&addr, sizeof(addr))	 == -1)
		throw_io(("connect"));
}

void UDPSocket::create() {
	Socket::create(PF_INET, SOCK_DGRAM, 0);
}

const int UDPSocket::send(const Socket::addr &addr, const void *data, const int len) const {
	sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = addr.ip;
	sockaddr.sin_port = htons(addr.port);
	
	return ::sendto(_sock, (const char *)data, len, 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
}

const int UDPSocket::recv(Socket::addr &addr, void *data, const int len) const {
	sockaddr_in sockaddr;
	socklen_t socklen = sizeof(sockaddr);
	memset(&sockaddr, 0, sizeof(sockaddr));

	sockaddr.sin_family = AF_INET;

	int r = ::recvfrom(_sock, (char *)data, len, 0, (struct sockaddr *)&sockaddr, &socklen);
	addr.ip = sockaddr.sin_addr.s_addr;
	addr.port = ntohs(sockaddr.sin_port);
	return r;
}