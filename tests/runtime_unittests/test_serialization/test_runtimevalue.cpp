#include "pch.h"
#include <runtime/utils/scopeguard.h>
#include <runtime/serialization/runtimevaluebuilder.h>

// #include <runtime/utils/stringconv.h>
// #include <runtime/utils/uid.h>



namespace {

struct OneFieldStruct1
{
	int field = 77;

	CLASS_INFO(
		CLASS_FIELDS(
			CLASS_FIELD(field)
		)
	)
	
};

struct FooObject1
{
	int field1 = 1;
	std::vector<unsigned> fieldArr;
	OneFieldStruct1 fieldObj;

	CLASS_INFO(
		CLASS_FIELDS(
			CLASS_FIELD(field1),
			CLASS_FIELD(fieldArr),
			CLASS_FIELD(fieldObj)
		)
	)
};
}


TEST(Test_RuntimeValue, PrimitiveValue)
{
	int i;
	auto iValue = Runtime::runtimeValue(i);
	static_assert(std::is_assignable_v<Runtime::RuntimeIntegerValue&, decltype(iValue)::type&>);

}


TEST(Test_RuntimeValue, OptionalValue)
{
	boost::optional<std::string> opt;
	auto value = Runtime::runtimeValue(opt);
	static_assert(std::is_assignable_v<Runtime::RuntimeOptionalValue&, decltype(value)::type&>);
}


TEST(Test_RuntimeValue, ArrayValue)
{
	std::vector<int> arr;
	auto value = Runtime::runtimeValue(arr);
	static_assert(std::is_assignable_v<Runtime::RuntimeArray&, decltype(value)::type&>);
}


TEST(Test_RuntimeValue, TupleValue)
{
	std::tuple<std::string> arr;
	auto value = Runtime::runtimeValue(arr);
	static_assert(std::is_assignable_v<Runtime::RuntimeTuple&, decltype(value)::type&>);
}

TEST(Test_RuntimeValue, DictionaryValue)
{
	std::map<std::string, int> arr;
	auto value = Runtime::runtimeValue(arr);
	static_assert(std::is_assignable_v<Runtime::RuntimeDictionary&, decltype(value)::type&>);

}

TEST(Test_RuntimeValue, ObjectValue) {
	FooObject1 obj;
	auto value = Runtime::runtimeValue(obj);

	static_assert(std::is_assignable_v<Runtime::RuntimeObject&, decltype(value)::type&>);
}

