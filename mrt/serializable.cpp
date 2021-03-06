/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "serializable.h"
#include "serializator.h"
#include "chunk.h"

using namespace mrt;

void Serializable::serialize2(mrt::Chunk &d) const {
	mrt::Serializator s;
	serialize(s);
	s.finalize(d);
}

void Serializable::deserialize2(const mrt::Chunk &d) {
	mrt::Serializator s(&d);
	deserialize(s);
}

Serializable::~Serializable() {}
