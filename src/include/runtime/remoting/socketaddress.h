#pragma once

#include <runtime/utils/result.h>

#include <string>
#include <string_view>
#include <tuple>

namespace Runtime::Remoting {


/// <summary>
///
/// </summary>
struct SocketAddress
{
	static std::tuple<std::string_view, std::string_view> parseAddressString(std::string_view address);

	static std::tuple<std::string_view, std::string_view> parseIpcAddress(std::string_view address);

	static std::tuple<std::string, std::string> parseTcpAddress(std::string_view address);
};


}
