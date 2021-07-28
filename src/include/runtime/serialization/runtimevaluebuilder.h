#pragma once
#include <runtime/serialization/runtimevalue.h>
#include <runtime/serialization/valuerepresentation.h>
#include <runtime/utils/strings.h>
#include <runtime/utils/result.h>
#include <runtime/utils/tostring.h>
#include <runtime/utils/typeutility.h>
#include <runtime/com/comclass.h>

#include <EngineAssert.h>

namespace Runtime {
namespace Detail {

enum class KeepValueMode
{
	ByReference,
	ByValue
};

template<typename T, KeepValueMode KeepMode>
using KeepedValueType = std::conditional_t<KeepMode == KeepValueMode::ByValue, T, std::add_lvalue_reference_t<T>>;

template<typename>
struct IsBasicString : std::false_type
{};

template<typename ... Traits>
struct IsBasicString<std::basic_string<char, Traits...>> : std::true_type
{};

template<typename T>
inline constexpr bool CanKeepStringByRef = IsBasicString<T>::value;

}

/* -------------------------------------------------------------------------- */
template<typename T,
	std::enable_if_t<std::is_base_of_v<RuntimeValue, T>, int> = 0>
RuntimeValueRef::Ptr runtimeValueRef(const ComPtr<T>& value);

template<typename T,
	std::enable_if_t<std::is_base_of_v<RuntimeValue, T>, int> = 0>
RuntimeValueRef::Ptr runtimeValueRef(ComPtr<T>& value);

//template<typename T>
//ComPtr<T> runtimeValueCopy(RuntimeValueRefWrapper<T> value);


/* -------------------------------------------------------------------------- */
// Integral
template<typename T,
	std::enable_if_t<std::is_integral_v<T>, int> = 0>
ComPtr<RuntimeIntegerValue> runtimeValueRef(T&); //requires (std::is_integral_v<T>);

template<typename T,
	std::enable_if_t<std::is_integral_v<T>, int> = 0>
ComPtr<RuntimeIntegerValue> runtimeValueRef(const T&); // requires (std::is_integral_v<T>);

ComPtr<RuntimeBooleanValue> runtimeValueRef(bool&);

ComPtr<RuntimeBooleanValue> runtimeValueRef(const bool&);


/* -------------------------------------------------------------------------- */
template<typename T,
	std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
ComPtr<RuntimeFloatValue> runtimeValueRef(T&); //requires (std::is_floating_point_v<T>);

template<typename T,
	std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
ComPtr<RuntimeFloatValue> runtimeValueRef(const T&); // requires (std::is_floating_point_v<T>);

/* -------------------------------------------------------------------------- */
//template<typename T,
//	std::enable_if_t<std::is_convertible_v<T, std::string>, int> = 0>
//ComPtr<RuntimeStringValue> runtimeValueRef(T&);

template<typename T,
	std::enable_if_t<std::is_constructible_v<std::string_view, T>, int> = 0>
ComPtr<RuntimeStringValue> runtimeValueRef(T&);

template<typename T,
	std::enable_if_t<std::is_constructible_v<std::string_view, T>, int> = 0>
ComPtr<RuntimeStringValue> runtimeValueRef(const T&);

template<typename ... Traits>
ComPtr<RuntimeStringValue> runtimeValueRef(const std::basic_string<char, Traits...>&);

template<typename ... Traits>
ComPtr<RuntimeStringValue> runtimeValueRef(std::basic_string<char, Traits...>&);

ComPtr<RuntimeStringValue> runtimeValueCopy(std::string_view);

/* -------------------------------------------------------------------------- */
template<typename T,
	std::enable_if_t<
		IsPrimitiveRepresentable<T> &&
		!(std::is_floating_point_v<T> || std::is_integral_v<T>)>, int = 0>
ComPtr<RuntimePrimitiveValue> runtimeValueRef(T&); 


template<typename T,
	std::enable_if_t<
		IsPrimitiveRepresentable<T> &&
		!(std::is_floating_point_v<T> || std::is_integral_v<T>)>, int = 0>
ComPtr<RuntimePrimitiveValue> runtimeValueRef(const T&); 

/* -------------------------------------------------------------------------- */

//template<Meta::OptionalRepresentable T>
template<typename T,
	std::enable_if_t<IsOptionalRepresentable<T>, int> = 0>
ComPtr<RuntimeOptionalValue> runtimeValueRef(T&);

//template<Meta::OptionalRepresentable T>
template<typename T,
	std::enable_if_t<IsOptionalRepresentable<T>, int> = 0>
ComPtr<RuntimeOptionalValue> runtimeValueRef(const T&);

//template<Meta::ArrayRepresentable T>
template<typename T,
	std::enable_if_t<IsArrayRepresentable<T>, int> = 0>
ComPtr<RuntimeArray> runtimeValueRef(T&);

//template<Meta::ArrayRepresentable T>
template<typename T,
	std::enable_if_t<IsArrayRepresentable<T>, int> = 0>
ComPtr<RuntimeArray> runtimeValueRef(const T&);

//template<Meta::TupleRepresentable T>
template<typename T,
	std::enable_if_t<IsTupleRepresentable<T>, int> = 0>
ComPtr<RuntimeTuple> runtimeValueRef(T&);

//template<Meta::TupleRepresentable  T>
template<typename T,
	std::enable_if_t<IsTupleRepresentable<T>, int> = 0>
ComPtr<RuntimeTuple> runtimeValueRef(const T&);

//template<Meta::DictionaryRepresentable T>
template<typename T,
	std::enable_if_t<IsDictionaryRepresentable<T>, int> = 0>
ComPtr<RuntimeDictionary> runtimeValueRef(T&);

//template<Meta::DictionaryRepresentable T>
template<typename T,
	std::enable_if_t<IsDictionaryRepresentable<T>, int> = 0>
ComPtr<RuntimeDictionary> runtimeValueRef(const T&);

//template<Meta::ObjectRepresentable T>
template<typename T,
	std::enable_if_t<IsObjectRepresentable<T>, int> = 0>
ComPtr<RuntimeObject> runtimeValueRef(T&);

//template<Meta::ObjectRepresentable T>
template<typename T,
	std::enable_if_t<IsObjectRepresentable<T>, int> = 0>
ComPtr<RuntimeObject> runtimeValueRef(const T&);

//template<typename T,
//	std::enable_if_t<std::is_base_of_v<RuntimeValue, T>, int> = 0>
//ComPtr<T> runtimeValueRef(ComPtr<T>);


//-----------------------------------------------------------------------------
/**
* 
*/
template<typename T, Detail::KeepValueMode KeepMode>
// requires (std::is_integral_v<T>)
class NativeIntegerValue final : public RuntimeIntegerValue
{
	COMCLASS_(RuntimeIntegerValue)

public:
	static constexpr bool IsMutableValue = !std::is_const_v<T>;

	// using ValueType = std::remove_const_t<T>;

	NativeIntegerValue(T& value): m_value(value)
	{}

	constexpr bool isMutable() const override {
		return IsMutableValue;
	}

	constexpr bool isSigned() const override {
		return std::is_signed_v<T>;
	}

	constexpr size_t bits() const override {
		return sizeof(T);
	}

	void setInt64(int64_t value) override {
		if constexpr (IsMutableValue) {
			m_value = static_cast<T>(value);
		}
		else {
			Halt("Attempt to modify non mutable value");
		}
	}

	void setUint64(uint64_t value) override {
		if constexpr (IsMutableValue) {
			m_value = static_cast<T>(value);
		}
		else {
			Halt("Attempt to modify non mutable value");
		}
	}

	int64_t getInt64() const override
	{
		return static_cast<int64_t>(m_value);
	}

	uint64_t getUint64() const override
	{
		return static_cast<uint64_t>(m_value);
	}

private:
	Detail::KeepedValueType<T, KeepMode> m_value;
};

/**
* 
*/
template<typename BoolType, Detail::KeepValueMode KeepMode>
class NativeBooleanValue final : public RuntimeBooleanValue
{
	COMCLASS_(RuntimeBooleanValue)

public:
	static constexpr bool IsMutableValue = !std::is_const_v<BoolType>;

	NativeBooleanValue(BoolType& value): m_value(value)
	{}

	constexpr bool isMutable() const override {
		return IsMutableValue;
	}

	void set(bool value) override {
		if constexpr (IsMutableValue) {
			m_value = value;
		}
		else {
			Halt("Attempt to modify non mutable value");
		}
	}

	bool get() const override {
		return m_value;
	}

private:

	Detail::KeepedValueType<BoolType, KeepMode> m_value;
};


/**
* 
*/
template<typename T, Detail::KeepValueMode KeepMode>
// requires (std::is_floating_point_v<T>)
class NativeFloatValue final : public RuntimeFloatValue
{
	COMCLASS_(RuntimeFloatValue)

public:
	static constexpr bool IsMutable = !std::is_const_v<T>;
	using ValueType = std::remove_const_t<T>;

	NativeFloatValue(T& value): m_value(value)
	{}

	bool isMutable() const override
	{
		return IsMutable;
	}

	size_t bits() const override
	{
		return sizeof(T);
	}

	void setDouble([[maybe_unused]] double value) override
	{
		if constexpr (!IsMutable) {
			Halt("Attempt to modify non mutable runtime value");
		}
		else {
			m_value = static_cast<T>(value);
		}
	}

	void setSingle([[maybe_unused]] float value) override
	{
		if constexpr (!IsMutable) {
			Halt("Attempt to modify non mutable runtime value");
		}
		else {
			m_value = static_cast<T>(value);
		}
	}

	double getDouble() const override {
		return static_cast<double>(m_value);
	}
	
	float getSingle() const override {
		return static_cast<float>(m_value);
	}

private:
	Detail::KeepedValueType<T, KeepMode> m_value;
};

/**
* String
*/
template<typename T, bool IsMutable, Detail::KeepValueMode KeepMode>
class NativeStringValue; // final : public RuntimeStringValue


template<>
class NativeStringValue<std::string_view, false, Detail::KeepValueMode::ByReference> final : public RuntimeStringValue
{
	COMCLASS_(RuntimeStringValue)

	NativeStringValue(std::string_view str): m_value(str)
	{}

	bool isMutable() const override {
		return false;
	}

	void setUtf8(std::string_view) override {
		Halt("Attempt to change non mutable string value");

	}

	void set(std::wstring) override {
		Halt("Attempt to change non mutable string value");
	}

	std::wstring get() const override {
		Halt("wstring get()");
		return {};
	}

	std::string getUtf8() const override {
		return std::string{m_value};
	}

private:
	std::string_view m_value;
};


template<bool IsMutableValue, Detail::KeepValueMode KeepMode, typename ... Traits>
class NativeStringValue<std::basic_string<char, Traits...>, IsMutableValue, KeepMode> final : public RuntimeStringValue
{
	COMCLASS_(RuntimeStringValue)

public:
	
	using StringType = std::conditional_t<IsMutableValue, std::basic_string<char, Traits...>, const std::basic_string<char, Traits...>>;

	NativeStringValue(StringType& str): m_value(str)
	{}

	NativeStringValue(std::string_view str): m_value(str) {
		static_assert(KeepMode == Detail::KeepValueMode::ByValue);
	}

	bool isMutable() const override {
		return IsMutableValue;
	}

	void setUtf8(std::string_view str) override {
		if constexpr (IsMutableValue) {
			m_value.assign(str.data(), str.length());
		}
		else {
			Halt("Attempt to change non mutable string value");
		}
	}

	void set(std::wstring) override {
		if constexpr (!IsMutableValue) {
			Halt("Set wstring for UTF-8 is not implemented");
		}
	}

	std::wstring get() const override {
		Halt("UTF-8 -> std::wstring converions is not implemented");
		return {};
	}

	std::string getUtf8() const override {
		return std::string{m_value.c_str(), m_value.length()};
	}

private:
	Detail::KeepedValueType<StringType, KeepMode> m_value;
};


// template<Meta::OptionalRepresentable T>
template<typename T, Detail::KeepValueMode KeepMode>
class NativeOptionalValue final : public RuntimeOptionalValue
{
	COMCLASS_(RuntimeOptionalValue)

	using OptionalType = std::remove_cv_t<T>;
	using OptionalOp = OptionalValueOperations<OptionalType>;

public:

	static constexpr bool IsMutableValue = !std::is_const_v<T>;

	NativeOptionalValue(T& optionalValue) : m_optionalValue(optionalValue)
	{}

	bool isMutable() const override {
		return IsMutableValue;
	}

	bool hasValue() const override {
		return OptionalOp::hasValue(m_optionalValue);
	}

	RuntimeValue::Ptr value() const override
	{
		if (!this->hasValue())
		{
			return nothing;
		}

		decltype(auto) value = OptionalOp::value(m_optionalValue);
		return runtimeValueRef(value);
	}

	Result<> setValue(RuntimeValue::Ptr value_) override {

		if constexpr (!IsMutableValue) {
			Halt("Attempt to modify non mutable optional value");
			return Excpt_("Attempt to modify non mutable optional value");
		}
		else {
			if (!value_) {
				OptionalOp::reset(m_optionalValue);
				return success;
			}
		
			decltype(auto) myValue = OptionalOp::hasValue(m_optionalValue) ? OptionalOp::value(m_optionalValue) : OptionalOp::emplace(m_optionalValue);

			return RuntimeValue::assign(runtimeValueRef(myValue), std::move(value_));
		}
	}

private:

	Detail::KeepedValueType<T, KeepMode> m_optionalValue;
};


/**
*/
template<typename T, Detail::KeepValueMode KeepMode>
class NativeArray final : public RuntimeArray
{
	COMCLASS_(RuntimeArray)

	using ContainerType = std::remove_cv_t<T>;

public:

	static constexpr bool IsMutable = !std::is_const_v<T>;

	NativeArray(T& array_): m_array(array_)
	{
	}

	bool isMutable() const override
	{
		return IsMutable;
	}

	size_t size() const override
	{
		return ArrayValueOperations<ContainerType>::size(m_array);
	}

	RuntimeValue::Ptr element(size_t index) const override {
		Assert(index < this->size());

		return runtimeValueRef(m_array[index]);
	}

	void clear() override {
		if constexpr (!IsMutable)
		{
			Halt("Attempt to modify non mutable array value");
		}
		else
		{
			ArrayValueOperations<ContainerType>::clear(m_array, boost::none);
		}
	}

	void reserve(size_t) override
	{

	}

	Result<> push(RuntimeValue::Ptr value) override {
		if constexpr (!IsMutable)
		{
			Halt("Attempt to modify non mutable array");
		}
		else
		{
			decltype(auto) newElement = ArrayValueOperations<ContainerType>::emplaceBack(m_array);
			return RuntimeValue::assign(runtimeValueRef(newElement), std::move(value));
		}

		return success;
	}

private:

	Detail::KeepedValueType<T, KeepMode> m_array;
};


//template<Meta::TupleRepresentable T>
template<typename T, Detail::KeepValueMode KeepMode>
class NativeTuple final : public RuntimeTuple
{
	using TupleType = std::decay_t<T>;

	COMCLASS_(RuntimeTuple)

public:
	static constexpr bool IsMutableValue = !std::is_const_v<T>;
	static constexpr size_t Size = TupleValueOperations<TupleType>::TupleSize;

	template<typename U,
		std::enable_if_t<std::is_same_v<U,T> && KeepMode == Detail::KeepValueMode::ByReference, int> = 0>
	NativeTuple(U& value): m_value(value)
	{}

	template<typename U,
		std::enable_if_t<std::is_same_v<U,T> && KeepMode == Detail::KeepValueMode::ByValue, int> = 0>
	NativeTuple(U&& value): m_value(std::forward<U>(value))
	{}


	bool isMutable() const override {
		return IsMutableValue;
	}

	size_t size() const override {
		return Size;
	}

	RuntimeValue::Ptr element(size_t index) const override {
		Assert(index < Size);
		return GetElementInternal(index, std::make_index_sequence<Size>{});
	}

private:

	template<size_t ... I>
	RuntimeValue::Ptr GetElementInternal(size_t index, std::index_sequence<I...>) const {
		Assert(index < Size);

		using F = RuntimeValue::Ptr(*) (T&);

		auto factories = std::array<F, Size> { &CreateElementValue<I> ... };

		const F f = factories[index];
		return f(m_value);
	}

	template<size_t I>
	static RuntimeValue::Ptr CreateElementValue(T& container) {
		decltype(auto) el = TupleValueOperations<TupleType>::element<I>(container);
		return runtimeValueRef(el);
	}

	Detail::KeepedValueType<T, KeepMode> m_value;
};


// template<Meta::DictionaryRepresentable T>
template<typename T, Detail::KeepValueMode KeepMode>
class NativeDictionary final : public RuntimeDictionary
{
	COMCLASS_(RuntimeDictionary)

public:
	static constexpr bool IsMutable = !std::is_const_v<T>;
	using Key = typename DictionaryValueOperations<T>::Key;
	using Value= typename DictionaryValueOperations<T>::Value;

	NativeDictionary(T& container): m_container(container)
	{}

	bool isMutable() const override {
		return IsMutable;
	}

	size_t size() const override
	{
		return DictionaryValueOperations<T>::size(m_container);
	}

	std::string_view key(size_t index) const override
	{
		decltype(auto) k = DictionaryValueOperations<T>::key(m_container, index);

		return k;
	}

	RuntimeValue::Ptr value(std::string_view key) const override
	{
		std::conditional_t<IsMutable, Value, const Value>* value = nullptr;

		if (!DictionaryValueOperations<T>::find(m_container, Key{key}, &value))
		{
			return nothing;
		}

		return runtimeValueRef(*value);
	}

	bool hasKey(std::string_view key) const override
	{
		return DictionaryValueOperations<T>::find(m_container, Key{key});
	}

	void clear() override
	{
		if constexpr (!IsMutable) {
			Halt("Attempt to modify non mutable dictionary value");
		}
		else {
			DictionaryValueOperations<T>::clear(m_container);
		}
	}

	Result<> set(std::string_view key, RuntimeValue::Ptr value) override
	{
		if constexpr (!IsMutable) {
			Halt("Attempt to modify non mutable dictionary value");
		}
		else
		{
			Value* myValue = nullptr;
			if (!DictionaryValueOperations<T>::find(m_container, Key{key}, &myValue))
			{
				myValue = &DictionaryValueOperations<T>::emplace(m_container, Key{key});
			}

			Assert(myValue);

			return RuntimeValue::assign(runtimeValueRef(*myValue), value);
		}

		return success;
	}

	RuntimeValue::Ptr erase(std::string_view) override
	{
		if constexpr (!IsMutable) {
			Halt("Attempt to modify non mutable dictionary value");
		}
		else
		{
			
		}

		return nothing;
	}

private:

	Detail::KeepedValueType<T, KeepMode> m_container;
};


// template<Meta::ObjectRepresentable T>
template<typename T, Detail::KeepValueMode KeepMode>
class NativeObject : public RuntimeObject
{
	using ThisType = NativeObject<T, KeepMode>;

	COMCLASS_(RuntimeObject)

public:
	static constexpr bool IsMutable = !std::is_const_v<T>;

	NativeObject(T& value): m_value(value), m_fields(makeFields(Meta::classFields(this->m_value)))
	{}

	bool isMutable() const override {
		return IsMutable;
	}

	size_t size() const override {
		return ThisType::FieldsCount;
	}

	std::string_view key(size_t index) const override {
		return m_fields[index].name();
	}

	RuntimeValue::Ptr value(std::string_view key_) const override {
		auto iter = std::find_if(m_fields.begin(), m_fields.end(), [key_](const Field& field) { return Strings::icaseEqual(field.name(), key_); });

		return iter != m_fields.end() ? iter->value() : nothing;
	}

	bool hasKey(std::string_view key_) const override {
		auto iter = std::find_if(m_fields.begin(), m_fields.end(), [key_](const Field& field) { return Strings::icaseEqual(field.name(), key_); });

		return iter != m_fields.end();
	}

	Result<> set(std::string_view, RuntimeValue::Ptr value) override {
		return success;
	}

	boost::optional<FieldInfo> fieldInfo(std::string_view) const override {
		return boost::none;
	}

private:
	using FieldValueFactory = RuntimeValue::Ptr (*) (void*) noexcept;

	class Field
	{
	public:
		Field(std::string_view name_, void* value_, FieldValueFactory factory): m_name(name_), m_fieldValue(value_), m_factory(factory)
		{}

		std::string_view name() const
		{
			return m_name;
		}

		const RuntimeValue::Ptr& value() const
		{
			if (!m_rtValue)
			{
				m_rtValue = m_factory(m_fieldValue);
			}

			return m_rtValue;
		}

	private:
		const std::string_view m_name;
		void* const m_fieldValue;
		const FieldValueFactory m_factory;
		mutable RuntimeValue::Ptr m_rtValue;
	};

	template<typename FieldValueType, typename ... Attribs>
	static Field makeField(Meta::FieldInfo<FieldValueType, Attribs...> field)
	{
		const FieldValueFactory factory = [](void* ptr) noexcept -> RuntimeValue::Ptr
		{
			FieldValueType* const value = reinterpret_cast<FieldValueType*>(ptr);
			return runtimeValueRef(*value);
		};

		FieldValueType& fieldValue = field.value();

		return Field(field.name(), reinterpret_cast<void*>(const_cast<std::remove_const_t<FieldValueType>*>(&fieldValue)), factory);
	}


	template<typename ... FieldInfo, size_t ... I>
	static auto makeFields(std::tuple<FieldInfo...>&& fields, std::index_sequence<I...>) {
		return std::array {makeField(std::get<I>(fields)) ...};
	}

	template<typename ... FieldInfo>
	static auto makeFields(std::tuple<FieldInfo...>&& fields) {
		return makeFields(std::move(fields), Tuple::indexes(fields));
	}

	using Fields = decltype(ThisType::makeFields(Meta::classFields<T>()));

	static constexpr size_t FieldsCount = std::tuple_size_v<Fields>;

	Detail::KeepedValueType<T, KeepMode> m_value;
	Fields m_fields;
};

/* -------------------------------------------------------------------------- */
// Integer
template<typename T,
	std::enable_if_t<std::is_integral_v<T>, int>>
RuntimeIntegerValue::Ptr runtimeValueRef(T& value)// requires (std::is_integral_v<T>)
{
	return Com::createInstance<NativeIntegerValue<T, Detail::KeepValueMode::ByReference>, RuntimeIntegerValue>(value);
}

template<typename T,
	std::enable_if_t<std::is_integral_v<T>, int>>
RuntimeIntegerValue::Ptr runtimeValueRef(const T& value) // requires (std::is_integral_v<T>)
{
	return Com::createInstance<NativeIntegerValue<const T, Detail::KeepValueMode::ByReference>, RuntimeIntegerValue>(value);
}

template<typename T,
	std::enable_if_t<std::is_integral_v<T>, int> = 0>
RuntimeIntegerValue::Ptr runtimeValueCopy(T value) {
	return Com::createInstance<NativeIntegerValue<T, Detail::KeepValueMode::ByValue>, RuntimeIntegerValue>(value);
}

/* -------------------------------------------------------------------------- */
// Boolean
inline RuntimeBooleanValue::Ptr runtimeValueRef(bool& value) {
	return Com::createInstance<NativeBooleanValue<bool, Detail::KeepValueMode::ByReference>, RuntimeBooleanValue>(value);
}

inline RuntimeBooleanValue::Ptr runtimeValueRef(const bool& value) {
	return Com::createInstance<NativeBooleanValue<const bool, Detail::KeepValueMode::ByReference>, RuntimeBooleanValue>(value);
}

inline RuntimeBooleanValue::Ptr runtimeValueCopy(bool value) {
	return Com::createInstance<NativeBooleanValue<bool, Detail::KeepValueMode::ByValue>, RuntimeBooleanValue>(value);
}

/* -------------------------------------------------------------------------- */
// Floating point
template<typename T,
	std::enable_if_t<std::is_floating_point_v<T>, int>>
ComPtr<RuntimeFloatValue> runtimeValueRef(T& value) // requires (std::is_floating_point_v<T>)
{
	return Com::createInstance<NativeFloatValue<T, Detail::KeepValueMode::ByReference>, RuntimeFloatValue>(value);
}

template<typename T,
	std::enable_if_t<std::is_floating_point_v<T>, int>>
ComPtr<RuntimeFloatValue> runtimeValueRef(const T& value)// requires (std::is_floating_point_v<T>)
{
	return Com::createInstance<NativeFloatValue<const T, Detail::KeepValueMode::ByReference>, RuntimeFloatValue>(value);
}


template<typename T,
	std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
ComPtr<RuntimeFloatValue> runtimeValueCopy(T value) {
	return Com::createInstance<NativeFloatValue<T, Detail::KeepValueMode::ByValue>, RuntimeFloatValue>(value);
}

/* -------------------------------------------------------------------------- */
// String
#if 0
template<typename T,
	std::enable_if_t<std::is_convertible_v<T, std::string>, int>>
ComPtr<RuntimeStringValue> runtimeValueRef(T& value) {
	static_assert(Detail::CanKeepStringByRef<T>, "Unknown string representation. std::string can be keeped by reference");

	return Com::createInstance<NativeStringValue<T, Detail::KeepValueMode::ByReference>, RuntimeStringValue>(value);
}

template<typename T,
	std::enable_if_t<std::is_constructible_v<std::string_view, T>, int>>
ComPtr<RuntimeStringValue> runtimeValueRef(const T& value) {
	const std::string_view str (value);
	return Com::createInstance<NativeStringValue<std::string_view, Detail::KeepValueMode::ByReference>, RuntimeStringValue>(str);
}
#endif

template<typename T,
	std::enable_if_t<std::is_constructible_v<std::string_view, T>, int>>
ComPtr<RuntimeStringValue> runtimeValueRef(T&) {
	//static_assert(false, 
	return nothing;
}

template<typename T,
	std::enable_if_t<std::is_constructible_v<std::string_view, T>, int>>
ComPtr<RuntimeStringValue> runtimeValueRef(const T& value) {
	const std::string_view str (value);
	return Com::createInstance<NativeStringValue<std::string_view, false, Detail::KeepValueMode::ByReference>, RuntimeStringValue>(str);
}

template<typename ... Traits>
ComPtr<RuntimeStringValue> runtimeValueRef(const std::basic_string<char, Traits...>& str) {
	using NativeStringValueType = NativeStringValue<std::basic_string<char, Traits...>, false, Detail::KeepValueMode::ByReference>;
	return Com::createInstance<NativeStringValueType, RuntimeStringValue>(str);
}

template<typename ... Traits>
ComPtr<RuntimeStringValue> runtimeValueRef(std::basic_string<char, Traits...>& str) {
	using NativeStringValueType = NativeStringValue<std::basic_string<char, Traits...>, true, Detail::KeepValueMode::ByReference>;
	return Com::createInstance<NativeStringValueType, RuntimeStringValue>(str);
}

inline ComPtr<RuntimeStringValue> runtimeValueCopy(std::string_view str) {
	using NativeStringValueType = NativeStringValue<std::string, true, Detail::KeepValueMode::ByValue>;
	return Com::createInstance<NativeStringValueType, RuntimeStringValue>(str);
}

/* -------------------------------------------------------------------------- */
// Optional
template<typename T,
	std::enable_if_t<IsOptionalRepresentable<T>, int>>
ComPtr<RuntimeOptionalValue> runtimeValueRef(T& optionalValue) {
	return Com::createInstance<NativeOptionalValue<T, Detail::KeepValueMode::ByReference>, RuntimeOptionalValue>(optionalValue);
}

template<typename T,
	std::enable_if_t<IsOptionalRepresentable<T>, int>>
ComPtr<RuntimeOptionalValue> runtimeValueRef(const T& value)
{
	return Com::createInstance<NativeOptionalValue<const T, Detail::KeepValueMode::ByReference>, RuntimeOptionalValue>(value);
}

/* -------------------------------------------------------------------------- */
// Array
template<typename T,
	std::enable_if_t<IsArrayRepresentable<T>, int>>
ComPtr<RuntimeArray> runtimeValueRef(T& container)
{
	return Com::createInstance<NativeArray<T, Detail::KeepValueMode::ByReference>, RuntimeArray>(container);
}

template<typename T,
	std::enable_if_t<IsArrayRepresentable<T>, int>>
ComPtr<RuntimeArray> runtimeValueRef(const T& container)
{
	return Com::createInstance<NativeArray<const T, Detail::KeepValueMode::ByReference>, RuntimeArray>(container);
}

/* -------------------------------------------------------------------------- */
// Tuple
template<typename T,
	std::enable_if_t<IsTupleRepresentable<T>, int>>
ComPtr<RuntimeTuple> runtimeValueRef(T& container) {
	return Com::createInstance<NativeTuple<T, Detail::KeepValueMode::ByReference>, RuntimeTuple>(container);
}

template<typename T,
	std::enable_if_t<IsTupleRepresentable<T>, int>>
ComPtr<RuntimeTuple> runtimeValueRef(const T& container) {
	return Com::createInstance<NativeTuple<const T, Detail::KeepValueMode::ByReference>, RuntimeTuple>(container);
}

template<typename T,
	std::enable_if_t<IsTupleRepresentable<T>, int> = 0>
ComPtr<RuntimeTuple> runtimeValueCopy(T&& container) {
	return Com::createInstance<NativeTuple<T, Detail::KeepValueMode::ByValue>, RuntimeTuple>(std::forward<T>(container));
}

/* -------------------------------------------------------------------------- */
// Dictionary
template<typename T,
	std::enable_if_t<IsDictionaryRepresentable<T>, int>>
ComPtr<RuntimeDictionary> runtimeValueRef(T& container) {
	return Com::createInstance<NativeDictionary<T, Detail::KeepValueMode::ByReference>, RuntimeDictionary>(container);
}

template<typename T,
	std::enable_if_t<IsDictionaryRepresentable<T>, int>>
ComPtr<RuntimeDictionary> runtimeValueRef(const T& container) {
	return Com::createInstance<NativeDictionary<const T, Detail::KeepValueMode::ByReference>, RuntimeDictionary>(container);
}

template<typename T,
	std::enable_if_t<IsDictionaryRepresentable<T>, int> = 0>
ComPtr<RuntimeDictionary> runtimeValueCopy(T&& value) {
	return Com::createInstance<NativeDictionary<T, Detail::KeepValueMode::ByValue>, RuntimeDictionary>(std::forward<T>(value));
}

/* -------------------------------------------------------------------------- */
// Object
template<typename T,
	std::enable_if_t<IsObjectRepresentable<T>, int>>
ComPtr<RuntimeObject> runtimeValueRef(T& value) {
	return Com::createInstance<NativeObject<T, Detail::KeepValueMode::ByReference>, RuntimeObject>(value);
}

template<typename T,
	std::enable_if_t<IsObjectRepresentable<T>, int>>
ComPtr<RuntimeObject> runtimeValueRef(const T& value) {
	return Com::createInstance<NativeObject<const T, Detail::KeepValueMode::ByReference>, RuntimeObject>(value);
}

template<typename T,
	std::enable_if_t<IsObjectRepresentable<T>, int> = 0>
ComPtr<RuntimeObject> runtimeValueCopy(T&& value) {
	return Com::createInstance<NativeObject<T, Detail::KeepValueMode::ByValue>, RuntimeObject>(std::forward<T>(value));
}

/* ---------------------------------------------------------------------------*/
/**
* 
*/
template<typename T,
	std::enable_if_t<std::is_base_of_v<RuntimeValue, T>, int>>
RuntimeValueRef::Ptr runtimeValueRef(const ComPtr<T>& value) {
	return RuntimeValueRef::Create(value);
}

template<typename T,
	std::enable_if_t<std::is_base_of_v<RuntimeValue, T>, int>>
RuntimeValueRef::Ptr runtimeValueRef(ComPtr<T>& value) {
	return RuntimeValueRef::Create(value);
}

/* ---------------------------------------------------------------------------*/
template<typename T>
[[nodiscard]] T RuntimeValueCast(RuntimeValue::Ptr rtValue) {
	T value{};
	RuntimeValue::assign(runtimeValueRef(value), std::move(rtValue)).rethrowIfException();
	return value;
}


} // namespace Runtime
