//◦ Playrix ◦
#pragma once
#include <runtime/com/comclass.h>
#include <runtime/io/reader.h>
#include <runtime/io/writer.h>
#include <runtime/memory/bytesbuffer.h>

#include <EngineAssert.h>

#include <string>

namespace Runtime::Io {
/**
* 
*/
class StringWriter final : public Runtime::Io::Writer
{
	DECLARE_CLASS_BASE(Runtime::Io::Writer)
	IMPLEMENT_ANYTHING

public:

	StringWriter(std::string& str_): m_str(str_)
	{}

	void write(const std::byte* ptr, size_t size) override {
		m_str.append(reinterpret_cast<const char*>(ptr), size);
	}

	size_t offset() const override {
		return m_str.size();
	}

private:
	std::string& m_str;
};

/**
* 
*/
class BufferWriter final : public Runtime::Io::Writer 
{
	DECLARE_CLASS_BASE(Runtime::Io::Writer)
	IMPLEMENT_ANYTHING

public:

	BufferWriter(Runtime::BytesBuffer& buffer_): m_buffer(buffer_), m_initialSize(buffer_.size())
	{}

	void write(const std::byte* ptr, size_t size) override {
		memcpy(m_buffer.append(size), ptr, size);
	}

	size_t offset() const override { 
		return m_buffer.size() - m_initialSize;
	}

private:
	Runtime::BytesBuffer& m_buffer;
	const size_t m_initialSize;
};

/**
* 
*/
class StringReader : public Runtime::Io::Reader
{
	using Ch = char;

	DECLARE_CLASS_BASE(Runtime::Io::Reader)
	IMPLEMENT_ANYTHING

public:

	StringReader(std::string_view str) : m_str{str}
	{}

	size_t read(std::byte* buffer, size_t readCount) override {
		Assert2((readCount % sizeof(Ch)) == 0, Core::Format::format("Invalid read bytes cout: ({}), because size of char:(})", readCount, sizeof(Ch)));

		const size_t bytesAvailable = (m_str.size() * sizeof(Ch)) - m_offset;
		const auto actualRead = std::min(bytesAvailable, readCount);

		if (actualRead == 0) {
			return 0;
		}

		if (readCount == sizeof(Ch)) {
			*reinterpret_cast<Ch*>(buffer) = m_str[m_offset];
		}
		else {
			Assert((m_offset % sizeof(Ch)) == 0);

			const size_t charsOffset = m_offset / sizeof(Ch);
			memcpy(buffer, m_str.data() + charsOffset, actualRead);
		}
		m_offset += actualRead;
		return actualRead;
	}
private:
	std::string_view m_str;
	size_t m_offset = 0;
};

} // namespace Runtime::Io
