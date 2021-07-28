//◦ Playrix ◦
#include "pch.h"
#include "../helpers/readwrite.h"
#include <runtime/serialization/json.h>
#include <runtime/serialization/runtimevaluebuilder.h>
#include <runtime/utils/strings.h>

using namespace Runtime;
using namespace testing;

namespace {


struct GenericData
{
	int id;
	std::string type;
	RuntimeValue::Ptr data1;
	RuntimeValue::Ptr data2;

#pragma region Class info
	CLASS_INFO(
		CLASS_FIELDS(
			CLASS_FIELD(id),
			CLASS_FIELD(type),
			CLASS_FIELD(data1),
			CLASS_FIELD(data2)
		)
	)
#pragma endregion
};



template<typename T>
std::string JsonStringify(const T& value) {
	std::string buffer;
	StringWriter writer{buffer};
	Serialization::JsonWrite(writer, runtimeValueRef(value), {}).ignore();
	
	return buffer;
}

template<typename T>
T JsonParse(std::string_view buffer) {
	StringReader reader{buffer};
	T value{};
	RuntimeValue::assign(runtimeValueRef(value), *Serialization::JsonParse(reader)).rethrowIfException();

	return value;
}

template<typename T>
testing::AssertionResult CheckPrimitive(T value, std::string_view expectedStr) {
	using namespace Core;

	const std::string text = JsonStringify(value);
	if (!Strings::icaseEqual(text, expectedStr)) {
		return testing::AssertionFailure() << Format::format("Invalid json string:({}), expected:({})", text, expectedStr);
	}

	const T parsedValue = JsonParse<T>(text);
	if (parsedValue != value) {
		return testing::AssertionFailure() << Format::format("Invalid json parse value on type:({})", typeid(T).name());
	}

	return testing::AssertionSuccess();
}


TEST(Test_SerializationJson, ReadWritePrimtive)
{
	EXPECT_TRUE(CheckPrimitive(10ui16, "10"));
	EXPECT_TRUE(CheckPrimitive(-236i32, "-236"));
	EXPECT_TRUE(CheckPrimitive(101.75, "101.75"));
	EXPECT_TRUE(CheckPrimitive(true, "true"));
	EXPECT_TRUE(CheckPrimitive(false, "false"));
	EXPECT_TRUE(CheckPrimitive(std::string{"abc"}, "\"abc\""));
}

TEST(Test_SerializationJson, Keep1) {

	//std::string_view json1 = R"--(
	//	{
	//		"id": 100,
	//		"type": "number",
	//		"data1": 75
	//	}
	//)--";

//	auto value1 = JsonParse<GenericData>(json1);
//	ASSERT_THAT(RuntimeValueCast<int>(value1.data1), Eq(75));

	std::string_view json2 = R"--(
		{
			"id": 222,
			"type": "object",
			"data1": {
				"id": 101,
				"type": "number",
				"data1": 100,
				"data2": 200
			}
		}
	)--";


	auto value2 = JsonParse<GenericData>(json2);

	const auto field21 = RuntimeValueCast<GenericData>(value2.data1);

	ASSERT_THAT(RuntimeValueCast<int>(field21.data1), Eq(100));
	ASSERT_THAT(RuntimeValueCast<int>(field21.data2), Eq(200));
}

}
