#pragma once
#include "representationbase.h"
#include <runtime/utils/typeutility.h>

namespace Runtime {

template<typename T>
struct DictionaryValueOperations;
//
//template<typename T>
//concept DictionaryRepresentable =
//	requires(const T& dict)
//	{
//		typename DictionaryValueOperations<T>::Key;
//		typename DictionaryValueOperations<T>::Value;
//
//		{DictionaryValueOperations<T>::size(dict)} -> concepts::AssignableTo<size_t&>;
//		{DictionaryValueOperations<T>::key(dict, size_t{0})} -> concepts::AssignableTo<typename DictionaryValueOperations<T>::Key&>;
//		{DictionaryValueOperations<T>::find(dict, std::declval<typename DictionaryValueOperations<T>::Key>(), (std::add_const_t<typename DictionaryValueOperations<T>::Value*>*)nullptr)} -> concepts::Same<bool>;
//	} &&
//	requires(T& dict)
//	{
//		DictionaryValueOperations<T>::clear(dict);
//		{DictionaryValueOperations<T>::emplace(dict, std::declval<typename DictionaryValueOperations<T>::Key>())} -> concepts::AssignableTo<typename DictionaryValueOperations<T>::Value&>;
//		{DictionaryValueOperations<T>::find(dict, std::declval<typename DictionaryValueOperations<T>::Key>(), (typename DictionaryValueOperations<T>::Value**)nullptr)} -> concepts::Same<bool>;
//	};
//
//



namespace Detail {

template<typename T>
decltype(
	ConstLValueRef<typename DictionaryValueOperations<T>::Key>(),
	ConstLValueRef<typename DictionaryValueOperations<T>::Value>(),
	DictionaryValueOperations<T>::size(ConstLValueRef<T>()),
	DictionaryValueOperations<T>::key(ConstLValueRef<T>(), size_t{0}),
	// DictionaryValueOperations<T>::find(ConstLValueRef<T>(), ConstLValueRef<typename DictionaryValueOperations<T>::Key>()),
	DictionaryValueOperations<T>::clear(LValueRef<T>()),

	std::true_type{}) DictionaryRepresentableHelper(int);

template<typename>
std::false_type DictionaryRepresentableHelper(...);

}

template<typename T>
inline constexpr bool IsDictionaryRepresentable = decltype(Detail::DictionaryRepresentableHelper<T>(int{}))::value;



}

