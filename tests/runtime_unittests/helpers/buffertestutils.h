#pragma once
#include <runtime/memory/bytesbuffer.h>
#include <gtest/gtest.h>
#include <optional>

void fillBufferWithDefaultContent(Runtime::BytesBuffer& buffer, size_t offset = 0, std::optional<size_t> size = std::nullopt);

Runtime::BytesBuffer createBufferWithDefaultContent(size_t size);

testing::AssertionResult buffersEqual(const Runtime::BufferView& buffer1, const Runtime::BufferView& buffer2);
