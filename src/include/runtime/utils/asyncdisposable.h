#pragma once
#include "com/interface.h"
#include "async/task.h"

namespace Runtime {

struct ABSTRACT_TYPE AsyncDisposable
{
	virtual Async::Task<> disposeAsync() = 0;
};

}
