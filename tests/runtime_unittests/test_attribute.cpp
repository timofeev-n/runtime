#include "pch.h"
#include <runtime/meta/attribute.h>

using namespace Runtime;
using namespace testing;

namespace {

struct StrAttribute : Meta::Attribute
{
	static const std::string defaultValue() { return "default"; }

	std::string value;
 
	StrAttribute(std::string value_): value(std::move(value_))
	{}
};

struct IntAttribute : Meta::Attribute
{
	static constexpr int DefaultValue = 777;

	int value;

	IntAttribute(int value_): value(value_)
	{}
};


struct Attribute1 : Meta::Attribute
{};

template<typename T>
struct Attribute1Value : Meta::AttributeValue<Attribute1> {
	using type = T;
};


struct Attribute2 : Meta::Attribute
{};

template<typename T>
struct Attribute2Value : Meta::AttributeValue<Attribute2> {
	using type = T;
};

class MyType1 {

	CLASS_ATTRIBUTES
		StrAttribute{StrAttribute::defaultValue()},
		Attribute1Value<int>{}

	END_CLASS_ATTRIBUTES
};



}




TEST(Common_Attribute, AttributeDefined)
{
	static_assert(Meta::AttributeDefined<MyType1, StrAttribute>);
	static_assert(Meta::AttributeDefined<MyType1, Attribute1>);
	static_assert(!Meta::AttributeDefined<MyType1, IntAttribute>);
	static_assert(!Meta::AttributeDefined<MyType1, Attribute2>);
}



TEST(Common_Attribute, AttributeValueType)
{
	static_assert(std::is_same_v<Meta::AttributeValueType<MyType1, StrAttribute>, StrAttribute>);
	static_assert(std::is_same_v<Meta::AttributeValueType<MyType1, Attribute1>, Attribute1Value<int>>);
}


TEST(Common_Attribute, AttributeValue)
{
	const auto value1 = Meta::attributeValue<MyType1, StrAttribute>().value;
	ASSERT_THAT(value1, Eq(StrAttribute::defaultValue()));
}
