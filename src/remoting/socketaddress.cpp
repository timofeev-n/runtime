#include "pch.h"
#include "runtime/remoting/socketaddress.h"
#include "runtime/diagnostics/exception.h"
#include "runtime/utils/strings.h"
//#include "runtime/utils/stringconv.h"

#include <regex>

using namespace Runtime::Strings;

namespace Runtime::Remoting {

namespace {

constexpr auto DefaultMatchOptions = std::regex_constants::ECMAScript | std::regex_constants::icase;

std::string_view ToStringView(std::sub_match<std::string_view::iterator> str) {
	const size_t len = str.second - str.first;
	if (len == 0) {
		return {};
	}

	const char* const ptr = &(*str.first);

	return std::string_view{ptr, len};
}


}

std::tuple<std::string_view, std::string_view> SocketAddress::parseAddressString(std::string_view address)
{
	std::match_results<std::string_view::iterator> match;

	if (!std::regex_match(address.begin(), address.end(), match, std::regex {"^([A-Za-z0-9\\+_-]+)://(.+)$", DefaultMatchOptions}))
	{
		throw Excpt_("Invalid address: [%1]", address);
	}

	Assert(match.size() == 3);


	std::string_view protocol = ToStringView(match[1]);
	std::string_view addressString = ToStringView(match[2]);

	return {protocol, addressString};
}

std::tuple<std::string_view, std::string_view> SocketAddress::parseIpcAddress(std::string_view address)
{
	std::string_view host;
	std::string_view service;

	std::match_results<std::string_view::iterator> match;

	if (std::regex_match(address.begin(), address.end(), match, std::regex{"^([A-Za-z0-9_\\.\\-]+)\\\\([^\\\\]+)$", DefaultMatchOptions})) {
		host = ToStringView(match[1]);
		if (host == ".") {
			host = "";
		}

		service = ToStringView(match[2]);
	}
	else {
		throw Excpt_("Invalid ipc address:{0}", address);
	}

	return std::tuple{std::move(host), std::move(service)};
}

std::tuple<std::string, std::string> SocketAddress::parseTcpAddress(std::string_view address)
{
	std::match_results<std::string_view::iterator> match;

	if (!(
		std::regex_match(address.begin(), address.end(), match, std::regex{"^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})?:(\\d{1,5})$", DefaultMatchOptions}) ||
		std::regex_match(address.begin(), address.end(), match, std::regex{"^([A-Za-z0-9\\._\\-]+):(\\d{1,5})$", DefaultMatchOptions}))
		)
	{
		throw Excpt_("Invalid tcp address:{0}", address);
	}

	std::string host(ToStringView(match[1]));
	std::string service(ToStringView(match[match.size() - 1]));

	return std::tuple{std::move(host), std::move(service)};
}

}
