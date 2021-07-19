#pragma once
#include "networkexport.h"
#include "stream.h"
#include <optional>

namespace Runtime::Network {


struct ABSTRACT_TYPE Server: IRefCounted, Disposable
{
	DECLARE_CLASS_BASE(Disposable)

	using Ptr = ComPtr<Server>;

	static Async::Task<Server::Ptr> listen(std::string address, std::optional<unsigned> = std::nullopt);


	virtual Async::Task<Stream::Ptr> accept() = 0;
};

}
