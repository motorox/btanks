#include "monitor.h"
#include "mrt/chunk.h"
#include "mrt/logger.h"
#include "mrt/socket_set.h"
#include "mrt/tcp_socket.h"
#include "connection.h"

#ifdef WIN32
#	include "Winsock2.h"
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h> /* superset of previous */
#	include <arpa/inet.h>
#	include <netdb.h>
#endif              


Monitor::Task::Task(const int id) : id(id), data(new mrt::Chunk), pos(0), len(0), size_task(false) {}
Monitor::Task::Task(const int id, const mrt::Chunk &d) : id(id), data(new mrt::Chunk(d)), pos(0), len(data->getSize()), size_task(false) {}
Monitor::Task::Task(const int id, const int size) : id(id), data(new mrt::Chunk(size)), pos(0), len(data->getSize()), size_task(false) {}
void Monitor::Task::clear() { delete data; pos = len = 0; }

Monitor::Monitor() : _running(false) {
}

void Monitor::add(const int id, Connection *c) {
	sdlx::AutoMutex m(_connections_mutex);
	_connections[id] = c;
}
	
void Monitor::send(const int id, const mrt::Chunk &data) {
	int size = data.getSize();
	assert(size < 65536);

	Task *t = new Task(id, size + 2);

	unsigned short nsize = htons((short)size);
	memcpy(data.getPtr(), &nsize, 2);
	memcpy((unsigned char *)t->data->getPtr() + 2, data.getPtr(), size);
	
	sdlx::AutoMutex m(_connections_mutex);
	_send_q.push_back(t);
}

Monitor::TaskQueue::iterator Monitor::findTask(TaskQueue &queue, const int conn_id) {
	Monitor::TaskQueue::iterator i;
	for(i = queue.begin(); i != queue.end(); ++i) 
		if ((*i)->id == conn_id)
			return i;
	return i;
}


const bool Monitor::recv(int &id, mrt::Chunk &data) {
	sdlx::AutoMutex m(_result_mutex);
	if (_result_q.empty())
		return false;
	
	id = _result_q.front()->id;
	data = *_result_q.front()->data;
	_result_q.front()->clear();
	
	_result_q.pop_front();
	return true;
}


void Monitor::eraseTask(TaskQueue &q, const TaskQueue::iterator &i) {
	(*i)->clear();
	q.erase(i);
}

const int Monitor::run() {
	_running = true;
	LOG_DEBUG(("network monitor thread was started..."));
	while(_running) {
		int n;
		{
			sdlx::AutoMutex m(_connections_mutex);
			n = _connections.size();
		}
		if (n == 0) {
			SDL_Delay(100);
			continue;
		} 

		mrt::SocketSet set; 
		for(ConnectionMap::iterator i = _connections.begin(); i != _connections.end(); ++i) {
			set.add(i->second->sock);
		}
		if (set.check(10) == 0)
			continue;
		
		for(ConnectionMap::iterator i = _connections.begin(); i != _connections.end();) {
			const mrt::TCPSocket *sock = i->second->sock;
			if (set.check(sock, mrt::SocketSet::Exception)) {
				//fixme: cleanup all tasks with this connection, notify upper layer 
				_connections.erase(i++);
				continue;
			}

			if (set.check(sock, mrt::SocketSet::Read)) {

				TaskQueue::iterator ti = findTask(_recv_q, i->first);
				if (ti == _recv_q.end()) {
					Task *t = new Task(i->first, 2);
					t->size_task = true;
					_recv_q.push_back(t);
				
					ti = findTask(_recv_q, i->first);
					assert(ti != _recv_q.end());
				}
				Task *t = *ti;
			
				int estimate = t->len - t->pos;
				assert(estimate > 0);
			
				int r = sock->recv((char *)(t->data->getPtr()) + t->pos, t->len);
				if (r == -1 || r == 0) 
					goto skip_read;
					
				t->pos += r;
				assert(t->pos <= t->len);
			
				if (t->pos == t->len) {
					if (t->size_task) {
						unsigned short len = ntohs(*((unsigned short *)(t->data->getPtr())));
						LOG_DEBUG(("packet size = %u", len));
						eraseTask(_recv_q, ti);
						
						Task *t = new Task(i->first, len);
						_recv_q.push_back(t);
					} else {
						LOG_DEBUG(("recv'ed %u bytes", t->len));
						_result_q.push_back(t);
						_recv_q.erase(ti);
					}
				}
			}
		skip_read:

			if (set.check(sock, mrt::SocketSet::Write)) {
				TaskQueue::iterator ti = findTask(_send_q, i->first);
				if (ti != _send_q.end()) {
					Task *t = *ti;

					int estimate = t->len - t->pos;
					assert(estimate > 0);
			
					int r = sock->send((char *)(t->data->getPtr()) + t->pos, t->len);
					if (r == -1 || r == 0) 
						goto skip_write;

					t->pos += r;
					assert(t->pos <= t->len);
					if (t->pos == t->len) {
						LOG_DEBUG(("sent %u bytes", t->len));
						eraseTask(_send_q, ti);
					}
				}
			}
		skip_write:
			
			++i;
		}
		
	}
	return 0;
}


Monitor::~Monitor() {
	_running = false;
	wait();

	for(ConnectionMap::iterator i = _connections.begin(); i != _connections.end(); ++i) {
		delete i->second;
	}

	for(TaskQueue::iterator i = _send_q.begin(); i != _send_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
	for(TaskQueue::iterator i = _recv_q.begin(); i != _recv_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
	for(TaskQueue::iterator i = _result_q.begin(); i != _result_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
}
