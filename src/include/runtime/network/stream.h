#pragma once
#include <runtime/com/comptr.h>
#include <runtime/io/asyncreader.h>
#include <runtime/io/asyncwriter.h>
#include <runtime/utils/disposable.h>


namespace Runtime::Network {

struct INTERFACE_API Stream : IRefCounted, Io::AsyncWriter, Io::AsyncReader, Disposable
{
	DECLARE_CLASS_BASE(Io::AsyncWriter, Io::AsyncReader, Disposable)

	using Ptr = ComPtr<Stream>;
};

}
