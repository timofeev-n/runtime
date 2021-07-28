//◦ Playrix ◦
#pragma once
#include <runtime/automation/dispatch.h>
#include <boost/optional.hpp>

namespace Runtime {

template<typename T>
class AutoDispatch : public Dispatch
{
	DECLARE_CLASS_BASE(Dispatch)

public:

	ComPtr<> Invoke(std::string_view contract, std::string_view method, Runtime::RuntimeReadonlyCollection::Ptr arguments) override final {

		auto methods = Meta::classMethods<T>();

		auto result = TryFindAndInvokeMethod(method, arguments, std::move(methods), Tuple::indexes(methods));
		if (!result) {

		}
		return *result;
	}

private:

	template<typename U>
	static U CastRuntimeArgument(const Runtime::RuntimeReadonlyCollection::Ptr& arguments, size_t index) {
		U value;
		if (arguments->size() < index) {

		}

		Runtime::RuntimeValue::assign(Runtime::runtimeValueRef(value), arguments->element(index)).rethrowIfException();

		return value;
	}

	template<typename ... Methods, size_t ... I>
	boost::optional<ComPtr<>> TryFindAndInvokeMethod(std::string_view methodName, const Runtime::RuntimeReadonlyCollection::Ptr& arguments, std::tuple<Methods ...> methods, std::index_sequence<I...>) {

		ComPtr<> result;

		const auto TryInvokeNamedMethod = [this, methodName, &arguments, &result](auto& methodInfo) -> bool{

			if (!Strings::icaseEqual(methodInfo.name(), methodName)) {
				return false;
			}

			using MethodInfo = std::remove_reference_t<decltype(methodInfo)>;
			using FuncTypeInfo = typename MethodInfo::FunctionTypeInfo;
			using Parameters = typename FuncTypeInfo::ParametersList;

			result = this->NativeCall(methodInfo, arguments, Parameters{}, std::make_index_sequence<Parameters::Size>{} );
			return true;
		};

		const bool success = (TryInvokeNamedMethod(std::get<I>(methods)) || ...);
		if (!success) {
			return boost::none;
		}

		return result;
	}

	template<auto F, typename ... Attribs, typename ... ArgType, size_t ... ArgIndex>
	ComPtr<> NativeCall(const Meta::MethodInfo<F, Attribs...>&, const Runtime::RuntimeReadonlyCollection::Ptr& arguments, TypeList<ArgType ...>, std::index_sequence<ArgIndex...>) {

		using FunctionTypeInfo = typename Meta::MethodInfo<F, Attribs...>::FunctionTypeInfo;
		static_assert(Meta::IsMemberFunction<decltype(F)>);

		if (auto targetInterface = this->as<typename FunctionTypeInfo::Class*>(); targetInterface) {
			auto result = (targetInterface->*F)(CastRuntimeArgument<ArgType>(arguments, ArgIndex) ... );
			return Runtime::runtimeValueCopy(std::move(result));
		}

		return nothing;
	}
};

}
