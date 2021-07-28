#pragma once
#define REPRESENT_STD_OPTIONAL
#define REPRESENT_BOOST_OPTIONAL

#include "arrayrepresentation.h"
#include "optionalrepresentation.h"
#include "tuplerepresentation.h"
#include "dictionaryrepresentation.h"

#include <EngineAssert.h>

#include <array>
#include <list>
#include <map>

#ifdef REPRESENT_STD_OPTIONAL
#include <optional>
#endif

#ifdef REPRESENT_BOOST_OPTIONAL

#if __has_include(<boost/optional.hpp>)
#include <boost/optional.hpp>
#else
#undef REPRESENT_BOOST_OPTIONAL
#endif

#endif

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace Runtime {

/**
	
*/
template<typename T, typename ... A>
struct ArrayValueOperations<std::vector<T, A...>>
{
	using Type = std::vector<T, A...>;

	static size_t size(const Type& container) {
		return container.size();
	}

	static decltype(auto) element(const Type& container, size_t index) {
		Assert2(index < container.size(), Core::Format::format("Invalid list access [{}], size ({})", container.size(), index));
		return container[index];
	}

	static decltype(auto) element(Type& container, size_t index) {
		Assert2(index < container.size(), Core::Format::format("Invalid list access [{}], size ({})", container.size(), index));
		return container[index];
	}

	static void clear(Type& container, boost::optional<size_t> reserve)
	{
		static_assert(!std::is_const_v<Type>, "Attempt to modify non mutable value");
		container.clear();
		if (reserve && *reserve > 0)
		{
			container.reserve(*reserve);
		}
	}

	static decltype(auto) emplaceBack(Type& container)
	{
		static_assert(!std::is_const_v<Type>, "Attempt to modify non mutable value");
		return container.emplace_back();
	}
};


/**

*/
template<typename T, typename ... Traits>
struct ArrayValueOperations<std::list<T,Traits...>>
{
	using Type = std::list<T,Traits...> ;


	static size_t size(const Type& container)
	{
		return container.size();
	}

	static decltype(auto) element(const Type& container, size_t index)
	{
		Assert2(index < container.size(), Core::Format::foramt("Invalid list access [{}], size ({})", container.size(), index));
		return *std::next(container.begin(), static_cast<typename Type::difference_type>(index));
	}

	static decltype(auto) element(Type& container, size_t index)
	{
		Assert2(index < container.size(), Core::Format::format("Invalid list access [{}], size ({})", container.size(), index));
		return *std::next(container.begin(), static_cast<typename Type::difference_type>(index));
	}

	static void clear(Type& container, boost::optional<size_t>)
	{
		container.clear();
	}

	static decltype(auto) emplaceBack(Type& container)
	{
		return container.emplace_back();
	}
};


/**

*/
template<typename T, size_t Size>
struct ArrayValueOperations<std::array<T, Size>>
{
	using Type = std::array<T, Size>;

	static_assert(Size > 0, "std::array<T,0> does not supported");

	constexpr static size_t size(const Type&)
	{
		return Size;
	}

	constexpr static decltype(auto) element(const Type& container, size_t index)
	{
		Assert2(index < container.size(), Core::Format::format("Invalid list access [{}], size ({})", container.size(), index));
		return container[index];
	}


	constexpr static decltype(auto) element(Type& container, size_t index)
	{
		Assert2(index < container.size(), Core::Format::format("Invalid list access [{}], size ({})", container.size(), index));
		return container[index];
	}

	static void clear(Type&, boost::optional<size_t>) {
		Halt("std::array does not supports reset operation");
	}

	template<typename ...Values>
	static decltype(auto) emplaceBack(Type& container) {
		Halt("std::array does not supports emplaceBack operation");
		return container[0];
	}
};

/**

*/
template<typename ... T>
struct TupleValueOperations<std::tuple<T...>>
{
	static constexpr size_t TupleSize = sizeof ... (T);

	using Tuple = std::tuple<T...>;
	using TupleElements = TypeList<T...>;

	template<size_t Index>
	static decltype(auto) element(Tuple& tuple)
	{
		static_assert(Index < TupleSize, "Invalid tuple element index");

		return std::get<Index>(tuple);
	}

	template<size_t Index>
	static decltype(auto) element(const Tuple& tuple)
	{
		static_assert(Index < TupleSize, "Invalid tuple element index");

		return std::get<Index>(tuple);
	}
};


template<typename First, typename Second>
struct TupleValueOperations<std::pair<First, Second>>
{
	static constexpr size_t TupleSize = 2;

	using Tuple = std::pair<First, Second>;
	using TupleElements = TypeList<First, Second>;

	template<size_t Index>
	static decltype(auto) element(Tuple& tuple)
	{
		static_assert(Index < TupleSize, "Invalid pair index");
		return std::get<Index>(tuple);
	}

	template<size_t Index>
	static decltype(auto) element(const Tuple& tuple)
	{
		static_assert(Index < TupleSize, "Invalid pair index");
		return std::get<Index>(tuple);
	}
};


#ifdef REPRESENT_BOOST_OPTIONAL

/**
*/
template<typename T>
struct OptionalValueOperations<boost::optional<T>>
{
	using ValueType = T;
	using Optional = boost::optional<T>;


	static bool hasValue(const Optional& opt)
	{
		return opt.has_value();
	}

	static const ValueType& value(const Optional& opt) {
		Assert2(opt, "Optional has no value");
		return *opt;
	}

	static ValueType& value(Optional& opt) {
		Assert2(opt, "Optional has no value");
		return *opt;
	}

	static void reset(Optional& opt) {
		opt.reset();
	}

	template<typename ... Values>
	static ValueType& emplace(Optional& opt, Values&& ... values) {
		opt.emplace(std::forward<Values>(values)...);
		return opt.value();
	}
};

#undef REPRESENT_BOOST_OPTIONAL
#endif


#ifdef REPRESENT_STD_OPTIONAL

/**
*/
template<typename T>
struct OptionalValueOperations<std::optional<T>>
{
	using ValueType = T;
	using Optional = std::optional<T>;


	static bool hasValue(const Optional& opt) {
		return opt.has_value();
	}

	static const ValueType& value(const Optional& opt) {
		Assert2(opt, "Optional has no value");
		return *opt;
	}

	static ValueType& value(Optional& opt) {
		Assert2(opt, "Optional has no value");
		return *opt;
	}

	static void reset(Optional& opt) {
		opt.reset();
	}

	template<typename ... Values>
	static ValueType& emplace(Optional& opt, Values&& ... values) {
		return opt.emplace(std::forward<Values>(values)...);
	}
};
#undef REPRESENT_STD_OPTIONAL
#endif



template<typename Key_, typename Value_, typename ... Traits>
struct DictionaryValueOperations<std::map<Key_, Value_, Traits ... >>
{
	using Key = Key_;
	using Value = Value_;
	using Type = std::map<Key, Value, Traits ... >;

	static size_t size(const Type& dict) {
		return dict.size();
	}

	static const Key& key(const Type& dict, size_t index) {
		Assert(index < dict.size());

		auto head = dict.begin();
		std::advance(head, index);
		return head->first;
	}

	static bool find(const Type& dict, const Key key, std::add_const_t<Value*>* value = nullptr) {
		if (auto iter = dict.find(key); iter != dict.end())
		{
			if (value)
			{
				*value = &iter->second;
			}

			return true;
		}

		if (value)
		{
			*value = nullptr;
		}

		return false;
	}

	static bool find(Type& dict, const Key key, Value** value = nullptr)
	{
		if (auto iter = dict.find(key); iter != dict.end())
		{
			if (value)
			{
				*value = &iter->second;
			}

			return true;
		}

		if (value)
		{
			*value = nullptr;
		}

		return false;
	}

	static void clear(Type& dict)
	{
		dict.clear();
	}

	template<typename ... Args>
	static decltype(auto) emplace(Type& dict, Key key, Args&& ... args)
	{
		auto [iter, success] = dict.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<Args>(args)...)
		);

		if (!success)
		{

		}

		return (iter->second); // force return reference
	}
};

}


