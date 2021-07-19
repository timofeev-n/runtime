#pragma once
#include <runtime/com/ianything.h>

namespace Runtime::Network {

struct ABSTRACT_TYPE SocketControl
{
	virtual size_t setInboundBufferSize(size_t size) = 0;
};

}
