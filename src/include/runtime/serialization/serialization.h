#pragma once

#include <runtime/diagnostics/sourceinfo.h>
#include <runtime/diagnostics/exception.h>
#include <runtime/meta/classinfo.h>
#include <runtime/com/ianything.h>
#include <runtime/io/writer.h>
#include <runtime/io/reader.h>
#include <runtime/serialization/serialization.h>
#include <runtime/utils/result.h>
#include <runtime/meta/attribute.h>
#include <runtime/serialization/runtimevalue.h>
#include <runtime/com/comptr.h>

#include <fmt/format.h>

namespace Runtime {
namespace SerializationDetail {

class SerializationExceptionImpl;
class RequiredFieldMissedExceptionImpl;
class InvalidTypeExceptionImpl;
class NumericOverflowExcepionImpl;
class EndOfStreamExceptionImpl;

} // namespace SerializationDetail

namespace Serialization {

/**
*/
//struct INTERFACE_API ISerialization : virtual IAnything
//{
//	/**
//	*/
//	virtual Result<> serialize(io::Writer& writer, const RuntimeValue::Ptr) const = 0;
//
//	/**
//	*/
//	virtual Result<RuntimeValue::Ptr> deserialize(io::Reader&) const = 0;
//
//	/**
//	*/
//	virtual Result<> deserializeInplace(io::Reader&, RuntimeValue::Ptr target) const = 0;
//};

/**
*/
class ABSTRACT_TYPE SerializationException : public Exception
{
	DECLARE_ABSTRACT_EXCEPTION(Exception, Runtime::SerializationDetail::SerializationExceptionImpl)
};

/**
*/
class ABSTRACT_TYPE RequiredFieldMissedException : public SerializationException
{
	DECLARE_ABSTRACT_EXCEPTION(SerializationException, Runtime::SerializationDetail::RequiredFieldMissedExceptionImpl)

public:
	virtual std::string fieldName() const = 0;
	virtual std::string typeName() const = 0;
};

/**
*/
class ABSTRACT_TYPE InvalidTypeException : public SerializationException
{
	DECLARE_ABSTRACT_EXCEPTION(SerializationException, Runtime::SerializationDetail::InvalidTypeExceptionImpl)
public:
	virtual std::string expectedTypeName() const = 0;
	virtual std::string actualTypeName() const = 0;
};

/**
*/
class NumericOverflowExcepion : public SerializationException
{
	DECLARE_ABSTRACT_EXCEPTION(SerializationException, Runtime::SerializationDetail::NumericOverflowExcepionImpl)
public:
};

/**
*/
class EndOfStreamException : public SerializationException
{
	DECLARE_ABSTRACT_EXCEPTION(SerializationException, Runtime::SerializationDetail::EndOfStreamExceptionImpl)
};

/**
*/
struct RequiredFieldAttribute : Meta::Attribute
{};

/**
*/
struct IgnoreEmptyFieldAttribute : Meta::Attribute
{};

} // namespace Serialization

//-----------------------------------------------------------------------------
namespace SerializationDetail {

/**
	Implements Serialization::SerializationException
*/
class SerializationExceptionImpl final : public ExceptionImpl<Serialization::SerializationException>
{
	using ExceptionBase = ExceptionImpl<Serialization::SerializationException>;
	DECLARE_CLASS_BASE(ExceptionBase)
public:
	using ExceptionBase::ExceptionImpl;
};

/**
	Implements Serialization::RequiredFieldMissedException
*/
class RequiredFieldMissedExceptionImpl final : public ExceptionImpl<Serialization::RequiredFieldMissedException>
{
	using ExceptionBase = ExceptionImpl<Serialization::RequiredFieldMissedException>;
	DECLARE_CLASS_BASE(ExceptionBase)
public:

	RequiredFieldMissedExceptionImpl(const Diagnostics::SourceInfo& sourceInfo_, std::string_view typeName_, std::string_view fieldName_)
		: ExceptionBase(sourceInfo_, Core::Format::format("Required field ({}.{}) missed", typeName_, fieldName_))
		, m_typeName(typeName_)
		, m_fieldName(fieldName_)
	{}

	std::string typeName() const override {
		return m_typeName;
	}

	std::string fieldName() const override {
		return m_fieldName;
	}

private:
	std::string m_typeName;
	std::string m_fieldName;
};

/**
	Implements Serialization::RequiredFieldMissedException
*/
class InvalidTypeExceptionImpl  : public ExceptionImpl<Serialization::InvalidTypeException>
{
	using ExceptionBase = ExceptionImpl<Serialization::InvalidTypeException>;
	DECLARE_CLASS_BASE(ExceptionBase)
public:

	InvalidTypeExceptionImpl(const Diagnostics::SourceInfo& sourceInfo_, std::string_view expectedTypeName_, std::string_view actualTypeName_)
		: ExceptionBase(sourceInfo_, Core::Format::format("Expected type(category):({}), but:({})", expectedTypeName_, actualTypeName_))
		, m_expectedTypeName(expectedTypeName_)
		, m_actualTypeName(actualTypeName_)
	{}

	std::string expectedTypeName() const override {
		return m_expectedTypeName;
	}

	std::string actualTypeName() const override {
		return m_actualTypeName;
	}

private:
	std::string m_expectedTypeName;
	std::string m_actualTypeName;
};

/**
	Implements Serialization::NumericOverflowExcepion
*/
class NumericOverflowExcepionImpl final : public ExceptionImpl<Serialization::NumericOverflowExcepion>
{
	using ExceptionBase = ExceptionImpl<Serialization::NumericOverflowExcepion>;
	DECLARE_CLASS_BASE(ExceptionBase)
public:
	using ExceptionBase::ExceptionImpl;
};

/**
	Implements Serialization::EndOfStreamException
*/
class EndOfStreamExceptionImpl final : public ExceptionImpl<Serialization::EndOfStreamException>
{
	using ExceptionBase = ExceptionImpl<Serialization::EndOfStreamException>;
	DECLARE_CLASS_BASE(ExceptionBase)
public:
	using ExceptionBase::ExceptionImpl;

	EndOfStreamExceptionImpl(const Diagnostics::SourceInfo& source): ExceptionBase(source, "Unexpected end of stream")
	{}
};

}// namespace SerializationDetail
} // namespace Runtime
