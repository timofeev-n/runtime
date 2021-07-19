#pragma once

#include <runtime/com/comptr.h>

#include <chrono>
#include <optional>

namespace Runtime {


struct ABSTRACT_TYPE CancellationToken : IRefCounted
{
	using Ptr = ComPtr<CancellationToken>;

	virtual bool isCancelled() const = 0;
};

using Cancellation = CancellationToken::Ptr;


struct ABSTRACT_TYPE CancellationTokenSource
{
	using Ptr = std::unique_ptr<CancellationTokenSource>;

	static CancellationTokenSource::Ptr create();


	virtual ~CancellationTokenSource() = default;

	virtual bool isCancelled() const = 0;

	virtual Cancellation token() = 0;

	virtual void cancel() = 0;
};


class Expiration final
{
public:

	Expiration();

	Expiration(Cancellation cancellation, std::chrono::milliseconds timeout);

	Expiration(Cancellation cancellation);

	Expiration(std::chrono::milliseconds timeout);

	bool isExpired() const;

	operator Cancellation () const;

	inline static Expiration never()
	{
		return {};
	}

private:

	Cancellation m_cancellation;
	std::optional<std::chrono::milliseconds> m_timeout;
	const std::chrono::system_clock::time_point m_timePoint;
};

}
