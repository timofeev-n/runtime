//◦ Playrix ◦
#include "pch.h"
#if ! __has_include(<rapidjson/stringbuffer.h>)
#error rapid json dependency not used for current project
#endif


#include "jsonprimitive.h"
#include "runtime/serialization/json.h"
#include "runtime/com/comclass.h"
#include "runtime/utils/scopeguard.h"
#include "runtime/utils/intrusivelist.h"
#include "runtime/io/readerwriter.h"

#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <map>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4250)
#endif


namespace Runtime::Serialization {

namespace {

//
using DefaultEncoding = rapidjson::UTF8<char>;
using JsonDocument = rapidjson::GenericDocument<DefaultEncoding>;
using JsonValue = rapidjson::GenericValue<DefaultEncoding>;

template<typename Stream>
using JsonDefaultWriter = rapidjson::PrettyWriter<Stream,  DefaultEncoding, DefaultEncoding>;

/**

*/
//template<typename Encoding = rapidjson::UTF8<>>
class ReaderStream
{
public:
	using Ch = typename DefaultEncoding::Ch;

	static constexpr Ch NoChar = 0;

	ReaderStream(Io::Reader& reader_): m_reader(reader_)
	{
		this->fillBuffer();
	}

	Ch Peek() const
	{
		return (m_inBufferPos == m_avail) ? NoChar : m_buffer[m_inBufferPos];
	}

	Ch Take()
	{
		if (m_inBufferPos == m_avail)
		{
			return NoChar;
		}

		++m_offset;
		const Ch ch = m_buffer[m_inBufferPos];
		if (++m_inBufferPos == m_avail)
		{
			this->fillBuffer();
		}

		return ch;
	}

	size_t Tell() const
	{
		return m_offset;
	}

#pragma region Methods that must be never called, but required for compilation

	Ch* PutBegin() {
		Halt("PutBegin must not be caller for json::ReaderStream");
		return nullptr;
	}

	void Put(Ch) {
		Halt("Put must not be caller for json::ReaderStream");
	}

	void Flush() {
		Halt("Flush must not be caller for json::ReaderStream");
	}

	size_t PutEnd(Ch*) {
		Halt("PutEnd must not be caller for json::ReaderStream");
		return 0;
	}

#pragma endregion

private:

	static constexpr size_t PrefetchBufferSize = 64;
	using Buffer = std::array<Ch, PrefetchBufferSize>;

	void fillBuffer()
	{
		Assert(m_inBufferPos == m_avail);

		std::byte* const ptr = reinterpret_cast<std::byte*>(m_buffer.data());
		const size_t readSize = m_buffer.size() * sizeof(Ch);
		m_avail = m_reader.read(ptr, readSize);
		Assert(m_avail % sizeof(Ch) == 0);
		m_avail = m_avail / sizeof(Ch);
		m_inBufferPos = 0;
	}

	Io::Reader& m_reader;
	Buffer m_buffer;
	size_t m_offset = 0;
	size_t m_avail = 0;
	size_t m_inBufferPos = 0;
};



template<typename C = char>
class WriterStream
{
public:
	using Ch = C;

	WriterStream(Io::Writer& writer_) : m_writer(writer_)
	{}

	void Put(Ch c) {
		m_writer.write(reinterpret_cast<std::byte*>(&c), sizeof(c));
	}

	void Flush()
	{}

private:

	Io::Writer& m_writer;

	friend void PutUnsafe(WriterStream<C>& stream, Ch c)
	{
		stream.m_writer.write(reinterpret_cast<std::byte*>(&c), sizeof(c));
	}
};

Result<> parse(JsonDocument& document, ReaderStream& stream) {
	
	constexpr unsigned ParseFlags = rapidjson::kParseCommentsFlag | rapidjson::kParseStopWhenDoneFlag;

	document.ParseStream<ParseFlags>(stream);

	const auto error = document.GetParseError();

	switch (error)
	{
	case rapidjson::kParseErrorNone:
	{
		break;
	}

	case rapidjson::kParseErrorDocumentEmpty:
	{
		return Excpt(EndOfStreamException);
	}

	default:
	{
		return MAKE_Excpt(SerializationException, Core::Format::format("Json error:({})", error));
	}
	}

	return success;
}


class ValueOrDocument
{
public:
	ValueOrDocument(JsonDocument&& doc): m_valueOrDocument(std::in_place_type<JsonDocument>, std::move(doc))
	{}

	ValueOrDocument(JsonValue&& val): m_valueOrDocument(std::in_place_type<JsonValue>, std::move(val))
	{}

	ValueOrDocument(ValueOrDocument&& other) noexcept: m_valueOrDocument(std::move(other.m_valueOrDocument))// m_document(std::move(other.m_document)), m_value(std::move(other.m_value))
	{}

	JsonValue* operator-> ()
	{
		Assert(std::holds_alternative<JsonValue>(m_valueOrDocument) || std::holds_alternative<JsonDocument>(m_valueOrDocument));

		JsonValue& valueRef = std::holds_alternative<JsonValue>(m_valueOrDocument) ? std::get<JsonValue>(m_valueOrDocument) : static_cast<JsonValue&>(std::get<JsonDocument>(m_valueOrDocument));
		return &valueRef;
	}

	const JsonValue* operator-> () const
	{
		Assert(std::holds_alternative<JsonValue>(m_valueOrDocument) || std::holds_alternative<JsonDocument>(m_valueOrDocument));

		const JsonValue& valueRef = std::holds_alternative<JsonValue>(m_valueOrDocument) ? std::get<JsonValue>(m_valueOrDocument) : static_cast<const JsonValue&>(std::get<JsonDocument>(m_valueOrDocument));
		return &valueRef;
	}

	JsonValue& operator* ()
	{
		Assert(std::holds_alternative<JsonValue>(m_valueOrDocument) || std::holds_alternative<JsonDocument>(m_valueOrDocument));

		return std::holds_alternative<JsonValue>(m_valueOrDocument) ? std::get<JsonValue>(m_valueOrDocument) : static_cast<JsonValue&>(std::get<JsonDocument>(m_valueOrDocument));
	}

	const JsonValue& operator* () const
	{
		Assert(std::holds_alternative<JsonValue>(m_valueOrDocument) || std::holds_alternative<JsonDocument>(m_valueOrDocument));

		return std::holds_alternative<JsonValue>(m_valueOrDocument) ? std::get<JsonValue>(m_valueOrDocument) : static_cast<const JsonValue&>(std::get<JsonDocument>(m_valueOrDocument));
	}

private:
	std::variant<JsonValue, JsonDocument> m_valueOrDocument;
};


RuntimeValue::Ptr CreateRuntimeValueFromJson(ValueOrDocument value);


class JsonRuntimeValueBase : public virtual RuntimePrimitiveValue
{
	DECLARE_CLASS_BASE(RuntimePrimitiveValue)

public:

	JsonRuntimeValueBase(ValueOrDocument jsonValue): m_jsonValue(std::move(jsonValue))
	{}

	bool isMutable() const override
	{
		return false;
	}


	JsonValue& jsonValue()
	{
		return *m_jsonValue;
	}

	const JsonValue& jsonValue() const
	{
		return *m_jsonValue;
	}

private:

	ValueOrDocument m_jsonValue;
};


class JsonRuntimeIntegerValue final : public virtual JsonRuntimeValueBase, public virtual RuntimeIntegerValue
{
	COMCLASS_(JsonRuntimeValueBase, RuntimeIntegerValue)

public:

	using JsonRuntimeValueBase::JsonRuntimeValueBase;

	bool isSigned() const override {
		return this->jsonValue().IsInt() || this->jsonValue().IsInt64();
	}

	size_t bits() const {
		return this->jsonValue().IsInt64() || this->jsonValue().IsUint64() ? sizeof(int64_t) : sizeof(int32_t);
	}

	void setInt64(int64_t) override {
	}

	void setUint64(uint64_t) override {
	}

	int64_t getInt64() const override {
		return this->getWithRangeCheck<int64_t>();
	}
	uint64_t getUint64() const override {
		return this->getWithRangeCheck<uint64_t>();
	}

private:

	template<typename T>
	T getWithRangeCheck() const // requires(std::is_integral_v<T>)
	{
		decltype(auto) value = this->jsonValue();

		if (value.IsInt())
		{
			return castWithRangeCheck<T>(value.GetInt());
		}
		else if (value.IsUint())
		{
			return castWithRangeCheck<T>(value.GetUint());
		}
		else if (value.IsInt64())
		{
			return castWithRangeCheck<T>(value.GetInt64());
		}

		Assert(value.IsUint64());

		return castWithRangeCheck<T>(value.GetUint64());
	}
	
	template<typename T, typename U>
	inline static T castWithRangeCheck(U value)
	{
		if constexpr (sizeof(T) < sizeof(U))
		{
			// if (static_cast<std::numeric_limits<T>::max() < value
		}

		return static_cast<T>(value);
	}
};


class JsonRuntimeBooleanValue final : public virtual JsonRuntimeValueBase, public virtual RuntimeBooleanValue
{
	COMCLASS_(JsonRuntimeValueBase, RuntimeBooleanValue)

public:

	using JsonRuntimeValueBase::JsonRuntimeValueBase;

	void set(bool) override {
	}

	bool get() const override {
		Assert(this->jsonValue().GetType() == rapidjson::kTrueType || this->jsonValue().GetType() == rapidjson::kFalseType);
		return this->jsonValue().GetType() == rapidjson::kTrueType;
	}
};


class JsonRuntimeFloatValue final : public virtual JsonRuntimeValueBase, public RuntimeFloatValue
{
	COMCLASS_(JsonRuntimeValueBase, RuntimeFloatValue)

public:

	using JsonRuntimeValueBase::JsonRuntimeValueBase;

	size_t bits() const override {
		return this->jsonValue().IsDouble() ? sizeof(double) : sizeof(float);
	}

	void setDouble(double) override {
	}

	void setSingle(float) override {
	}

	double getDouble() const override {

		if (this->jsonValue().IsDouble()) {
			return this->jsonValue().GetDouble();
		}

		Assert(this->jsonValue().IsFloat());
		return static_cast<double>(this->jsonValue().GetFloat());
	}
	
	float getSingle() const override {
		return this->jsonValue().GetFloat();
	}
};


class JsonRuntimeStringValue final: public virtual JsonRuntimeValueBase, public RuntimeStringValue
{
	COMCLASS_(JsonRuntimeValueBase, RuntimeStringValue)

public:

	using JsonRuntimeValueBase::JsonRuntimeValueBase;


	void setUtf8(std::string_view) override {

	}

	void set(std::wstring) override {
	}

	std::wstring get() const override {
		Halt("JSON string UTF-8 -> std::wstring conversion not implemented");
		return {};
	}

	std::string getUtf8() const override {
		const char* const ptr = this->jsonValue().GetString();
		const size_t len = static_cast<size_t>(this->jsonValue().GetStringLength());
		return {ptr, len};
	}
};



class JsonRuntimeArray final : public JsonRuntimeValueBase, public virtual RuntimeReadonlyCollection, public virtual RuntimeTuple
{
	COMCLASS_(JsonRuntimeValueBase, RuntimeReadonlyCollection, RuntimeTuple)

public:

	JsonRuntimeArray(ValueOrDocument value)
		: JsonRuntimeValueBase(std::move(value))
		, m_jsonArray(arrayOfValue(this->jsonValue()))
		, m_size(static_cast<size_t>(this->m_jsonArray.Size()))
	{
		m_elements.resize(this->size());
	}

	size_t size() const override
	{
		return m_size;
	}

	RuntimeValue::Ptr element(size_t index) const override
	{
		if (this->size() <= index)
		{
			throw Excpt_("Index ({0}) out of bounds [0, {1}]", index, this->size());
		}

		if (!m_elements[index])
		{
			JsonValue& el = m_jsonArray[static_cast<rapidjson::SizeType>(index)];
			m_elements[index] = CreateRuntimeValueFromJson(std::move(el));
		}

		return m_elements[index];
	}

	//void clear() override
	//{
	//}

	//void reserve(size_t) override
	//{
	//}

	//void push(RuntimeValue::Ptr) override
	//{}

private:

	static inline JsonValue::Array arrayOfValue(JsonValue& value) {
		Assert(value.IsArray());
		return value.GetArray();
	}


	JsonValue::Array m_jsonArray;
	const size_t m_size;
	mutable std::vector<RuntimeValue::Ptr> m_elements;
};


class JsonRuntimeDictionary final : public JsonRuntimeValueBase, public virtual RuntimeReadonlyDictionary
{
	COMCLASS_(JsonRuntimeValueBase, RuntimeReadonlyDictionary)

public:
	JsonRuntimeDictionary(ValueOrDocument jsonValue): JsonRuntimeValueBase(std::move(jsonValue)) {
		Assert(this->jsonValue().IsObject());

		JsonValue::Object obj = this->jsonValue().GetObj();
		
		for (auto& member : obj)
		{
			std::string_view memberName = member.name.GetString();
			auto value = CreateRuntimeValueFromJson(std::move(member.value));

			m_members.emplace(std::move(memberName), std::move(value));
			// member.value
		}
	}

	size_t size() const override
	{
		return m_members.size();
	}

	std::string_view key(size_t index) const override
	{
		Assert2(index < this->size(), Core::Format::format("Invalid index ({}) > size:({})", index, m_members.size()));

		auto iter = m_members.begin();
		std::advance(iter, index);

		return iter->first;
	}

	RuntimeValue::Ptr value(std::string_view key) const override
	{
		if (auto iter = m_members.find(key); iter != m_members.end())
		{
			return iter->second;
		}

		return nothing;
	}

	bool hasKey(std::string_view key) const override
	{
		return m_members.find(key) != m_members.end();
	}

private:

	//std::map<std::string_view, ComPtr<JsonRuntimeValueBase>, std::less<>> m_members;
	std::map<std::string, ComPtr<JsonRuntimeValueBase>, std::less<>> m_members;

};

//-----------------------------------------------------------------------------
RuntimeValue::Ptr CreateRuntimeValueFromJson(ValueOrDocument value) {
	const rapidjson::Type valueType = value->GetType();

	if (valueType == rapidjson::kNumberType) {

		if (value->IsFloat() || value->IsDouble()) {
			return Com::createInstance<JsonRuntimeFloatValue>(std::move(value));
		}

		Assert(value->IsInt() || value->IsUint() || value->IsInt64() || value->IsUint64());

		return Com::createInstance<JsonRuntimeIntegerValue, RuntimeValue>(std::move(value));
	}

	if (valueType == rapidjson::kTrueType || valueType ==  rapidjson::kFalseType) {
		return Com::createInstance<JsonRuntimeBooleanValue, RuntimeValue>(std::move(value));
	}

	if (valueType == rapidjson::kStringType) {
		return Com::createInstance<JsonRuntimeStringValue, RuntimeValue>(std::move(value));
	}

	if (valueType == rapidjson::kArrayType) {
		return Com::createInstance<JsonRuntimeArray>(std::move(value));
	}

	if (valueType == rapidjson::kObjectType) {
		return Com::createInstance<JsonRuntimeDictionary>(std::move(value));
	}

	Halt(Core::Format::format("Unhandled Json value type while creating runtime value"));

	return {};
}


template<typename Stream, typename SrcEnc, typename TargetEnc>
void writeJsonPrimitiveValue(rapidjson::Writer<Stream, SrcEnc, TargetEnc>& writer, const RuntimePrimitiveValue& value)
{
	if (auto integer = value.as<const RuntimeIntegerValue*>(); integer)
	{
		if (integer->isSigned())
		{
			writer.Int64(integer->getInt64());
		}
		else
		{
			writer.Uint64(integer->getUint64());
		}
	}
	else if (auto floatPoint = value.as<const RuntimeFloatValue*>(); floatPoint)
	{
		if (floatPoint->bits() == sizeof(double))
		{
			writer.Double(floatPoint->getDouble());
		}
		else
		{
			writer.Double(floatPoint->getDouble());
		}
	}
	else if (auto str = value.as<const RuntimeStringValue*>(); str) {
		auto text = str->getUtf8();
		writer.String(text.c_str(), static_cast<rapidjson::SizeType>(text.length()), true);
	}
	else if (auto boolValue = value.as<const RuntimeBooleanValue*>(); boolValue) {
		writer.Bool(boolValue->get());
	}
	else {
		Halt("Unknown primitive type for json serialization");
	}
}



/**
*/ 
template<typename Stream, typename SrcEnc, typename TargetEnc>
Result<> writeJsonValue(rapidjson::Writer<Stream, SrcEnc, TargetEnc>& writer, const RuntimeValue::Ptr& value)
{
	if (const RuntimeOptionalValue* optionalValue = value->as<const RuntimeOptionalValue*>()) {
		if (optionalValue->hasValue()) {
			return writeJsonValue(writer, optionalValue->value());
		}

		writer.Null();
		return success;
	}

	if (const RuntimeValueRef* refValue = value->as<const RuntimeValueRef*>()) {
		const auto referencedValue = refValue->get();
		if (referencedValue) {
			return writeJsonValue(writer, referencedValue);
		}

		writer.Null();
		return success;
	}

	
	if (const RuntimePrimitiveValue* primitiveValue = value->as<const RuntimePrimitiveValue*>(); primitiveValue) {
		writeJsonPrimitiveValue(writer, *primitiveValue);
	}
	else if (value->is<RuntimeReadonlyCollection>()) {
		writer.StartArray();

		SCOPE_Success{
			writer.EndArray();
		};

		const RuntimeReadonlyCollection& array = value->as<const RuntimeReadonlyCollection&>();

		for (size_t i = 0, sz = array.size(); i < sz; ++i)
		{
			if (auto result = writeJsonValue(writer, array[i]); !result)
			{
				return result;
			}
		}
	}
	else if (value->is<RuntimeReadonlyDictionary>()) {
		writer.StartObject();

		SCOPE_Success{
			writer.EndObject();
		};

		const RuntimeReadonlyDictionary& obj = value->as<const RuntimeReadonlyDictionary&>();
		for (size_t i = 0, sz = obj.size(); i < sz; ++i)
		{
			auto key = obj.key(i);
			auto value = obj.value(key);

			writer.String(key.data(), static_cast<rapidjson::SizeType>(key.size()));

			if (auto result = writeJsonValue(writer, value); !result) {
				return result;
			}
		}
	}

	return success;
}

} // namespace


Result<> JsonWrite(Io::Writer& charWriter, const RuntimeValue::Ptr& value, JsonSettings settings)
{
	WriterStream<> stream(charWriter);

	JsonDefaultWriter<WriterStream<>> jsonWriter(stream);

	return writeJsonValue(jsonWriter, value);
}

Result<RuntimeValue::Ptr> JsonParse(Io::Reader& reader)
{
	JsonDocument document;
	ReaderStream stream{reader};

	if (auto result = parse(document, stream); !result)
	{
		return result.err();
	}

	return CreateRuntimeValueFromJson(std::move(document));

	// return RuntimeValue::Ptr{};
}

//Result<> jsonDeserialize(io::Reader&, RuntimeValue::Ptr)
//{
//	return success;
//}

Result<RuntimeValue::Ptr> JsonParseString(std::string_view str) {
	if (str.empty()) {
		return Excpt_("Empty string");
	}

	Io::StringReader reader{str};
	return JsonParse(reader);
}

} // namespace Runtime::Serialization

#ifdef _MSC_VER
#pragma warning(pop)
#endif
