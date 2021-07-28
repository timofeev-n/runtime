//◦ Playrix ◦
#pragma once
#include <runtime/utils/result.h>
#include <runtime/serialization/runtimevalue.h>
#include <runtime/serialization/serialization.h>
#include <string_view>

namespace Runtime::Serialization {

/**
*/
struct JsonSettings
{
	bool pretty = false;
};

Result<> JsonWrite(Io::Writer&, const RuntimeValue::Ptr&, JsonSettings = {});

Result<RuntimeValue::Ptr> JsonParse(Io::Reader&);

Result<RuntimeValue::Ptr> JsonParseString(std::string_view);

} //namespace Runtime::Serialization
