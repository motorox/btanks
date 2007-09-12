#ifndef __BTANKS_SYS_SOCKET_H__
#define __BTANKS_SYS_SOCKET_H__
/* M-runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "export_mrt.h"

namespace mrt {
	class MRTAPI Socket {
	public:
		struct addr {
			addr() : ip(0), port(0) {}
			unsigned ip;
			unsigned port;
			const bool operator==(const addr &other) const {
				return ip == other.ip && port == other.port;
			}
		};

		Socket();
		static void init();
		void create(const int af, int type, int protocol);
		
		void close(); 
		virtual ~Socket();
	protected: 
		int _sock;

		friend class SocketSet;
	};
}

#endif
