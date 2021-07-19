#pragma once

#include <runtime/diagnostics/exception.h>


namespace Runtime::Network {

namespace NetworkDetail {

class NetworkExceptionImpl;

}


class ABSTRACT_TYPE NetworkException : public Exception
{
	DECLARE_ABSTRACT_EXCEPTION(Exception, NetworkDetail::NetworkExceptionImpl)
};


namespace NetworkDetail {

class NetworkExceptionImpl final : public ExceptionImpl<NetworkException>
{
	DECLARE_CLASS_BASE(ExceptionImpl<NetworkException>)

public:

	using ExceptionImpl<NetworkException>::ExceptionImpl;

	//NetworkExceptionImpl(const diagnostics::SourceInfo& sourceInfo_, std::string_view message = ""): ExceptionImpl<NetworkException>(sourceInfo_, message)
	//{}

	//NetworkExceptionImpl(const diagnostics::SourceInfo& sourceInfo_, std::string_view message = ""): ExceptionImpl<NetworkException>(sourceInfo_, message)
	//{}
	//	: ExceptionImpl<NetworkException>(sourceInfo_, format(L"Required field ({0}.{1}) missed", typeName_, fieldName_))
	//	, m_typeName(typeName_)
	//	, m_fieldName(fieldName_)
	//{}

};

}

}
