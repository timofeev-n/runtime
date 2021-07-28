//◦ Playrix ◦
#include "pch.h"
#include "runtime/serialization/runtimevalue.h"
#include "runtime/diagnostics/exception.h"
#include "runtime/com/comclass.h"

//#include "runtime/utils/stringconv.h"
// #include "runtime/com/comclass.h"


namespace Runtime {

namespace {
/*
template<typename U, typename T>
decltype(auto) cast_(T& src_) 
{
	using Target = std::conditional_t<std::is_const_v<T>, std::add_const_t<U>, U>;
	return src_.as<Target&>();
}


template<typename RT, typename F>
Result<> visitNumericPrimitiveValue(RT& value, F f) 
{
	if (value->is<RuntimeInt8Value>())
	{
		return f(cast_<RuntimeInt8Value>(*value));
	}
	else if (value->is<RuntimeInt16Value>())
	{
		return f(cast_<RuntimeInt16Value>(*value));
	}
	else if (value->is<RuntimeInt32Value>())
	{
		return f(cast_<RuntimeInt32Value>(*value));
	}
	else if (value->is<RuntimeInt64Value>())
	{
		return f(cast_<RuntimeInt64Value>(*value));
	}
	else if (value->is<RuntimeUInt8Value>())
	{
		return f(cast_<RuntimeUInt8Value>(*value));
	}
	else if (value->is<RuntimeUInt16Value>())
	{
		return f(cast_<RuntimeUInt16Value>(*value));
	}
	else if (value->is<RuntimeUInt32Value>())
	{
		return f(cast_<RuntimeUInt32Value>(*value));
	}
	else if (value->is<RuntimeUInt64Value>())
	{
		return f(cast_<RuntimeUInt64Value>(*value));
	}
	else if (value->is<RuntimeSingleValue>())
	{
		return f(cast_<RuntimeSingleValue>(*value));
	}
	else if (value->is<RuntimeDoubleValue>())
	{
		return f(cast_<RuntimeDoubleValue>(*value));
	}
	
	RUNTIME_FAILURE("Unexpected code path")
	return Excpt_("Unexpected code path");
}
*/

class RuntimeValueRefImpl final : public RuntimeValueRef
{
	COMCLASS_(RuntimeValueRef);

public:

	RuntimeValueRefImpl(RuntimeValue::Ptr& valueRef): _valueRef(valueRef), _isMutable(true)
	{}

	RuntimeValueRefImpl(const RuntimeValue::Ptr& valueRef): _valueRef(const_cast<RuntimeValue::Ptr&>(valueRef)), _isMutable(false)
	{}

	bool isMutable() const {
		return _isMutable;
	}

	void set(RuntimeValue::Ptr value) override {
		_valueRef = std::move(value);
	}

	RuntimeValue::Ptr get() const override {
		return _valueRef;
	}

private:
	RuntimeValue::Ptr& _valueRef;
	const bool _isMutable;
};

/* -------------------------------------------------------------------------- */

Result<> assignStringValue(RuntimeStringValue& dstStr, const RuntimePrimitiveValue& src) {

	if (const RuntimeStringValue* const srcStr = src.as<const RuntimeStringValue*>()) {
		auto strBytes = srcStr->getUtf8();
		dstStr.setUtf8(strBytes);
		return success;
	}

	return Excpt_("String value can be assigned only from other string");
}

Result<> assignPrimitiveValue(RuntimePrimitiveValue& dst, const RuntimePrimitiveValue& src) {
	
	if (RuntimeStringValue* const dstStr = dst.as<RuntimeStringValue*>()) {
		return assignStringValue(*dstStr, src);
	}

	if (RuntimeBooleanValue* const dstBool = dst.as<RuntimeBooleanValue*>(); dstBool)
	{
		if (auto srcBool = src.as<const RuntimeBooleanValue*>(); srcBool)
		{
			dstBool->set(srcBool->get());
			return success;
		}

		return Excpt_("Expected Boolean");
	}

	if (src.is<RuntimeBooleanValue>())
	{
		return Excpt_("Boolean can be assigned only to boolean");
	}

	if (RuntimeIntegerValue* const dstInt = dst.as<RuntimeIntegerValue*>(); dstInt)
	{
		if (const RuntimeIntegerValue* const srcInt = src.as<const RuntimeIntegerValue*>(); srcInt)
		{
			if (dstInt->isSigned())
			{
				dstInt->setInt64(srcInt->getInt64());
			}
			else
			{
				dstInt->setUint64(srcInt->getUint64());
			}
		}
		else if (const RuntimeFloatValue* const srcFloat = src.as<const RuntimeFloatValue*>(); srcFloat)
		{
			const auto iValue = static_cast<int64_t>(std::floor(srcFloat->getSingle()));

			if (dstInt->isSigned())
			{
				dstInt->setInt64(iValue);
			}
			else
			{
				dstInt->setUint64(iValue);
			}
		}
		else
		{
			return Excpt_("Can t assgin value to integer");
		}

		return success;
	}

	if (RuntimeFloatValue* const dstFloat = dst.as<RuntimeFloatValue*>(); dstFloat)
	{
		if (const RuntimeFloatValue* const srcFloat = src.as<const RuntimeFloatValue*>(); srcFloat)
		{
			dstFloat->setDouble(srcFloat->getDouble());
		}
		else if (const RuntimeIntegerValue* const srcInt = src.as<const RuntimeIntegerValue*>(); srcInt)
		{
			if (dstFloat->bits() == sizeof(double))
			{
				dstFloat->setDouble(static_cast<double>(srcInt->getInt64()));
			}
			else
			{
				dstFloat->setSingle(static_cast<float>(srcInt->getInt64()));
			}
		}
		else
		{
			return Excpt_("Can t assgin value to float");
		}

		return success;
	}


	return Excpt_("Do not known how to assing primitive runtime value");

	//const auto isString = [](const RuntimePrimitiveValue& val) 
	//{
	//	return val.is<RuntimeStringValue>() || val.is<RuntimeWStringValue>();
	//};
	/*
	if (isString(*dst) || isString(*srcPtr))
	{
		if (!isString(*dst) || !isString(*srcPtr))
		{
			return Excpt_("String is assignable only to string");
		}

		if (dst->is<RuntimeStringValue>()) 
		{
			RuntimeStringValue& dstStr = dst->as<RuntimeStringValue&>();

			if (srcPtr->is<RuntimeStringValue>())
			{
				const RuntimeStringValue& srcStr = srcPtr->as<const RuntimeStringValue&>();
				return dstStr.set(srcStr.get());
			}
			
			const RuntimeWStringValue& srcWStr = srcPtr->as<const RuntimeWStringValue&>();
			std::string bytes = strings::toUtf8(srcWStr.get());
			return dstStr.set(std::move(bytes));
		}

		RuntimeWStringValue& dstWStr = dst->as<RuntimeWStringValue&>();
		if (srcRtv.is<RuntimeStringValue>()) {
			const RuntimeStringValue& srcStr = srcPtr->as<const RuntimeStringValue&>();
			std::wstring wstr = strings::wstringFromUtf8(srcStr.get());
			return dstWStr.set(std::move(wstr));
		}

		const RuntimeWStringValue& srcWStr = srcPtr->as<const RuntimeWStringValue&>();
		return dstWStr.set(srcWStr.get());
	}*/

	//return visitNumericPrimitiveValue(dst, [&]<typename T>(TypedRuntimePrimitiveValue<T>& dstRtValue) -> Result<> {
	//	return visitNumericPrimitiveValue(srcPtr, [&]<typename U>(const TypedRuntimePrimitiveValue<U>& srcRtValue) -> Result<>{
	//		const auto srcValue = *srcRtValue;
	//		return dstRtValue.set(srcValue);
	//	});
	//});
	return success;
}

Result<> assignArray(RuntimeArray& arr, const RuntimeReadonlyCollection& collection) 
{
	const size_t size = collection.size();
	arr.clear();
	if (size == 0)
	{
		return success;
	}

	arr.reserve(size);

	for (size_t i = 0; i < size; ++i)
	{
		if (auto pushResult = arr.push(collection[i]); !pushResult)
		{
			return pushResult;
		}
	}
	
	return success;
}


Result<> assignDictionary(RuntimeDictionary& dict, const RuntimeReadonlyCollection& src)
{
	return Excpt_("assignDictionary not implemented");
}


Result<> assignObject(RuntimeObject& obj, const RuntimeReadonlyDictionary& dict)
{
	for (size_t i = 0, size = obj.size(); i < size; ++i)
	{
		const auto [key, fieldValue] = obj[i];

		if (const RuntimeValue::Ptr value = dict.value(key); value)
		{
			if (auto assignResult = RuntimeValue::assign(fieldValue, value); !assignResult)
			{
				return assignResult;
			}
		}
		else
		{

		}
	}

	return success;
}

} // namespace


Result<> RuntimeValue::assign(RuntimeValue::Ptr dst, RuntimeValue::Ptr src)
{
	Assert(dst);
	Assert(dst->isMutable());

	if (RuntimeValueRef* const valueRef = dst->as<RuntimeValueRef*>()) {
		valueRef->set(std::move(src));
		return success;
	}

	Assert(src);

	if (const RuntimeOptionalValue* const srcOpt = src->as<const RuntimeOptionalValue*>(); srcOpt) {

		if (srcOpt->hasValue()) {
			return RuntimeValue::assign(dst, srcOpt->value());
		}

		if (RuntimeOptionalValue* const dstOpt = dst->as<RuntimeOptionalValue*>()) {
			dstOpt->reset();
			return success;
		}
		
		return Excpt_("Attempt to assgin non optional value from null (optional)");
	}

	if (RuntimeOptionalValue* const dstOpt = dst->as<RuntimeOptionalValue*>()) {
		return dstOpt->setValue(std::move(src));
	}

	if (RuntimePrimitiveValue* const dstValue = dst->as<RuntimePrimitiveValue*>(); dstValue)
	{
		auto srcValue = src->as<const RuntimePrimitiveValue*>();
		return assignPrimitiveValue(*dstValue, *srcValue);
	}
	
	if (RuntimeArray* const arr = dst->as<RuntimeArray*>(); arr)
	{
		const RuntimeReadonlyCollection* const collection = src->as<const RuntimeReadonlyCollection*>();
		if (!collection)
		{
			return Excpt_("Collection required");
		}

		return assignArray(*arr, *collection);
	}

	if (RuntimeDictionary* const dict = dst->as<RuntimeDictionary*>(); dict)
	{
		
	}

	if (RuntimeObject* const obj = dst->as<RuntimeObject*>(); obj)
	{
		const RuntimeReadonlyDictionary* const dict = src->as<const RuntimeReadonlyDictionary*>();
		if (!dict)
		{
			return Excpt_("Collection required");
		}

		return assignObject(*obj, *dict);
	}

	return Excpt_("Do not known how to assign runtime value");
}

/* -------------------------------------------------------------------------- */
RuntimeValueRef::Ptr RuntimeValueRef::Create(RuntimeValue::Ptr& value) {
	return Com::createInstance<RuntimeValueRefImpl, RuntimeValueRef>(std::ref(value));
}

RuntimeValueRef::Ptr RuntimeValueRef::Create(const RuntimeValue::Ptr& value) {
	return Com::createInstance<RuntimeValueRefImpl, RuntimeValueRef>(std::cref(value));
}

}
