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
#include "pp_foreach.h"
#include "SafeObject.h"

#ifdef _MANAGED
#define REF_SYMBOL %
#else
#define REF_SYMBOL &
#endif

/*
	Usage:

	Declare new conversion in TypeConvert namespace

		namespace TypeConvert
		{
			inline void ConvertImplementation(TDestination REF_SYMBOL dst, const TSource REF_SYMBOL src)
			{
				dst = src;
			}
		}

	Declare new conversion inside source or destination struct/class type

		struct TDestination/TSource
		{
		public:
			inline static void ConvertImplementation(TDestination REF_SYMBOL dst, const TSource REF_SYMBOL src)
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
	template<typename T>
	struct ConvertCallerFromType
	{
		template<typename TDst, typename TSrc>
		static inline void Convert(TDst REF_SYMBOL dst, const TSrc REF_SYMBOL src) 
		{
			T::ConvertImplementation(dst, src);		
		}
	};

	struct ConvertCallerFromNamespace
	{
		template<typename TDst, typename TSrc>
		static inline void Convert(TDst REF_SYMBOL dst, const TSrc REF_SYMBOL src)
		{
			TypeConvert::ConvertImplementation(dst, src);
		}
	};

 	template<typename TDst, typename TSrc>
	auto GetConvertCaller(TDst REF_SYMBOL dst, const TSrc REF_SYMBOL src, decltype(((TDst*)0)->ConvertImplementation(dst, src)) *)-> ConvertCallerFromType<TDst>;
	template<typename TDst, typename TSrc>
	auto GetConvertCaller(TDst REF_SYMBOL dst, const TSrc REF_SYMBOL src, decltype(((TSrc*)0)->ConvertImplementation(dst, src)) *)->ConvertCallerFromType<TSrc>;
	template<typename TDst, typename TSrc>
	auto GetConvertCaller(TDst REF_SYMBOL dst, const TSrc REF_SYMBOL src, ...)->ConvertCallerFromNamespace;

	template<typename TSrc>
	struct ConvertSrcHolder
	{
		inline ConvertSrcHolder(const TSrc & src):m_src(src) {}
		template<typename TDst>
		inline operator const TDst () const
		{
			PP_REMOVE_MANAGED_CONST(std::remove_const<std::remove_reference<TDst>::type>::type()) dst;
			typedef decltype(GetConvertCaller(dst, m_src, 0)) TConvertCaller;
			TConvertCaller::Convert(dst, m_src);
			return dst;
		}
		template<typename TDst>
		inline void ConvertTo(TDst & dst) const
		{
			typedef decltype(GetConvertCaller(dst, m_src, 0)) TConvertCaller;
			TConvertCaller::Convert(dst, m_src);
		}
		const TSrc & m_src;
	};

#ifdef _MANAGED
	template<typename TSrc>
	value struct ConvertSrcHolderManaged
	{
		inline ConvertSrcHolderManaged(const TSrc ^ const % src):	m_src(src) {}
		const TSrc ^ m_src;
	};
	template<typename TSrc>
	inline auto ConvertWithHolder(const TSrc ^ const % src) -> ConvertSrcHolder<ConvertSrcHolderManaged<TSrc>>
	{
		return ConvertSrcHolder<ConvertSrcHolderManaged<TSrc>>(ConvertSrcHolderManaged<TSrc>(src));
	}
	template<typename TDst, typename TSrc>
	inline void ConvertImplementation(TDst % dst, const ConvertSrcHolderManaged<TSrc> % src)
	{
		Convert(dst, src.m_src);
	}
#endif
	template<typename TSrc>
	inline auto ConvertWithHolder(const TSrc & src) -> ConvertSrcHolder<TSrc>
	{
		return ConvertSrcHolder<TSrc>(src);
	}
	template<typename TSrc>
	inline auto Convert(const TSrc & src) -> decltype(ConvertWithHolder(src))
	{
		return ConvertWithHolder(src);
	}

	template<typename TDst, typename TSrc>
	inline TDst Convert(const TSrc REF_SYMBOL src)
	{
		PP_REMOVE_MANAGED_CONST(std::remove_const<std::remove_reference<TDst>::type>::type()) dst;
		typedef decltype(GetConvertCaller(dst, src, 0)) TConvertCaller;
		TConvertCaller::Convert(dst, src);		
		return dst;
	}
	template<typename TDst, typename TSrc>
	inline auto Convert(TDst REF_SYMBOL dst, const TSrc REF_SYMBOL src) -> TDst REF_SYMBOL
	{
		typedef decltype(GetConvertCaller(dst, src, 0)) TConvertCaller;
		TConvertCaller::Convert(dst, src);
		return dst;
	}

	template<typename TDst, typename TSrc>
	inline auto operator << (TDst REF_SYMBOL dst, const TSrc REF_SYMBOL src) -> TDst REF_SYMBOL
	{
		typedef decltype(GetConvertCaller(dst, src, 0)) TConvertCaller;
		TConvertCaller::Convert(dst, src);
		return dst;
	};
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


