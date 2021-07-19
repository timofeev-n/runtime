#pragma once

#include <runtime/com/interface.h>
#include <runtime/meta/attribute.h>
#include <runtime/meta/classinfo.h>
#include <runtime/utils/preprocessor.h>
#include <runtime/diagnostics/sourceinfo.h>

#include <exception>
#include <string>
#include <type_traits>

//#pragma warning(push)
//#pragma warning(disable: 4250 4251 4275)

namespace Runtime {


struct ExceptionImplementationAttribute : Meta::Attribute
{};

template<typename T>
struct ExceptionImplementation : Meta::AttributeValue<ExceptionImplementationAttribute>
{
	using type = T;
};


class ABSTRACT_TYPE Exception : public virtual std::exception
{
	DECLARE_CLASS_BASE(std::exception)
public:

	virtual const char* message() const = 0;
	virtual const char* diagMessage() const = 0;
	virtual Diagnostics::SourceInfo source() const = 0;
	virtual unsigned flags() const = 0;

protected:
	virtual ~Exception() = default;
};

/**
*/
template<typename E>
class ExceptionImpl : public E
{
	static_assert(std::is_base_of_v<Exception, E>, "Type must inherit Runtime::Exception");
	DECLARE_CLASS_BASE(E)

public:
	ExceptionImpl(const Diagnostics::SourceInfo& sourceInfo_, std::string message_ , unsigned flags_ = 0)
		: m_sourceInfo(sourceInfo_)
		, m_message(std::move(message_))
		, m_flags(flags_)
	{}

	//ExceptionImpl(const Diagnostics::SourceInfo& sourceInfo_, std::string_view message_,  unsigned flags_ = 0)
	//	: ExceptionImpl(sourceInfo_, Runtime::utf8ToWString(message_), flags_)
	//{}

	const char* what() const override
	{
		if (!m_what)
		{
			m_what = m_message;
		}
		return m_what->c_str();
	}

	const char* message() const override
	{
		return m_message.c_str();
	}

	const char* diagMessage() const override
	{
		if (!m_sourceInfo)
		{
			return m_message.c_str();
		}

		if (!m_diagMessage)
		{
			const unsigned line = m_sourceInfo.line ? *m_sourceInfo.line : 0;
			m_diagMessage = Core::Format::format("{}({}):{}", m_sourceInfo.filePath, line, m_message);
		}

		return m_diagMessage->c_str();
	}

	Diagnostics::SourceInfo source() const  override {
		return m_sourceInfo;
	}

	unsigned flags() const  override {
		return m_flags;
	}

private:

	const Diagnostics::SourceInfo m_sourceInfo;
	const std::string m_message;
	const unsigned m_flags;
	mutable boost::optional<std::string> m_diagMessage;
	mutable boost::optional<std::string> m_what;
};


namespace internal {

class DefaultException : public ExceptionImpl<Exception>
{
	CLASS_INFO(
		CLASS_BASE(ExceptionImpl<Exception>)
	)
public:
	using ExceptionImpl<Exception>::ExceptionImpl;

};


template<typename T = Runtime::Exception, bool HasImplementation = Meta::AttributeDefined<T, ExceptionImplementationAttribute>>
struct ExceptionImplType__;

template<>
struct ExceptionImplType__<Runtime::Exception, false>
{
	using type = DefaultException;
};

template<typename T>
struct ExceptionImplType__<T, false>
{
	using type = T;
};

template<typename T>
struct ExceptionImplType__<T, true>
{
	using type = typename Meta::AttributeValueType<T, ExceptionImplementationAttribute>::type;
};

} // namespace internal


template<typename T>
using ExceptionImplType = typename internal::ExceptionImplType__<T>::type;


} // namespace Runtime


#define Excpt(ExceptionType, ...) ::Runtime::ExceptionImplType<ExceptionType>(INLINED_SOURCE_INFO, __VA_ARGS__)
#define Excpt_(message, ...) Excpt(::Runtime::Exception, ::Core::Format::format(message, __VA_ARGS__))

#define MAKE_Excpt(ExceptionType, ...) std::make_exception_ptr(Excpt(ExceptionType, __VA_ARGS__))
#define MAKE_Excpt_(message, ...) std::make_exception_ptr(Excpt_(message, __VA_ARGS__))

#define DECLARE_ABSTRACT_EXCEPTION(Base, ImplType) \
	CLASS_INFO(\
		CLASS_BASE(Base),\
		::Runtime::ExceptionImplementation<ImplType>{}\
	)\
