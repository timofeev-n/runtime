//◦ Playrix ◦
#pragma once
#include <runtime/com/ianything.h>
#include <runtime/com/interface.h>
#include <runtime/meta/classinfo.h>
#include <runtime/utils/result.h>

#include <runtime/com/comptr.h>

#include <boost/optional.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace Runtime {

/**

*/
struct ABSTRACT_TYPE RuntimeValue : virtual IRefCounted
{
	using Ptr = ComPtr<RuntimeValue>;

	static Result<> assign(RuntimeValue::Ptr dst, RuntimeValue::Ptr src);

	virtual ~RuntimeValue() = default;

	virtual bool isMutable() const = 0;
};

/**

*/
struct ABSTRACT_TYPE RuntimeValueRef : virtual RuntimeValue
{
	DECLARE_CLASS_BASE(RuntimeValue)

	using Ptr = ComPtr<RuntimeValueRef>;

	static RuntimeValueRef::Ptr Create(RuntimeValue::Ptr&);

	static RuntimeValueRef::Ptr Create(const RuntimeValue::Ptr&);

	virtual void set(RuntimeValue::Ptr) = 0;

	virtual RuntimeValue::Ptr get() const = 0;
};


/**
*/
struct ABSTRACT_TYPE RuntimePrimitiveValue : virtual RuntimeValue
{
	DECLARE_CLASS_BASE(RuntimeValue)

	using Ptr = ComPtr<RuntimePrimitiveValue>;
};


struct ABSTRACT_TYPE RuntimeStringValue : virtual RuntimePrimitiveValue
{
	DECLARE_CLASS_BASE(RuntimePrimitiveValue)

	using Ptr = ComPtr<RuntimeStringValue>;


	virtual void setUtf8(std::string_view) = 0;

	virtual void set(std::wstring) = 0;

	virtual std::wstring get() const = 0;

	virtual std::string getUtf8() const = 0;
};



struct ABSTRACT_TYPE RuntimeIntegerValue : virtual RuntimePrimitiveValue
{
	DECLARE_CLASS_BASE(RuntimePrimitiveValue)

	using Ptr = ComPtr<RuntimeIntegerValue>;


	virtual bool isSigned() const = 0;

	virtual size_t bits() const = 0;

	virtual void setInt64(int64_t) = 0;

	virtual void setUint64(uint64_t) = 0;

	virtual int64_t getInt64() const = 0;

	virtual uint64_t getUint64() const = 0;


	template<typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
	void set(T value) //requires(std::is_integral_v<T>)
	{
		// this->set(&value, sizeof(T));
	}

	template<typename T>
	T get() //requires(std::is_integral_v<T>)
	{
		//T value = 0;
		//this->get(&value, sizeof(T));

		return 0;
	}
};


struct ABSTRACT_TYPE RuntimeFloatValue : virtual RuntimePrimitiveValue
{
	DECLARE_CLASS_BASE(RuntimePrimitiveValue)

	using Ptr = ComPtr<RuntimeFloatValue>;


	virtual size_t bits() const = 0;

	virtual void setDouble(double) = 0;

	virtual void setSingle(float) = 0;

	virtual double getDouble() const = 0;
	
	virtual float getSingle() const = 0;

	template<typename T,
		std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>

	T get() const // requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
	{
		return static_cast<T>(this->getDouble());
	}
};


struct ABSTRACT_TYPE RuntimeBooleanValue : virtual RuntimePrimitiveValue
{
	DECLARE_CLASS_BASE(RuntimePrimitiveValue)

	using Ptr = ComPtr<RuntimeBooleanValue>;


	virtual void set(bool) = 0;

	virtual bool get() const = 0;
};


/**

*/
struct ABSTRACT_TYPE RuntimeOptionalValue : virtual RuntimeValue
{
	DECLARE_CLASS_BASE(RuntimeValue)

	using Ptr = ComPtr<RuntimeOptionalValue>;

	virtual bool hasValue() const = 0;

	virtual RuntimeValue::Ptr value() const = 0;

	virtual Result<> setValue(RuntimeValue::Ptr value = nothing) = 0;

	explicit inline operator bool () const {
		return this->hasValue();
	}

	inline void reset() {
		this->setValue(nothing).rethrowIfException();
	}
};

/**
*/
struct ABSTRACT_TYPE RuntimeReadonlyCollection : virtual RuntimeValue
{
	DECLARE_CLASS_BASE(RuntimeValue)

	using Ptr = ComPtr<RuntimeReadonlyCollection>;


	virtual size_t size() const = 0;

	virtual RuntimeValue::Ptr element(size_t index) const = 0;

	inline RuntimeValue::Ptr operator[](size_t index) const
	{
		return this->element(index);
	}
};


/**
*/
struct ABSTRACT_TYPE RuntimeArray : virtual RuntimeReadonlyCollection
{
	DECLARE_CLASS_BASE(RuntimeReadonlyCollection)

	using Ptr = ComPtr<RuntimeArray>;


	virtual void clear() = 0;

	virtual void reserve(size_t) = 0;

	virtual Result<> push(RuntimeValue::Ptr) = 0;
};


/**
*/
struct ABSTRACT_TYPE RuntimeTuple : virtual RuntimeReadonlyCollection
{
	DECLARE_CLASS_BASE(RuntimeReadonlyCollection)

	using Ptr = ComPtr<RuntimeTuple>;
};


/**
*/
struct ABSTRACT_TYPE RuntimeReadonlyDictionary : virtual RuntimeValue
{
	DECLARE_CLASS_BASE(RuntimeValue)
	
	using Ptr = ComPtr<RuntimeReadonlyDictionary>;


	virtual size_t size() const = 0;

	virtual std::string_view key(size_t index) const = 0;

	virtual RuntimeValue::Ptr value(std::string_view) const = 0;

	virtual bool hasKey(std::string_view) const = 0;


	inline RuntimeValue::Ptr operator[](std::string_view key) const
	{
		return this->value(key);
	}

	inline std::pair<std::string_view, RuntimeValue::Ptr> operator[](size_t index) const
	{
		auto key_ = this->key(index);
		return {key_, this->value(key_)};
	}
};


/**
* 
*/
struct ABSTRACT_TYPE RuntimeDictionary : virtual RuntimeReadonlyDictionary
{
	DECLARE_CLASS_BASE(RuntimeReadonlyDictionary)

	using Ptr = ComPtr<RuntimeDictionary>;


	virtual void clear() = 0;

	virtual Result<> set(std::string_view, RuntimeValue::Ptr value) = 0;

	virtual RuntimeValue::Ptr erase(std::string_view) = 0;
};


/**
	Generalized object runtime representation.
*/
struct ABSTRACT_TYPE RuntimeObject : virtual RuntimeReadonlyDictionary
{
	DECLARE_CLASS_BASE(RuntimeReadonlyDictionary)

	using Ptr = ComPtr<RuntimeObject>;


	struct FieldInfo
	{

	};

	virtual Result<> set(std::string_view, RuntimeValue::Ptr value) = 0;

	virtual boost::optional<FieldInfo> fieldInfo(std::string_view) const  = 0;
};


class ReadonlyDynamicObject
{
public:

	ReadonlyDynamicObject(RuntimeValue::Ptr value): _dict(std::move(value))
	{}

	explicit operator bool () const {
		return static_cast<bool>(_dict);
	}

	operator RuntimeValue::Ptr () const {
		return _dict;
	}

	bool Has(std::string_view key) const {
		return _dict && _dict->hasKey(key);
	}

	template<typename T = RuntimeValue::Ptr>
	T Get(std::string_view key) const;

	template<>
	RuntimeValue::Ptr Get<RuntimeValue::Ptr>(std::string_view key) const {
		Assert(_dict);
		auto value = _dict->value(key);
		Assert2(value, Core::Format::format("Named field ({}) does not exists", key));
		return value;
	
	}

	template<>
	std::string Get<std::string>(std::string_view key) const {
		const auto value = this->Get<RuntimeValue::Ptr>(key);
		auto strValue = value->as<const RuntimeStringValue*>();
		Assert2(strValue, Core::Format::format("Field ({}) is not a string", key));

		return strValue->getUtf8();
	}

private:

	

	RuntimeReadonlyDictionary::Ptr _dict;

};



}


