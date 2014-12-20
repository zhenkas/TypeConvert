#pragma once
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/preprocessor.hpp>
#include <memory>
#include "pp_common.h"
#include "ttraits.h"
#include "TypeConvert.h"

struct ISafeObject;
typedef std::shared_ptr<ISafeObject> TSafePtr;
typedef std::weak_ptr<ISafeObject> TWeakPtr;


struct ISafeObject
{
protected:
	inline ISafeObject() PP_DEBUG_ONLY(:m_typeName("Not created using ISafeObject::Create method")) {}
	inline ISafeObject(const ISafeObject &) {}
public:
	virtual ~ISafeObject() {}
	template <typename T, typename ...TParams>
	struct Creator : T
	{
		Creator(TParams&&... params) : T(std::forward<TParams>(params)...) {}
	private:
		virtual void This_Class_Must_Be_Created_Using_SafeObject_Template() override {}
#pragma warning(suppress: 4250) // "inherits via dominance"
	};
	template <typename T, typename ...TParams>
	inline static T & Create(TSafePtr & out_ptr, TParams&&... params)
	{
		BOOST_STATIC_ASSERT_MSG((boost::is_base_of<ISafeObject, T>::value), "The class must derive from ISafeObject");
		T * pNew = new Creator<T, TParams...>(std::forward<TParams>(params)...);
		if (pNew->m_weakPtr.use_count() == 0)
		{
			out_ptr = TSafePtr(pNew);
			pNew->m_weakPtr = out_ptr;
		}
		else
		{
			out_ptr = TSafePtr(pNew->m_weakPtr);
		}
		PP_DEBUG_ONLY(pNew->m_typeName = typeid(T).name();)
			return *pNew;
	}
private:
	//This function declared to prevent create classes inherited from ISafeObject directly.
	//Such classes must be created by SafeObject<T> template!
	virtual void This_Class_Must_Be_Created_Using_SafeObject_Template() = 0;
protected:
	const ISafeObject & operator= (const ISafeObject & /*s*/) {
		return *this;
	}
public:
	PP_DEBUG_ONLY(const char * m_typeName);
	mutable TWeakPtr m_weakPtr;
};

struct ConstructT 
{
	inline ConstructT(int){}
};
#define CONSTRUCT ConstructT(0)

#ifdef _MANAGED
struct ConstructT_gcroot
{
	inline ConstructT_gcroot(int){}
};
#define CONSTRUCT_GCROOT ConstructT_gcroot(0)
#endif 

template<typename T = ISafeObject>
struct SafeObject
{
	typedef typename std::remove_const<T>::type type;
	typedef typename std::add_const<type>::type const_type;
	const static bool isConstT = std::is_const<T>::value;
	inline SafeObject()
	{
		ptr = NULL;
	}
	template<typename O>
	inline SafeObject(O && val)
	{
		*this = val;
	}

	template<typename ...TParams>
	inline SafeObject(ConstructT, TParams&&... params)
	{
		CreateInstance<T>(std::forward<TParams>(params)...);
	}
#ifdef _MANAGED
	template<typename ...TParams>
	inline SafeObject(ConstructT_gcroot, TParams... params)
	{
		CreateInstance<T>(gcroot<TParams>(params)...);
	}
#endif
	template <typename NewInstanceType = T, typename ...TParams>
	inline NewInstanceType & CreateInstance(TParams&&... params)
	{
		BOOST_STATIC_ASSERT_MSG((::boost::is_base_of<T, NewInstanceType>::value), "Not derived class");
		NewInstanceType & new_inst = ISafeObject::Create<NewInstanceType>(safePtr, std::forward<TParams>(params)...);
		ptr = &new_inst;
		return new_inst;
	}
	inline void ReleaseInstance()
	{
		safePtr = TSafePtr();
		ptr = NULL;
	}
	inline T * operator->() { BOOST_ASSERT(ptr != NULL); return ptr; }
	inline T & operator*() { BOOST_ASSERT(ptr != NULL); return *ptr; }
	inline const_type * operator->() const { BOOST_ASSERT(ptr != NULL); return ptr; }
	inline const_type & operator*() const { BOOST_ASSERT(ptr != NULL); return *ptr; }
	inline explicit operator bool() const { return ptr != NULL; }

	inline SafeObject<T> & OperatorEqual(type & val)
	{		
		return *this = &val;
	}
	inline SafeObject<T> & OperatorEqual(const_type & val)
	{
		return *this = &val;
	}
	template <typename O>
	SafeObject<T> & OperatorEqualPtr(O * val)
	{
		typedef decltype(static_cast<T *>(val)) check_constrain;
		BOOST_STATIC_ASSERT_MSG((::boost::is_base_of<T, O>::value), "Not derived class");

		if (!val) {
			ReleaseInstance();
			return *this;
		}
		if (val->m_weakPtr.use_count() == 0)
		{
			//this case may be only if assigning from constructor of ISafeObject type to self, before safe ptr on this is created
			safePtr = TSafePtr((ISafeObject *)val);
			val->m_weakPtr = safePtr;
		}
		else {
			safePtr = TSafePtr(val->m_weakPtr);
		}
		ptr = val;
		return *this;
	}
	template <typename O>
	inline SafeObject<T> & OperatorEqual(O * val)
	{
		return OperatorEqualPtr(val);
	}
	template <typename O>
	inline SafeObject<T> & OperatorEqual(const SafeObject<O> & src)
	{
		*this = (SafeObject<O>::const_type *) src.ptr;
		return *this;
	}
	template <typename O>
	inline SafeObject<T> & operator=(O && src)
	{
		return OperatorEqual(src);
	}
	template <typename O>
	inline SafeObject<T> & OperatorEqual(SafeObject<O> & src)
	{
		*this = src.ptr;
		return *this;
	}
	template <typename O>
	inline SafeObject<T> & OperatorEqual(const TypeConvert::ConvertSrcHolderNative<O> & src)
	{
		src.ConvertTo(*this);
		return *this;
	}
#ifdef _MANAGED
	template <typename O>
	inline SafeObject<T> & OperatorEqual(const TypeConvert::ConvertSrcHolderManaged<O> & src)
	{
		src.ConvertTo(*this);
		return *this;
	}
#endif
	inline SafeObject<T> & OperatorEqual(std::nullptr_t)
	{
		ReleaseInstance();
		return *this;
	}
	inline operator const_type & () const
	{
		BOOST_ASSERT(ptr != NULL);
		return *ptr;
	}
	inline operator T & ()
	{
		BOOST_ASSERT(ptr != NULL);
		return *ptr;
	}

	template<typename O>
	inline operator O * ()
	{
		BOOST_STATIC_ASSERT((boost::is_base_of<O, T>::value));
		return ptr;
	}
	template<typename O>
	inline operator const O*() const
	{
		BOOST_STATIC_ASSERT((boost::is_base_of<O, T>::value));
		return ptr;
	}
	template<typename O>
	inline const O * DynamicCast() const
	{
		O * ptr2 = dynamic_cast<O *>(ptr);
		return ptr2;
	}
	template<typename O>
	inline O * DynamicCast()
	{
		typedef decltype(dynamic_cast<O *>(ptr)) check_T;
		O * ptr2 = dynamic_cast<O *>(ptr);
		return ptr2;
	}
	template<typename TIndex>
	inline auto operator [] (TIndex i) -> decltype(ptr->operator[](i))
	{
		return ptr->operator[](i);
	}
public:
	TSafePtr safePtr;
	T * ptr;
};

namespace TypeConvert
{
	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_N_2(SafeObject<TDst> & dst, TSrc && src, typename CHECK_DST::type *)->ConvertCallerFromTypeN_N<TDst>;
#ifdef _MANAGED
	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_M_2(SafeObject<TDst> & dst, TSrc % src, typename CHECK_DST2::type *)-> ConvertCallerFromTypeN_M<TDst>;
#endif
	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_N_2(TDst & dst, const SafeObject<TSrc> & src, typename CHECK_SRC2::type *)->ConvertCallerFromTypeN_N<TSrc>;
#ifdef _MANAGED
	template<typename TDst, typename TSrc>
	auto GetConvertCallerM_N_2(TDst % dst, const SafeObject<TSrc> & src, typename CHECK_SRC2::type *)->ConvertCallerFromTypeM_N<TSrc>;
#endif
}

