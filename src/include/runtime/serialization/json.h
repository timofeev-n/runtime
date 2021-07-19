#pragma once
#include <runtime/utils/result.h>
#include <runtime/serialization/runtimevalue.h>
#include <runtime/serialization/serialization.h>


namespace Runtime::Serialization {

/**
*/
struct JsonSettings
{
	bool pretty = false;
};

Result<> jsonWrite(Io::Writer&, const RuntimeValue::Ptr&, JsonSettings);

Result<RuntimeValue::Ptr> jsonParse(Io::Reader&);

} //namespace Runtime::Serialization
