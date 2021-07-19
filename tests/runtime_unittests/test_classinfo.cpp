
#include "pch.h"
#include <runtime/meta/classinfo.h>


using namespace testing;
using namespace Runtime;


namespace {

struct SuperBase
{};

struct Base1 : virtual SuperBase
{
	CLASS_INFO(
		CLASS_BASE(SuperBase)
	)

};

struct Base2
{};

struct Base3 : Base2, virtual SuperBase
{
	CLASS_INFO(
		CLASS_BASE(Base2, SuperBase)
	)
};

struct MyClass : Base1, Base3
{
	CLASS_INFO(
		CLASS_BASE(Base1, Base3)
	)
};



struct StructWithFields
{
	static constexpr const char* DefaultStr = "default";
	static constexpr int DefaultInt = 77;

	int intField = DefaultInt;
	std::string strField = DefaultStr;

	CLASS_INFO(
		CLASS_FIELDS(
			CLASS_FIELD(intField),
			CLASS_FIELD(strField)
		)
	)
};


class ClassWithNamedFields
{
public:
	static constexpr int DefaultInt1 = 11;
	static constexpr int DefaultInt2 = 22;

	CLASS_INFO(
		CLASS_FIELDS(
			CLASS_NAMED_FIELD(m_field1, "field1"),
			CLASS_NAMED_FIELD(m_field2, "field2")
		)
	)

private:
	int m_field1 = DefaultInt1;
	int m_field2 = DefaultInt2;
};


class ClassCompoundFileds : public StructWithFields, public ClassWithNamedFields
{
	CLASS_INFO(
		CLASS_BASE(StructWithFields, ClassWithNamedFields),
		CLASS_FIELDS(
			CLASS_FIELD(m_field3),
			CLASS_FIELD(m_field4)
		)
	)

private:
	int m_field3 = DefaultInt1;
	float m_field4 = DefaultInt2;
};


template<typename ExpectedT, typename ... ExpectedAttribs, typename T, typename ... Attribs>
AssertionResult checkField(const Meta::FieldInfo<T, Attribs...> field, std::string_view name, std::optional<T> value = std::nullopt)
{
	static_assert(std::is_same_v<ExpectedT, T>, "field type mismatch");
	static_assert((std::is_same_v<ExpectedAttribs, Attribs> && ...), "field attribute type mismatch");

	if (!field.name() != name) {
		return AssertionFailure() << format("Field name ({0}) mismatch ({1})", field.name(), name);
	}

	return AssertionSuccess();
}

template<size_t ... I, typename ... Fields, typename ... ExpectedFields>
AssertionResult checkFieldsHelper(std::index_sequence<I...>, const std::tuple<Fields...>& fields, std::tuple<ExpectedFields...>& expectedFields) {

	using namespace Runtime::Meta;

	static_assert(sizeof ... (Fields) == sizeof ... (ExpectedFields), "Invalid fields count");
	static_assert((std::is_same_v<Fields, ExpectedFields> && ...), "Field value type mismatch");

	const bool success = ((std::get<I>(fields).name() == std::get<I>(expectedFields).name()) && ... );
	if (!success) {
		return AssertionFailure() << "Field name mismatch";
	}

	return AssertionSuccess();

}


template<typename ... Fields, typename ... ExpectedFields>
AssertionResult checkFields(const std::tuple<Fields...>& fields, ExpectedFields... expected) {
	return checkFieldsHelper(std::make_index_sequence<sizeof ... (Fields)>{}, fields, std::tuple {expected...});
}


}

TEST(Common_ClassInfo, Bases) {

	using ActualDirectBase = Meta::ClassDirectBase<MyClass>;
	static_assert(std::is_same_v<ActualDirectBase, TypeList<Base1, Base3>>);

	using ActualAllBase = Meta::ClassAllBase<MyClass>;
	static_assert(std::is_same_v<ActualAllBase, TypeList<Base1, Base3, SuperBase, Base2, SuperBase>>);

	using ActualAllUniqueBase = Meta::ClassAllUniqueBase<MyClass>;  
	static_assert(std::is_same_v<ActualAllUniqueBase, TypeList<Base1, Base3, SuperBase, Base2>>);
}


TEST(Common_ClassInfo, Fields) {
	const StructWithFields instance;
	auto fields = Meta::classFields(instance);

	const auto result = checkFields(fields,
		Meta::FieldInfo<const int>(&instance.intField, "intField"),
		Meta::FieldInfo<const std::string>(&instance.strField, "strField")
	);

	ASSERT_TRUE(result);
}


TEST(Common_ClassInfo, NamedFields) {
	ClassWithNamedFields instance;
	auto fields = Meta::classFields(instance);

	const auto result = checkFields(fields,
		Meta::FieldInfo<int>(nullptr, "field1"),
		Meta::FieldInfo<int>(nullptr, "field2")
	);

	ASSERT_TRUE(result);
}

TEST(Common_ClassInfo, FieldInheritance) {
	ClassCompoundFileds instance;
	auto fields = Meta::classFields(instance);

	const auto result = checkFields(fields,
		Meta::FieldInfo<int>(nullptr, "intField"),
		Meta::FieldInfo<std::string>(nullptr, "strField"),
		Meta::FieldInfo<int>(nullptr, "field1"),
		Meta::FieldInfo<int>(nullptr, "field2"),
		Meta::FieldInfo<int>(nullptr, "m_field3"),
		Meta::FieldInfo<float>(nullptr, "m_field4")
	);

	ASSERT_TRUE(result);
}

