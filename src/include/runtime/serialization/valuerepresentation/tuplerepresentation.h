#pragma once
#include "representationbase.h"
#include <runtime/utils/typeutility.h>

namespace Runtime {

template<typename>
struct TupleValueOperations;


namespace Detail {

template<typename T>
decltype(
	TupleValueOperations<T>::TupleSize,
	std::true_type{}) TupleRepresentableHelper(int);

template<typename>
std::false_type TupleRepresentableHelper(...);

} // namespace Detail

template<typename T>
inline constexpr bool IsTupleRepresentable = decltype(Detail::TupleRepresentableHelper<std::decay_t<T>>(int{}))::value;


//template<typename T>
//concept TupleRepresentable =
//	requires(const T& tup)
//	{
//		{TupleValueOperations<T>::TupleSize} -> concepts::AssignableTo<size_t&>;
//	};

#if 0
/**
*/
template<TupleRepresentable T>
class TupleRepresentation final : public RepresentationBase<T, T>
{
public:
	static constexpr size_t TupleSize = TupleValueOperations<T>::TupleSize;

	TupleRepresentation(T& tup): m_tuple(tup)
	{}

	template<size_t Idx>
	requires (Idx < TupleValueOperations<T>::TupleSize)
	auto element() const
	{
		decltype(auto) el = TupleValueOperations<T>::template element<Idx>(m_tuple);
		static_assert(std::is_reference_v<decltype(el)>);
		return represent(el);
	}

private:

	T& m_tuple;
};

/**
*/
template<TupleRepresentable T>
class ConstTupleRepresentation final : public RepresentationBase<T, T>
{
public:
	static constexpr size_t TupleSize = TupleValueOperations<T>::TupleSize;

	ConstTupleRepresentation(const T& tup): m_tuple(tup)
	{}

	template<size_t Idx>
	requires (Idx < TupleValueOperations<T>::TupleSize)
	auto element() const
	{
		decltype(auto) el = TupleValueOperations<T>::template element<Idx>(m_tuple);
		static_assert(std::is_reference_v<decltype(el)>);
		return represent(el);
	}

private:
	const T& m_tuple;
};

//-----------------------------------------------------------------------------

template<TupleRepresentable T>
TupleRepresentation<T> represent(T& tup)
{
	return tup;
}

template<TupleRepresentable T>
ConstTupleRepresentation<T> represent(const T& tup)
{
	return tup;
}

#endif

} // Runtime
