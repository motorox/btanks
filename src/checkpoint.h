#ifndef BTANKS_CHECKPOINT_H__
#define BTANKS_CHECKPOINT_H__

#include "zbox.h"
#include <string>

class Checkpoint : public ZBox {
public: 
	Checkpoint(const ZBox & zbox, const std::string &name);
	const bool final() const;
private: 
	std::string _name;
};

#endif

