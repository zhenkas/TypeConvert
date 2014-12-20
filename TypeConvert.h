/***********************************************
Copyright(c) 2014, zhenkas
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :

*Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and / or other materials provided with the distribution.

* Neither the name of TypeConvert nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#ifdef _MANAGED
#include <gcroot.h>
#define REF_SYMBOL %
#else
#define REF_SYMBOL &
#endif

/*
	Usage:

	Declare new conversion in TypeConvert namespace

		namespace TypeConvert
		{
			inline void ConvertImplementation(TDestination & dst, const TSource & src)
			{
				dst = src;
			}
		}

	Declare new conversion inside source or destination struct/class type

		struct TDestination/TSource
		{
		public:
			inline static void ConvertImplementation(TDestination & dst, const TSource & src)
			{
				dst = src;
			}
		}	


	Usage:
		1.	TDst dst = TypeConvert::Convert(src);
		2.	TypeConvert::Convert(dst, src);
		3.	TDst dst = TypeConvert::Convert<TDst>(src);
*/

namespace TypeConvert
{
	using namespace Traits;

	template<typename T>
	struct ConvertCallerFromTypeN_N
	{
		typedef typename std::remove_reference<T>::type type;
		template<typename TDst, typename TSrc>
		static inline void Convert(TDst & dst, TSrc && src)
		{
			type::ConvertImplementation(dst, std::forward<TSrc>(src));
		}
	};
#ifdef _MANAGED
	template<typename T>
	struct ConvertCallerFromTypeM_N
	{
		typedef typename std::remove_reference<T>::type type;
		template<typename TDst, typename TSrc>
		static inline void Convert(TDst % dst, TSrc && src)
		{
			type::ConvertImplementation(dst, std::forward<TSrc>(src));
		}
	};
	template<typename T>
	struct ConvertCallerFromTypeN_M
	{
		typedef typename std::remove_reference<T>::type type;
		template<typename TDst, typename TSrc>
		static inline void Convert(TDst & dst, TSrc % src)
		{
			type::ConvertImplementation(dst, src);
		}
	};
#endif
	
	struct ConvertCallerFromNamespaceN_N
	{
		template<typename TDst, typename TSrc>
		static inline void Convert(TDst & dst, TSrc && src)
		{
			TypeConvert::ConvertImplementation(dst, std::forward<TSrc>(src));
		}
	};
#ifdef _MANAGED
	struct ConvertCallerFromNamespaceM_N
	{
		template<typename TDst, typename TSrc>
		static inline void Convert(TDst % dst, TSrc && src)
		{
			TypeConvert::ConvertImplementation(dst, std::forward<TSrc>(src));
		}
	};
	struct ConvertCallerFromNamespaceN_M
	{
		template<typename TDst, typename TSrc>
		static inline void Convert(TDst & dst, TSrc % src)
		{
			TypeConvert::ConvertImplementation(dst, src);
		}
	};
	struct ConvertCallerFromNamespaceM_M
	{
		template<typename TDst, typename TSrc>
		static inline void Convert(TDst % dst, TSrc % src)
		{
			TypeConvert::ConvertImplementation(dst, src);
		}
	}; 
#endif


#define CHECK_DST	TraceType<decltype(((std::remove_reference<TDst>::type*)0)->ConvertImplementation(dst, std::forward<TSrc>(src)))>
#define CHECK_DST2	TraceType<decltype(((std::remove_reference<TDst>::type*)0)->ConvertImplementation(dst, src))>
#define CHECK_SRC	TraceType<decltype(((std::remove_reference<TSrc>::type*)0)->ConvertImplementation(dst, std::forward<TSrc>(src)))>
#define CHECK_SRC2	TraceType<decltype(((std::remove_reference<TSrc>::type*)0)->ConvertImplementation(dst, src))>

	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_N_2(TDst & dst, TSrc && src, typename CHECK_DST::type *)-> ConvertCallerFromTypeN_N<TDst>;
	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_N_2(TDst & dst, TSrc && src, typename CHECK_SRC::type *)->ConvertCallerFromTypeN_N<TSrc>;
#ifdef _MANAGED
	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_M_2(TDst & dst, TSrc % src, typename CHECK_DST2::type *)-> ConvertCallerFromTypeN_M<TDst>;
	template<typename TDst, typename TSrc>
	auto GetConvertCallerM_N_2(TDst % dst, TSrc && src, typename CHECK_SRC::type *)-> ConvertCallerFromTypeM_N<TSrc>;	
#endif

	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_N(TDst & dst, TSrc && src, ...)->ConvertCallerFromNamespaceN_N;
	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_N(TDst & dst, TSrc && src, decltype(GetConvertCallerN_N_2(dst, std::forward<TSrc>(src), 0))*) -> decltype(GetConvertCallerN_N_2(dst, std::forward<TSrc>(src), 0));
#ifdef _MANAGED
	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_M(TDst & dst, TSrc % src, ...)-> ConvertCallerFromNamespaceN_M;
	template<typename TDst, typename TSrc>
	auto GetConvertCallerN_M(TDst & dst, TSrc % src, decltype(GetConvertCallerN_M_2(dst, src, 0))*) -> decltype(GetConvertCallerN_M_2(dst, src, 0));
	template<typename TDst, typename TSrc>
	auto GetConvertCallerM_N(TDst % dst, TSrc && src, ...)->ConvertCallerFromNamespaceM_N;
	template<typename TDst, typename TSrc>
	auto GetConvertCallerM_N(TDst % dst, TSrc && src, decltype(GetConvertCallerM_N_2(dst, std::forward<TSrc>(src), 0))*) -> decltype(GetConvertCallerM_N_2(dst, std::forward<TSrc>(src), 0));
#endif

//BASIC

	template<typename TDst, typename TSrc>
	auto GetConvertCaller(typename std::enable_if<TestIsNative<TDst>::value, TDst>::type & dst, typename std::enable_if<TestIsNative<TSrc>::value, TSrc>::type && src)-> decltype(GetConvertCallerN_N(dst, std::forward<TSrc>(src), 0));
#ifdef _MANAGED
	template<typename TDst, typename TSrc>
	auto GetConvertCaller(typename std::enable_if<TestIsNative<TDst>::value, TDst>::type & dst, typename std::enable_if<TestIsManaged<TSrc>::value, TSrc>::type % src)-> decltype(GetConvertCallerN_M(dst, src, 0));
	template<typename TDst, typename TSrc>
	auto GetConvertCaller(typename std::enable_if<TestIsManaged<TDst>::value, TDst>::type % dst, typename std::enable_if<TestIsNative<TSrc>::value, TSrc>::type && src)-> decltype(GetConvertCallerM_N(dst, std::forward<TSrc>(src), 0));
	template<typename TDst, typename TSrc>
	auto GetConvertCaller(typename std::enable_if<TestIsManaged<TDst>::value, TDst>::type % dst, typename std::enable_if<TestIsManaged<TSrc>::value, TSrc>::type % src)-> ConvertCallerFromNamespaceM_M;
#endif

	template<typename TSrc>
	struct ConvertSrcHolderNative
	{
		typedef typename std::add_lvalue_reference<TSrc>::type TSrcRef;
		template<typename TSrc2>
		inline ConvertSrcHolderNative(TSrc2 && src) : m_src(src) {}
		template<typename TDst>
		inline operator TDst () const
		{
			typedef PP_REMOVE_MANAGED_CONST(std::remove_const<std::remove_reference<TDst>::type>::type()) TDstClean;
			TDstClean dst;
			typedef decltype(GetConvertCaller<TDstClean, TSrcRef>(dst, m_src)) TConvertCaller;
			TConvertCaller::Convert(dst, m_src);
			return dst;
		}
		template<typename TDst>
		inline void ConvertTo(TDst & dst) const
		{
			typedef decltype(GetConvertCaller<TDst, TSrcRef>(dst, m_src)) TConvertCaller;
			TConvertCaller::Convert(dst, m_src);
		}
		TSrcRef m_src;
	};

#ifdef _MANAGED
	template<typename TSrc>
	struct ConvertSrcHolderManaged
	{
		inline ConvertSrcHolderManaged(TSrc % src) : m_src(src){}
		template<typename TDst>
		inline operator TDst () const
		{
			typedef PP_REMOVE_MANAGED_CONST(std::remove_const<std::remove_reference<TDst>::type>::type()) TDstClean;
			TDstClean dst;
			typedef decltype(GetConvertCaller<TDstClean, TSrc>(dst, m_src.operator->())) TConvertCaller;
			TConvertCaller::Convert(dst, m_src.operator->());
			return dst;
		}
		template<typename TDst>
		inline void ConvertTo(TDst & dst) const
		{
			typedef decltype(GetConvertCaller<TDst, TSrc>(dst, m_src.operator->())) TConvertCaller;
			TConvertCaller::Convert(dst, m_src.operator->());
		}
		gcroot<TSrc> m_src;
	};

	template<typename TSrc>
	auto Convert(TSrc % src) -> typename std::enable_if<TestIsManaged<TSrc>::value, ConvertSrcHolderManaged<TSrc>>::type
	{
		return ConvertSrcHolderManaged<TSrc>(src);
	}

	template<typename TDst, typename TSrc>
	inline auto Convert(TSrc src) -> typename std::enable_if<TestIsManaged<TSrc>::value, TDst>::type
	{
		typedef PP_REMOVE_MANAGED_CONST(std::remove_const<std::remove_reference<TDst>::type>::type()) TDstClean;
		TDstClean dst;
		typedef decltype(GetConvertCaller<TDstClean, TSrc>(dst, src)) TConvertCaller;
		TConvertCaller::Convert(dst, src);
		return dst;
	}

#endif

	template<typename TSrc>
	auto Convert(TSrc && src) -> typename std::enable_if<TestIsNative<TSrc>::value, ConvertSrcHolderNative<TSrc>>::type
	{
		return ConvertSrcHolderNative<TSrc>(std::forward<TSrc>(src));
	}


	template<typename TDst, typename TSrc>
	inline auto Convert(TSrc && src) -> typename std::enable_if<TestIsNative<TSrc>::value, TDst>::type
	{
		typedef PP_REMOVE_MANAGED_CONST(std::remove_const<std::remove_reference<TDst>::type>::type()) TDstClean;
		TDstClean dst;
		typedef decltype(GetConvertCaller<TDstClean, TSrc>(dst, std::forward<TSrc>(src))) TConvertCaller;
		TConvertCaller::Convert(dst, std::forward<TSrc>(src));
		return dst;
	}

	template<typename TDst, typename TSrc>
	inline auto Convert(TDst & dst, TSrc && src) -> typename std::enable_if<BothTrue<TestIsNative<TDst>::value, TestIsNative<TSrc>::value>::value, TDst>::type &
	{
		typedef decltype(GetConvertCaller<TDst, TSrc>(dst, std::forward<TSrc>(src))) TConvertCaller;
		TConvertCaller::Convert(dst, std::forward<TSrc>(src));
		return dst;
	}
#ifdef _MANAGED
	template<typename TDst, typename TSrc>
	inline auto Convert(TDst % dst, TSrc && src) -> typename std::enable_if<BothTrue<TestIsManaged<TDst>::value, TestIsNative<TSrc>::value>::value, TDst>::type %
	{
		typedef decltype(GetConvertCaller<TDst, TSrc>(dst, std::forward<TSrc>(src))) TConvertCaller;
		TConvertCaller::Convert(dst, std::forward<TSrc>(src));
		return dst;	
	}
	template<typename TDst, typename TSrc>
	inline auto Convert(TDst & dst, TSrc src) -> typename std::enable_if<BothTrue<TestIsNative<TDst>::value, TestIsManaged<TSrc>::value>::value, TDst>::type &
	{
		typedef decltype(GetConvertCaller<TDst, TSrc>(dst, src)) TConvertCaller;
		TConvertCaller::Convert(dst, src);
		return dst;	
	}
	template<typename TDst, typename TSrc>
	inline auto Convert(TDst % dst, TSrc src) -> typename std::enable_if<BothTrue<TestIsManaged<TDst>::value, TestIsManaged<TSrc>::value>::value, TDst>::type %
	{
		typedef decltype(GetConvertCaller<TDst, TSrc>(dst, src)) TConvertCaller;
		TConvertCaller::Convert(dst, src);
		return dst;
	}
#endif
}

#define CONVERT_SINGLE_MEMBER_NATIVE(param, v) TypeConvert::Convert(rdst.v, src->v);
#define CONVERT_SINGLE_MEMBER_MANAGED(param, v) dst->v = TypeConvert::Convert(src.v);

#ifdef _MANAGED

#define CONVERT_MEMBERS(...)\
template<typename TDst, typename TSrc>\
static inline void ConvertImplementation(SafeObject<TDst> & dst, TSrc ^ src)\
{\
	if (src == nullptr)\
	{\
		dst.ReleaseInstance();\
		return;\
	}\
	TDst & rdst = dst.CreateInstance<>();\
	PP_FOR_EACH(CONVERT_SINGLE_MEMBER_NATIVE, "", __VA_ARGS__)\
}\
template<typename TDst, typename TSrc>\
static inline void ConvertImplementation(TDst & dst, TSrc ^ src)\
{\
	if (src == nullptr)\
	{\
		return;\
	}\
	TDst & rdst = dst;\
	PP_FOR_EACH(CONVERT_SINGLE_MEMBER_NATIVE, "", __VA_ARGS__)\
}\
template<typename TDst, typename TSrc>\
static inline void ConvertImplementation(TDst ^% dst, const SafeObject<TSrc> & src)\
{\
	if (!src)\
	{\
		dst = nullptr;\
		return;\
	}\
	ConvertImplementation(dst, *src);\
}\
template<typename TDst, typename TSrc>\
static inline void ConvertImplementation(TDst ^% dst, const TSrc & src)\
{\
	dst = gcnew TDst();\
	PP_FOR_EACH(CONVERT_SINGLE_MEMBER_MANAGED, "", __VA_ARGS__)\
}

#else 
#define CONVERT_MEMBERS(...)
#endif
