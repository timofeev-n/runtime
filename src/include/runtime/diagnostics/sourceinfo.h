#pragma once
#include <runtime/utils/preprocessor.h>
#include <boost/optional.hpp>
#include <string_view>

namespace Runtime::Diagnostics {

/**
*/
struct SourceInfo
{
	const std::string_view functionName;
	const std::string_view filePath;
	const boost::optional<unsigned> line;


	SourceInfo(std::string_view function_, std::string_view filePath_, boost::optional<unsigned> line_ = boost::none): functionName(function_), filePath(filePath_), line(line_)
	{}

	explicit operator bool () const
	{
		return !functionName.empty() || !filePath.empty();
	}
};

}

#define INLINED_SOURCE_INFO ::Runtime::Diagnostics::SourceInfo{std::string_view{__FUNCTION__}, std::string_view{__FILE__}, static_cast<unsigned>(__LINE__)}
