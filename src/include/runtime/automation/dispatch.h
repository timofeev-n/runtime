//◦ Playrix ◦
#pragma once

#include <runtime/serialization/runtimevalue.h>
#include <string_view>

namespace Runtime {

struct ABSTRACT_TYPE Dispatch : virtual IRefCounted
{
	using Ptr = ComPtr<Dispatch>;

	virtual ComPtr<> Invoke(std::string_view contract, std::string_view method, Runtime::RuntimeReadonlyCollection::Ptr arguments) = 0;
};

}
