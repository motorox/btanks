#include "config.h"
#include "scanner.h"
#include "message.h"
#include "mrt/ioexception.h"
#include "mrt/logger.h"
#include "mrt/chunk.h"
#include "mrt/socket_set.h"
#include "mrt/udp_socket.h"
#include "mrt/serializator.h"

#ifdef WIN32
#	include <Winsock2.h>
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h> /* superset of previous */
#endif

Scanner::Scanner() : _running(true), _scan(true) {
	start();
}

Scanner::~Scanner() {
	LOG_DEBUG(("stopping scanner..."));
	_running = false;
	wait();
}

const int Scanner::run() {
TRY {
	GET_CONFIG_VALUE("multiplayer.bind-address", std::string, bindaddr, std::string());
	GET_CONFIG_VALUE("multiplayer.port", int, port, 9876);

	LOG_DEBUG(("searching for servers at port %d", port));

	mrt::UDPSocket udp_sock;
	udp_sock.listen(bindaddr, port, false);
	//udp_sock.create();
	udp_sock.setBroadcastMode(1);
	LOG_DEBUG(("udp socket started..."));
	
	mrt::SocketSet set; 
	set.add(udp_sock, mrt::SocketSet::Exception | mrt::SocketSet::Read);
	
	while(_running) {
		if (_scan) {
			mrt::Serializator s;
			Message m(Message::ServerDiscovery);
			mrt::Chunk data;
			m.serialize2(data);
	
			udp_sock.broadcast(data, port);	
			_scan = false;
		}

		if (set.check(100) == 0)
			continue;
		
		if (set.check(udp_sock, mrt::SocketSet::Exception)) {
			throw_io(("udp_socket"));
		} else if (set.check(udp_sock, mrt::SocketSet::Read)) {
			LOG_DEBUG(("incoming packet!"));
			mrt::Socket::addr addr;
			mrt::Chunk data;
			data.setSize(1500);
			int r = udp_sock.recv(addr, data.getPtr(), data.getSize());
			if (r == 0 || r == -1)
				throw_io(("udp_sock.read"));
			data.setSize(r);
			LOG_DEBUG(("data from addr %s: %s", addr.getAddr().c_str(), data.dump().c_str()));
		}
	}
} CATCH("run", return 1;)
	return 0;
}
