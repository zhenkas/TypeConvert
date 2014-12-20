#pragma once
#include "pp_foreach.h"

#define PP_TO_STR(a) #a

#define PP_GET_TYPED_VAR(T) (*(std::add_pointer<TParam1>::type)0)

#ifdef _DEBUG
#define PP_DEBUG_ONLY(...) __VA_ARGS__
#else
#define PP_DEBUG_ONLY(...)
#endif

#define PP_DECLARE_ENUM_BITMASK_OPERATORS(enumName)\
	inline enumName operator | (const enumName & symType1, const enumName & symType2) {\
		return enumName(DWORD(symType1) | DWORD(symType2));\
	}\
	inline enumName operator & (const enumName & symType1, const enumName & symType2) {\
		return enumName(DWORD(symType1) & DWORD(symType2));\
	}\
	inline enumName operator ^ (const enumName & symType1, const enumName & symType2) {\
		return enumName(DWORD(symType1) ^ DWORD(symType2));\
	}\
	inline enumName & operator |= (enumName & symType1, const enumName & symType2) {\
		symType1 = enumName(DWORD(symType1) | DWORD(symType2));\
		return symType1;\
	}\
	inline enumName operator &= (enumName & symType1, const enumName & symType2) {\
		symType1 = enumName(DWORD(symType1) & DWORD(symType2));\
		return symType1;\
	}\
	inline enumName operator ^= (enumName & symType1, const enumName & symType2) {\
		symType1 = enumName(DWORD(symType1) ^ DWORD(symType2));\
		return symType1;\
	}
