#include "pch.h"
#include <runtime/utils/scopeguard.h>
#include <runtime/serialization/runtimevaluebuilder.h>

// #include <runtime/utils/stringconv.h>
// #include <runtime/utils/uid.h>

using namespace testing;


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

template<bool IsMutable>
struct RuntimeValueByRef : std::bool_constant<IsMutable>
{
	template<typename T>
	auto operator()(T& value) const {
		return Runtime::runtimeValueRef(value);
	}

	bool CheckMutability(const Runtime::RuntimeValue& value) const {
		return value.isMutable() == IsMutable;
	}
};

template<bool IsMutable>
struct RuntimeValueCopy : std::bool_constant<IsMutable>
{
	template<typename T>
	auto operator()(T&& value) const {
		return Runtime::runtimeValueCopy(std::forward<T>(value));
	}

	bool CheckMutability(const Runtime::RuntimeValue& value) const {
		return value.isMutable();
	}
};


template<typename Factory>
class Test_MakeRuntimeValue: public testing::Test
{
protected:
	
	static constexpr bool IsMutable = Factory::value;

	template<typename T>
	using DeclValue = std::conditional_t<IsMutable, T, std::add_const_t<T>>;

	template<typename NativeType>
	testing::AssertionResult CheckRuntimeIntegerValue(NativeType val) const {
		DeclValue<NativeType> nativeValue = val;
		auto rtValue = _valueFactory(nativeValue);
		static_assert(std::is_assignable_v<Runtime::RuntimeIntegerValue&, decltype(rtValue)::type&>);

		if (!_valueFactory.CheckMutability(*rtValue)) {
			return testing::AssertionFailure() << "Invalid runtime value mutability";
		}

		if (rtValue->isSigned() != std::is_signed_v<NativeType>) {
			return testing::AssertionFailure() << "Invalid runtime value sign: " << typeid(NativeType).name();
		}

		if (rtValue->bits() != sizeof(NativeType)) {
			return testing::AssertionFailure() << "Invalid runtime value bits:" << typeid(NativeType).name();
		}

		if (rtValue->getInt64() != static_cast<int64_t>(val)) {
			return testing::AssertionFailure() << "Get returns unexpected integer value";
		}

		return testing::AssertionSuccess();
	}

	template<typename NativeType>
	testing::AssertionResult CheckRuntimeFloatValue(NativeType val) const {
		DeclValue<NativeType> nativeValue = val;
		auto rtValue = _valueFactory(nativeValue);
		static_assert(std::is_assignable_v<Runtime::RuntimeFloatValue&, decltype(rtValue)::type&>);

		if (!_valueFactory.CheckMutability(*rtValue)) {
			return testing::AssertionFailure() << "Invalid runtime value mutability";
		}

		if (rtValue->bits() != sizeof(NativeType)) {
			return testing::AssertionFailure() << "Invalid runtime value bits:" << typeid(NativeType).name();
		}

		return testing::AssertionSuccess();
	}


	const Factory _valueFactory{};
};

using Factories = testing::Types<RuntimeValueByRef<true>, RuntimeValueByRef<false>, RuntimeValueCopy<true>, RuntimeValueCopy<false>>;
TYPED_TEST_CASE(Test_MakeRuntimeValue, Factories);


TYPED_TEST(Test_MakeRuntimeValue, IntegerValue)
{
	ASSERT_TRUE(CheckRuntimeIntegerValue((char)0));
	ASSERT_TRUE(CheckRuntimeIntegerValue((unsigned char)0));
	ASSERT_TRUE(CheckRuntimeIntegerValue(0i16));
	ASSERT_TRUE(CheckRuntimeIntegerValue(0ui16));
	ASSERT_TRUE(CheckRuntimeIntegerValue(0i32));
	ASSERT_TRUE(CheckRuntimeIntegerValue(0ui32));
	ASSERT_TRUE(CheckRuntimeIntegerValue(0i64));
	ASSERT_TRUE(CheckRuntimeIntegerValue(0ui64));
}

TYPED_TEST(Test_MakeRuntimeValue, BooleanValue)
{
	DeclValue<bool> value = true;
	auto rtValue = _valueFactory(value);
	static_assert(std::is_same_v<Runtime::RuntimeBooleanValue::Ptr, decltype(rtValue)>);

	ASSERT_TRUE(_valueFactory.CheckMutability(*rtValue));
	ASSERT_THAT(rtValue->get(), Eq(value));
}

TYPED_TEST(Test_MakeRuntimeValue, FloatPointValue)
{
	ASSERT_TRUE(CheckRuntimeFloatValue(0.f));
	ASSERT_TRUE(CheckRuntimeFloatValue(0.0));
}

TYPED_TEST(Test_MakeRuntimeValue, StringValue)
{
	DeclValue<std::string> str;
	auto rtValue1 = _valueFactory(str);

	//std::string_view strView;
	//auto rtValue2 = _valueFactory(strView);
}

TYPED_TEST(Test_MakeRuntimeValue, OptionalValue)
{


	//DeclValue<boost::optional<std::string>> opt;
	//auto value = _valueFactory(opt);
	//static_assert(std::is_assignable_v<Runtime::RuntimeOptionalValue&, decltype(value)::type&>);
}


TEST(Test_RuntimeValue, ArrayValue)
{
	std::vector<int> arr;
	auto value = Runtime::runtimeValueRef(arr);
	static_assert(std::is_assignable_v<Runtime::RuntimeArray&, decltype(value)::type&>);
}


TYPED_TEST(Test_MakeRuntimeValue, TupleValue)
{
	DeclValue<std::tuple<int, float>> tuple1 = {};

	auto rtValue = _valueFactory(tuple1);
	static_assert(std::is_assignable_v<Runtime::RuntimeTuple&, decltype(rtValue)::type&>);

	ASSERT_TRUE(_valueFactory.CheckMutability(*rtValue));
	ASSERT_THAT(rtValue->size(), Eq(std::tuple_size_v<decltype(tuple1)>));
}

TEST(Test_RuntimeValue, DictionaryValue)
{
	std::map<std::string, int> arr;
	auto value = Runtime::runtimeValueRef(arr);
	static_assert(std::is_assignable_v<Runtime::RuntimeDictionary&, decltype(value)::type&>);

}

TEST(Test_RuntimeValue, ObjectValue) {
	FooObject1 obj;
	auto value = Runtime::runtimeValueRef(obj);

	static_assert(std::is_assignable_v<Runtime::RuntimeObject&, decltype(value)::type&>);
}


TEST(Test_RuntimeValue, RuntimeValueRef) {
	FooObject1 obj;
	auto value = Runtime::runtimeValueRef(obj);

	//auto valueRef = Runtime::runtimeValueRef(value);
	//static_assert(std::is_same_v<decltype(valueRef), decltype(value)>);

	//auto valueCopy = Runtime::runtimeValueCopy(value);
	//static_assert(std::is_same_v<decltype(valueCopy), decltype(value)>);
}


