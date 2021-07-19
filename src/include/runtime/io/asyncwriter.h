#pragma once
#include <runtime/async/task.h>
#include <runtime/com/ianything.h>
#include <runtime/memory/bytesbuffer.h>


namespace Runtime::Io {

/// <summary>
///
/// </summary>
struct INTERFACE_API AsyncWriter : virtual IAnything
{
	virtual Async::Task<> write(BufferView) = 0;
};

}
