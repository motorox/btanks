#ifndef CLUNK_SAMPLE_H__
#define CLUNK_SAMPLE_H__

/* libclunk - realtime 2d/3d sound render library
 * Copyright (C) 2005-2008 Netive Media Group
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


#include <SDL_audio.h>
#include "export_clunk.h"
#include "mrt/chunk.h"

namespace mrt {
	class Chunk;
}

namespace clunk {
class Context;
class CLUNKAPI Sample {
public: 
	~Sample();
	void init(const mrt::Chunk &data, int rate, const Uint16 format, const Uint8 channels);
	void generateSine(const int freq, const float len);
	
private: 	
	friend class Context;
	friend class Source;
	
	Sample(Context *context);

	Sample(const Sample &);
	const Sample& operator=(const Sample &);

	Context *context;
	SDL_AudioSpec spec;
	mrt::Chunk data;
};
}

#endif

