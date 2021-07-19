#include "pch.h"
#include <runtime/serialization/valuerepresentation.h>
#include <runtime/utils/tostring.h>
#include <runtime/utils/tostring.h>
#include <boost/optional.hpp>

namespace {

struct CustomPrimitive
{};

struct CustomObject
{
	int field1;
	CustomPrimitive field2;

	CLASS_INFO(
		CLASS_FIELDS(
			CLASS_FIELD(field1),
			CLASS_FIELD(field2)
		)
	)
};

}

TEST(Test_ValueRepresentation, CheckIsPrimitive)
{
	using namespace Runtime;

	static_assert(IsPrimitiveRepresentable<int>);
	static_assert(IsPrimitiveRepresentable<unsigned>);
	static_assert(IsPrimitiveRepresentable<float>);
	static_assert(IsPrimitiveRepresentable<std::string>);
	static_assert(IsPrimitiveRepresentable<std::string_view>);
	static_assert(IsPrimitiveRepresentable<CustomPrimitive>);

	static_assert(!IsPrimitiveRepresentable<boost::optional<CustomPrimitive>>);
	static_assert(!IsPrimitiveRepresentable<std::vector<std::string>>);
}

TEST(Test_ValueRepresentation, CheckIsString)
{
	using namespace Runtime;

	static_assert(ToStringIsApplicable<std::string>);
	static_assert(ToStringIsApplicable<std::string_view>);
}


TEST(Test_ValueRepresentation, CheckIsOptional)
{
	using namespace Runtime;

	static_assert(IsOptionalRepresentable<boost::optional<float>>);
}

TEST(Test_ValueRepresentation, CheckIsArray)
{
	using namespace Runtime;

	static_assert(IsArrayRepresentable<std::vector<std::string>>);
	static_assert(IsArrayRepresentable<std::list<std::string>>);
	static_assert(IsArrayRepresentable<std::array<std::string,10>>);
}

TEST(Test_ValueRepresentation, CheckIsTuple)
{
	using namespace Runtime;

	static_assert(IsTupleRepresentable<std::tuple<>>);
	static_assert(IsTupleRepresentable<std::tuple<float, int, std::string>>);
}

TEST(Test_ValueRepresentation, CheckIsDictionary)
{
	using namespace Runtime;

	static_assert(IsDictionaryRepresentable<std::map<std::wstring, CustomPrimitive>>);
}

TEST(Test_ValueRepresentation, CheckIsObject)
{
	using namespace Runtime;

	static_assert(IsObjectRepresentable<CustomObject>);
	static_assert(!IsObjectRepresentable<CustomPrimitive>);
}
