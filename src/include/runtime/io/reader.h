#pragma once

//#include "bytesbuffer.h"
//#include "async/tasks.h"
#include <runtime/com/ianything.h>

namespace Runtime::Io {

struct ABSTRACT_TYPE Reader : virtual IAnything{
	virtual size_t read(std::byte* buffer, size_t readCount) = 0;
};

/// <summary>
///
// /// </summary>
// struct INTERFACE_API AsyncReader : virtual Com::IAnything
// {
// 	virtual Async::Task<BytesBuffer> read() = 0;
// };


}

