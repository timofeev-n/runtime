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


//-----------------------------------------------------------------------------
template<typename T,
	std::enable_if_t<std::is_integral_v<T>, int> = 0>
ComPtr<RuntimeIntegerValue> runtimeValue(T&); //requires (std::is_integral_v<T>);

template<typename T,
	std::enable_if_t<std::is_integral_v<T>, int> = 0>
ComPtr<RuntimeIntegerValue> runtimeValue(const T&); // requires (std::is_integral_v<T>);

template<typename T,
	std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
ComPtr<RuntimeFloatValue> runtimeValue(T&); //requires (std::is_floating_point_v<T>);

template<typename T,
	std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
ComPtr<RuntimeFloatValue> runtimeValue(const T&); // requires (std::is_floating_point_v<T>);

template<typename T,
	std::enable_if_t<std::is_convertible_v<T, std::string>, int> = 0>
ComPtr<RuntimeStringValue> runtimeValue(T&);

template<typename T,
	std::enable_if_t<std::is_convertible_v<T, std::string>, int> = 0>
ComPtr<RuntimeStringValue> runtimeValue(const T&);

template<typename T,
	std::enable_if_t<
		IsPrimitiveRepresentable<T> &&
		!(std::is_floating_point_v<T> || std::is_integral_v<T>)>, int = 0>
ComPtr<RuntimePrimitiveValue> runtimeValue(T&); 


template<typename T,
	std::enable_if_t<
		IsPrimitiveRepresentable<T> &&
		!(std::is_floating_point_v<T> || std::is_integral_v<T>)>, int = 0>
ComPtr<RuntimePrimitiveValue> runtimeValue(const T&); 


//template<Meta::OptionalRepresentable T>
template<typename T,
	std::enable_if_t<IsOptionalRepresentable<T>, int> = 0>
ComPtr<RuntimeOptionalValue> runtimeValue(T&);

//template<Meta::OptionalRepresentable T>
template<typename T,
	std::enable_if_t<IsOptionalRepresentable<T>, int> = 0>
ComPtr<RuntimeOptionalValue> runtimeValue(const T&);

//template<Meta::ArrayRepresentable T>
template<typename T,
	std::enable_if_t<IsArrayRepresentable<T>, int> = 0>
ComPtr<RuntimeArray> runtimeValue(T&);

//template<Meta::ArrayRepresentable T>
template<typename T,
	std::enable_if_t<IsArrayRepresentable<T>, int> = 0>
ComPtr<RuntimeArray> runtimeValue(const T&);

//template<Meta::TupleRepresentable T>
template<typename T,
	std::enable_if_t<IsTupleRepresentable<T>, int> = 0>
ComPtr<RuntimeTuple> runtimeValue(T&);

//template<Meta::TupleRepresentable  T>
template<typename T,
	std::enable_if_t<IsTupleRepresentable<T>, int> = 0>
ComPtr<RuntimeTuple> runtimeValue(const T&);

//template<Meta::DictionaryRepresentable T>
template<typename T,
	std::enable_if_t<IsDictionaryRepresentable<T>, int> = 0>
ComPtr<RuntimeDictionary> runtimeValue(T&);

//template<Meta::DictionaryRepresentable T>
template<typename T,
	std::enable_if_t<IsDictionaryRepresentable<T>, int> = 0>
ComPtr<RuntimeDictionary> runtimeValue(const T&);

//template<Meta::ObjectRepresentable T>
template<typename T,
	std::enable_if_t<IsObjectRepresentable<T>, int> = 0>
ComPtr<RuntimeObject> runtimeValue(T&);

//template<Meta::ObjectRepresentable T>
template<typename T,
	std::enable_if_t<IsObjectRepresentable<T>, int> = 0>
ComPtr<RuntimeObject> runtimeValue(const T&);


//-----------------------------------------------------------------------------
/**
* 
*/
template<typename T>
// requires (std::is_integral_v<T>)
class NativeIntegerValue final : public RuntimeIntegerValue
{
	COMCLASS_(RuntimeIntegerValue)

public:
	static constexpr bool IsMutable = !std::is_const_v<T>;
	using ValueType = std::remove_const_t<T>;

	NativeIntegerValue(T& value): m_value(value)
	{}

	bool isMutable() const override
	{
		return IsMutable;
	}

	bool isSigned() const override
	{
		return std::is_signed_v<T>;
	}

	size_t bits() const override
	{
		return sizeof(T);
	}

	void setInt64(int64_t value) override
	{
		m_value = static_cast<T>(value);
	}

	void setUint64(uint64_t value) override
	{
		m_value = static_cast<T>(value);
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
	T& m_value;
};


/**
* 
*/
template<typename T>
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
		if constexpr (!IsMutable)
		{
			Halt("Attempt to modify non mutable runtime value");
		}
		else
		{
			m_value = static_cast<T>(value);
		}
	}

	void setSingle([[maybe_unused]] float value) override
	{
		if constexpr (!IsMutable)
		{
			Halt("Attempt to modify non mutable runtime value");
		}
		else
		{
			m_value = static_cast<T>(value);
		}
	}

	double getDouble() const override
	{
		return static_cast<double>(m_value);
	}
	
	float getSingle() const override
	{
		return static_cast<float>(m_value);
	}

private:
	T& m_value;
};


template<typename T>
class NativeStringValue final : public RuntimeStringValue
{
	COMCLASS_(RuntimeStringValue)

public:

	static constexpr bool IsMutable = !std::is_const_v<T>;

	NativeStringValue(T& value): m_value(value)
	{}

	constexpr bool isMutable() const override {
		return IsMutable;
	}

	void setUtf8(std::string_view str) override {
		m_value = str;//Parse(str);
	}

	void set(std::wstring) override {
		Halt("set(wstring) is not implemented");
	}

	std::wstring get() const override {
		Halt("wstring get()");
		return {};
	}

	std::string getUtf8() const override {
		return ToString(m_value);
	}

private:

	T& m_value;
};





// template<Meta::OptionalRepresentable T>
template<typename T>
class NativeOptionalValue final : public RuntimeOptionalValue
{
	COMCLASS_(RuntimeOptionalValue)
public:

	static constexpr bool IsMutable = !std::is_const_v<T>;

	NativeOptionalValue(T& optionalValue) : m_optionalValue(optionalValue)
	{}

	bool isMutable() const override
	{
		return IsMutable;
	}

	bool hasValue() const override
	{
		return OptionalValueOperations<T>::hasValue(m_optionalValue);
	}

	RuntimeValue::Ptr value() const override
	{
		if (!this->hasValue())
		{
			return nothing;
		}

		decltype(auto) value = OptionalValueOperations<T>::value(m_optionalValue);
		return runtimeValue(value);
	}

	Result<> setValue(RuntimeValue::Ptr value_) override {

		if (!value_) {
			OptionalValueOperations<T>::reset(m_optionalValue);
			return success;
		}

		using OptionalOp = OptionalValueOperations<T>;
		
		decltype(auto) myValue = OptionalOp::hasValue(m_optionalValue) ? OptionalOp::value(m_optionalValue) : OptionalOp::emplace(m_optionalValue);

		return RuntimeValue::assign(runtimeValue(myValue), std::move(value_));
	}

private:

	T& m_optionalValue;
};


/**
*/
template<typename T>
class NativeArray final : public RuntimeArray
{
	COMCLASS_(RuntimeArray)

	using ContainerType = std::remove_const_t<T>;

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

		return runtimeValue(m_array[index]);
	}

	void clear() override {
		if constexpr (!IsMutable)
		{
			Halt("Attempt to modify non mutable value");
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
			return RuntimeValue::assign(runtimeValue(newElement), std::move(value));
		}

		return success;
	}

private:

	T& m_array;
};


//template<Meta::TupleRepresentable T>
template<typename T>
class NativeTuple final : public RuntimeTuple
{
	COMCLASS_(RuntimeTuple)

public:
	static constexpr bool IsMutable = !std::is_const_v<T>;
	static constexpr size_t Size = TupleValueOperations<T>::TupleSize;

	NativeTuple(T& value): m_value(value)
	{}

	bool isMutable() const override {
		return IsMutable;
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

		auto factories = std::array<F, Size> { &createElementValue<I> ... };

		const F f = factories[index];
		return f(m_value);
	}


	template<size_t I>
	static RuntimeValue::Ptr createElementValue(T& container)
	{
		decltype(auto) el = TupleValueOperations<T>::element<I>(container);
		return runtimeValue(el);
	}

	T& m_value;
};


// template<Meta::DictionaryRepresentable T>
template<typename T>
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

		return runtimeValue(*value);
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

			return RuntimeValue::assign(runtimeValue(*myValue), value);
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

	T& m_container;
};


// template<Meta::ObjectRepresentable T>
template<typename T>
class NativeObject : public RuntimeObject
{
	COMCLASS_(RuntimeObject)

public:
	static constexpr bool IsMutable = !std::is_const_v<T>;

	NativeObject(T& value): m_value(value), m_fields(makeFields(Meta::classFields(this->m_value)))
	{}

	bool isMutable() const override {
		return IsMutable;
	}

	size_t size() const override {
		return NativeObject<T>::FieldsCount;
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
			return runtimeValue(*value);
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


	using Fields = decltype(NativeObject<T>::makeFields(Meta::classFields<T>()));

	static constexpr size_t FieldsCount = std::tuple_size_v<Fields>;

	T& m_value;
	Fields m_fields;
};

//-----------------------------------------------------------------------------
// Integer
template<typename T,
	std::enable_if_t<std::is_integral_v<T>, int>>
ComPtr<RuntimeIntegerValue> runtimeValue(T& value)// requires (std::is_integral_v<T>)
{
	return Com::createInstance<NativeIntegerValue<T>, RuntimeIntegerValue>(value);
}

template<typename T,
	std::enable_if_t<std::is_integral_v<T>, int>>

ComPtr<RuntimeIntegerValue> runtimeValue(const T& value) // requires (std::is_integral_v<T>)
{
	return Com::createInstance<NativeIntegerValue<const T>, RuntimeIntegerValue>(value);
}

// Floating point
template<typename T,
	std::enable_if_t<std::is_floating_point_v<T>, int>>
ComPtr<RuntimeFloatValue> runtimeValue(T& value) // requires (std::is_floating_point_v<T>)
{
	return Com::createInstance<NativeFloatValue<T>, RuntimeFloatValue>(value);
}

template<typename T,
	std::enable_if_t<std::is_floating_point_v<T>, int>>
ComPtr<RuntimeFloatValue> runtimeValue(const T& value)// requires (std::is_floating_point_v<T>)
{
	return Com::createInstance<NativeFloatValue<const T>, RuntimeFloatValue>(value);
}

// String
template<typename T,
	std::enable_if_t<std::is_convertible_v<T, std::string>, int>>
ComPtr<RuntimeStringValue> runtimeValue(T& value) {
	return Com::createInstance<NativeStringValue<T>, RuntimeStringValue>(value);
}

template<typename T,
	std::enable_if_t<std::is_convertible_v<T, std::string>, int>>
ComPtr<RuntimeStringValue> runtimeValue(const T& value) {
	return Com::createInstance<NativeStringValue<const T>, RuntimeStringValue>(value);
}


// Optional
template<typename T,
	std::enable_if_t<IsOptionalRepresentable<T>, int>>
ComPtr<RuntimeOptionalValue> runtimeValue(T& optionalValue) {
	return Com::createInstance<NativeOptionalValue<T>, RuntimeOptionalValue>(optionalValue);
}

template<typename T,
	std::enable_if_t<IsOptionalRepresentable<T>, int>>
ComPtr<RuntimeOptionalValue> runtimeValue(const T& value)
{
	return Com::createInstance<NativeOptionalValue<const T>, RuntimeOptionalValue>(value);
}

// Array
template<typename T,
	std::enable_if_t<IsArrayRepresentable<T>, int>>
ComPtr<RuntimeArray> runtimeValue(T& container)
{
	return Com::createInstance<NativeArray<T>, RuntimeArray>(container);
}

template<typename T,
	std::enable_if_t<IsArrayRepresentable<T>, int>>
ComPtr<RuntimeArray> runtimeValue(const T& container)
{
	return Com::createInstance<NativeArray<const T>, RuntimeArray>(container);
}

// Tuple
template<typename T,
	std::enable_if_t<IsTupleRepresentable<T>, int>>
ComPtr<RuntimeTuple> runtimeValue(T& container)
{
	return Com::createInstance<NativeTuple<T>, RuntimeTuple>(container);
}

template<typename T,
	std::enable_if_t<IsTupleRepresentable<T>, int>>
ComPtr<RuntimeTuple> runtimeValue(const T& container)
{
	return Com::createInstance<NativeTuple<const T>, RuntimeTuple>(container);
}

// Dictionary
template<typename T,
	std::enable_if_t<IsDictionaryRepresentable<T>, int>>
ComPtr<RuntimeDictionary> runtimeValue(T& container)
{
	return Com::createInstance<NativeDictionary<T>, RuntimeDictionary>(container);
}

template<typename T,
	std::enable_if_t<IsDictionaryRepresentable<T>, int>>

ComPtr<RuntimeDictionary> runtimeValue(const T& container)
{
	return Com::createInstance<NativeDictionary<const T>, RuntimeDictionary>(container);
}

// Object
template<typename T,
	std::enable_if_t<IsObjectRepresentable<T>, int>>
ComPtr<RuntimeObject> runtimeValue(T& value)
{
	return Com::createInstance<NativeObject<T>, RuntimeObject>(value);
}

template<typename T,
	std::enable_if_t<IsObjectRepresentable<T>, int>>
ComPtr<RuntimeObject> runtimeValue(const T& value) {
	return Com::createInstance<NativeObject<const T>, RuntimeObject>(value);
}

} // namespace runtime
