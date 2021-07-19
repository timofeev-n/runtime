#pragma once
#include <runtime/com/comutils.h>
// #include <runtime/diagnostics/logging.h>
#include <runtime/utils/disposable.h>
#include <runtime/utils/nothing.h>

#include <EngineAssert.h>


namespace Runtime {
namespace Detail {


template<typename T>
inline IRefCounted& asRefCounted(T& instance)
{
	[[maybe_unused]] IRefCounted* rc = nullptr;

	if constexpr (std::is_convertible_v<T*, IRefCounted*>)
	{
		return instance;
	}
	else if constexpr (std::is_convertible_v<T*, IAnything*>)
	{
		rc = static_cast<IAnything&>(instance).as<IRefCounted*>();
	}
	else if (rc = Runtime::Com::cast<IRefCounted>(instance); !rc)
	{
		rc = dynamic_cast<IRefCounted*>(&instance);
	}

	// Assert(rc, "Runtime can not find IRefCounted for (%1)", typeid(T))

	return *rc;
}

} // Detail


namespace Com {

/// <summary>
///
/// </summary>
template<typename T>
struct Acquire
{
	T* const ptr;

	Acquire(T* ptr_) : ptr(ptr_)
	{}

	Acquire(const Acquire&) = delete;
};


/// <summary>
///
/// </summary>
template<typename T>
struct TakeOwnership
{
	T* const ptr;

	TakeOwnership(T* ptr_) : ptr(ptr_)
	{}

	TakeOwnership(const TakeOwnership&) = delete;
};

} // namespace Com

template<typename T = IRefCounted>
class ComPtr
{
public:

	using type = T;

	ComPtr() = default;

	ComPtr(Nothing)
	{}

	ComPtr(const ComPtr<T>& other): m_instance(other.m_instance)
	{
		if (m_instance != nullptr)
		{
			Detail::asRefCounted(*m_instance).addRef();
		}
	}

	ComPtr(ComPtr<T>&& other): m_instance(other.giveup())
	{}

	ComPtr(const Com::Acquire<T>& acquire_): m_instance(acquire_.ptr)
	{
		if (m_instance)
		{
			Detail::asRefCounted(*m_instance).addRef();
		}
	}

	ComPtr(const Com::TakeOwnership<T>& ownership_): m_instance(ownership_.ptr)
	{}

	template<typename U, 
		std::enable_if_t<!std::is_same_v<U,T>, int> = 0>
	// requires (!std::is_same_v<U,T>)
	ComPtr(const ComPtr<U>& other)
	{
		//static_assert(std::is_convertible_v<U&, T&> || std::is_same_v<T, Com::IRefCountedObject> || std::is_same_v<U, Com::IRefCountedObject>, "Unsafe type cast");
		this->acquire(other.get());
	}

	template<typename U,
		std::enable_if_t<!std::is_same_v<U,T>, int> = 0>
	////requires (!std::is_same_v<U,T>)
	ComPtr(ComPtr<U>&& other)
	{
		//static_assert(std::is_convertible_v<U&, T&> || std::is_same_v<T, Com::IRefCountedObject> || std::is_same_v<U, Com::IRefCountedObject>, "Unsafe type cast");
		this->moveAcquire(other.giveup());
	}

	~ComPtr() {
		this->release();
	}

	T* giveup()
	{
		T* const instance = m_instance;
		m_instance = nullptr;

		return instance;
	}

	T* get() const
	{
		return m_instance;
	}

	void reset(T* ptr = nullptr)
	{
		acquire(ptr);
	}

	ComPtr<T>& operator = (const ComPtr<T>& other)
	{
		this->acquire(other.m_instance);
		return *this;
	}

	ComPtr<T>& operator = (ComPtr<T>&& other)
	{
		this->moveAcquire(other.giveup());
		return *this;
	}

	template<typename U,
		std::enable_if_t<!std::is_same_v<U,T>, int> = 0>
	//requires (!std::is_same_v<U,T>)
	ComPtr<T>& operator = (const ComPtr<U>& other)
	{
		U* instance = other.get();
		this->acquire<U>(instance);
		return *this;
	}

	template<typename U,
		std::enable_if_t<!std::is_same_v<U,T>, int> = 0>
	//requires (!std::is_same_v<U,T>)
	ComPtr<T>& operator = (ComPtr<U>&& other)
	{
		this->moveAcquire<U>(other.giveup());
		return *this;
	}

	T& operator * () const
	{
		// RUNTIME_CHECK(m_instance, "ComPtr<%1> is not dereferencible", typeid(T))
		return *m_instance;
	}

	T* operator -> () const
	{
		// RUNTIME_CHECK(m_instance , "ComPtr<%1> is not dereferencible", typeid(T))

		return m_instance;
	}

	explicit operator bool() const
	{
		return m_instance != nullptr;
	}


private:

	void release()
	{
		if (m_instance)
		{
			T* const temp = m_instance;
			m_instance = nullptr;
			Detail::asRefCounted(*temp).release();
		}
	}

	void acquire(T* instance)
	{
		T* const prevInstance = m_instance;

		if (m_instance = instance; m_instance)
		{
			Detail::asRefCounted(*m_instance).addRef();
		}

		if (prevInstance)
		{
			Detail::asRefCounted(*prevInstance).release();
		}
	}

	void moveAcquire(T* instance)
	{
		release();
		m_instance = instance;
	}

	template<typename U,
		std::enable_if_t<!std::is_same_v<U,T>, int> = 0>
		//requires (!std::is_same_v<U,T>)
	void acquire(U* instance)
	{
		release();
		if (instance == nullptr)
		{
			return;
		}
		
		if (m_instance = Detail::asRefCounted(*instance).template as<T*>(); m_instance != nullptr)
		{
			Detail::asRefCounted(*m_instance).addRef();
		}
		else
		{
			// LOG_warning_("Expected API not exposed: (%2). Source type (%1)", typeid(U), typeid(T))
#if (defined(DEBUG) || !defined(NDEBUG)) && defined(_APISETDEBUG_)
			if (IsDebuggerPresent() == TRUE)
			{
				DebugBreak();
			}
#endif
		}
	}

	template<typename U,
		std::enable_if_t<!std::is_same_v<U,T>, int> = 0>
	//requires (!std::is_same_v<U,T>)
	void moveAcquire(U* instance)
	{
		release();
		if (instance == nullptr)
		{
			return;
		}

		if (m_instance = Detail::asRefCounted(*instance).template as<T*>(); m_instance == nullptr)
		{
//			LOG_warning_("Expected API not exposed: (%2). Source type (%1)", typeid(U), typeid(T))

#if (defined(DEBUG) || !defined(NDEBUG)) && defined(_APISETDEBUG_)
			if (IsDebuggerPresent() == TRUE)
			{
				DebugBreak();
			}
#endif
			Detail::asRefCounted(*instance).release();
		}
	}

	T* m_instance = nullptr;
};


template<typename T>
ComPtr(Com::Acquire<T>) -> ComPtr<T>;

template<typename T>
ComPtr(Com::TakeOwnership<T>) -> ComPtr<T>;


template<typename T,
	std::enable_if_t<std::is_assignable_v<Disposable&, T&>, int> = 0>
inline void dispose(ComPtr<T>& ptr) //requires (std::is_assignable_v<Disposable&, T&>)
{
	if (ptr)
	{
		Disposable& disposable = static_cast<Disposable&>(*ptr);
		disposable.dispose();
	}
}

template<typename T,
	std::enable_if_t<!std::is_assignable_v<Disposable&, T&>, int> = 0>
inline void dispose(ComPtr<T>& ptr)
{
	if (Disposable* const disposable = ptr ? ptr->template as<Disposable*>() : nullptr; disposable)
	{
		disposable->dispose();
	}
}

template<typename T>
class DisposableGuard<ComPtr<T>> : protected ComPtr<T>
{
public:
	using ComPtr<T>::ComPtr;
	using ComPtr<T>::operator ->;
	using ComPtr<T>::operator *;

	DisposableGuard(ComPtr<T> ptr): ComPtr<T>(std::move(ptr))
	{}

	~DisposableGuard()
	{
		dispose(static_cast<ComPtr<T>&>(*this));
	}
};


template<typename T>
DisposableGuard(ComPtr<T>) -> DisposableGuard<ComPtr<T>>;


} // namespace Runtime


#define using_(var_) ::Runtime::DisposableGuard var_
