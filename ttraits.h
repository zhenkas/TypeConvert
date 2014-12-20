#pragma once
#include <type_traits>
#include "pp_foreach.h"

#define TRAITS_INNER_TYPE(T, InnerTypeName) Traits::InnerType_ ## InnerTypeName<T>
#define TRAITS_INNER_TEMPLATE(T, O, InnerTemplate) Traits::InnerTemplate_ ## InnerTemplate<T, O>

#define DECLTYPE_TRACE(...) decltype(Traits::TraceType<##__VA_ARGS__>::ftype())

#ifndef __CUDACC__
#define TRAITS_DECLARE_INNER_TYPE(InnerTypeName)\
	template<typename TClass, typename TSearchType = Traits::TraceType<std::true_type>>\
	struct has_type_ ## InnerTypeName\
	{\
		template<typename U>\
		static auto Test(Traits::TraceType<typename U::InnerTypeName> *) -> typename U::InnerTypeName;\
		template<typename U>\
		static Traits::TraceType<std::false_type> Test(...);\
		typedef decltype(Test<std::remove_reference<TClass>::type>(0)) type;\
		static const bool value = (std::is_same<Traits::TraceType<std::true_type>, TSearchType>::value && !std::is_same<Traits::TraceType<std::false_type>, type>::value) || std::is_same<type, TSearchType>::value;\
	};

#else
#define TRAITS_DECLARE_INNER_TYPE(InnerTypeName)\
	template<typename T>\
	struct has_type_ ## InnerTypeName\
	{\
		template<class V> struct T_Check\
		{ 	typedef typename V type; };\
\
		template<typename U> static auto Check (T_Check<typename U::InnerTypeName> * ) -> typename U::InnerTypeName;\
		template<class U> static int Check (...);\
		\
		static const bool value = !std::is_same<decltype(Check<T>(0)), int>::value;\
		typedef typename std::conditional<InnerType_ ## InnerTypeName<T>::exists, decltype(Check<T>(0)), T>::type type;\
	};
#endif

#ifndef __CUDACC__
#define TRAITS_DECLARE_INNER_TEMPLATE(InnerTemplateName)\
	template<typename T, typename O>\
	struct InnerTemplate_ ## InnerTemplateName\
	{\
		template<typename U1, template<class U> class V> struct T_Check\
		{ 	typedef typename V<O> type; };\
		\
		template<class U> static auto Check (typename T_Check<U, typename U::InnerTemplateName>::type * v) -> typename std::remove_reference<decltype(*v)>::type;\
		template<class U> static void Check (...);\
		\
		static const bool exists = !std::is_same<decltype(Check<T>(0)), void>::value;\
		typedef typename std::conditional<InnerTemplate_ ## InnerTemplateName<T, O>::exists, decltype(Check<T>(0)), T>::type type;\
	};
#else
#define TRAITS_DECLARE_INNER_TEMPLATE(InnerTemplateName)
#endif
namespace Traits
{
	template <typename T>
	struct TraceType{
		static auto ftype() -> T &;
		typedef typename T type;
	};

	template <typename T>
	struct normalize_type {
		typedef typename std::add_reference<typename std::add_const<T>::type>::type type; 	
	};	

	template <bool _exists, typename TBase>
	struct func_info
	{
		static const bool exists = _exists;
		typedef typename std::conditional<exists, TBase, void>::type base;
	};
#define TRAITS_DECLARE_HAS_FUNC(funcName)\
	template<typename TClassCheck, typename TRet, typename ...TParams>\
	struct has_func_ ## funcName\
	{\
		typedef typename std::remove_reference<TClassCheck>::type TClass;\
		template<typename ...T>\
		auto operator () (T&&... params)-> typename std::integral_constant<bool, std::is_same<decltype(((TClass *)0)->funcName(params...)), TRet>::value>;\
		auto operator () (...)->std::false_type;\
		typedef TRet(TClass::*TSearchFunc)(TParams...);\
		template<typename U>\
		static auto Test(decltype(&U::funcName) *) -> Traits::func_info<std::is_same<decltype(&U::funcName), TSearchFunc>::value, typename Traits::get_function_base_type<decltype(&U::funcName)>::type>;\
		template<typename U>\
		static auto Test(...) -> Traits::func_info<false, void>;\
		typedef decltype(Test<TClass>(0)) func_info_type;\
		static const bool value = func_info_type::exists;\
		typedef typename func_info_type::base base;\
	};

	template <typename TFunc>
	struct get_function_base_type
	{
		template<typename TClass, typename TRet, typename ...TParams>
		static auto TestType(TRet(TClass::*)(TParams...)) -> Traits::TraceType<TClass>;
 		static auto TestType(...) -> Traits::TraceType<void>;
		typedef typename decltype(TestType((TFunc)NULL)) traced_type;
		typedef typename traced_type::type type;
	};

	// define inside struct or class to get the "typeName &" typedef represents the type of this
 	#define TRAITS_DECLARE_THIS_TYPE(typeName)\
 	void GetTypeOfThis_ ## typeName();\
 	typedef DECLTYPE_TRACE(Traits::get_function_base_type<decltype(&GetTypeOfThis_ ## typeName)>::type) typeName;


	template<typename T1, typename TCheck, typename T2>
	struct if_not_equal
	{
		typedef typename std::conditional<!std::is_same<T1, TCheck>::value, T1, T2>::type type;
	};

	template<typename T, typename P>
	struct clone_struct: T
	{
		typedef P type;
		inline clone_struct(const clone_struct<T,P> & t) : T((const T &)t) {}
		template<typename O>
		inline clone_struct(const O & o): T(o) {}
		template<typename O>
		inline auto operator= (const O & o) -> decltype(((T &)(*this)) = o) {
			return ((T &)(*this)) = o;
		}
	};

	template<typename T>
	struct inherit_public: public T {};

	template<typename T>
	struct inherit_protected : protected T {};

	/*
		any_of - usage any_of<T1, T2...> up to x types
		If same function would accept 2 or more specific types when declaration should be
		void Func(any_of<T1, T2, ...>, TParam1 param1, TParam2 param2, ...)
	*/

	#define DECLARE_ANY_OF_TEMPLATE(param, index) typename T ## index = TAnyOf_None<index>,
	#define DECLARE_ANY_OF_COPYCONSTRUCTOR(param, index) any_of(T ## index){}
	#define DECLARE_ANY_OF(...)\
	template <int>\
	struct TAnyOf_None {}; \
	\
	template <PP_FOR_EACH(DECLARE_ANY_OF_TEMPLATE, NULL, __VA_ARGS__) typename bool = false>\
	struct any_of\
	{\
	PP_FOR_EACH(DECLARE_ANY_OF_COPYCONSTRUCTOR, NULL, __VA_ARGS__)\
	};
	DECLARE_ANY_OF(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);


	/* t_for_each_i - unwind for loop

		Usage:
			t_for_each_i<count>([&](int i) { todo(i); });
	*/

	template<int count, int start_i = 0>
	struct t_for_each_i;

#define PP_CALLFUNC_I(p, i) func(p + i - 1);
#define PP_DECLARE_T_FOREACH_I(count)\
	template<int start_i>\
	struct t_for_each_i< ## count ##, start_i>\
	{\
		template<typename TFunc>\
		inline t_for_each_i(const TFunc & func)\
		{\
			PP_FOR_I(PP_CALLFUNC_I, start_i, count)\
		}\
		template<typename TFunc>\
		inline t_for_each_i(const TFunc & func, int start_ii)\
		{\
			PP_FOR_I(PP_CALLFUNC_I, start_ii, count)\
		}\
	};

	PP_DECLARE_T_FOREACH_I(0);
	PP_DECLARE_T_FOREACH_I(1);
	PP_DECLARE_T_FOREACH_I(2);
	PP_DECLARE_T_FOREACH_I(3);
	PP_DECLARE_T_FOREACH_I(4);
	PP_DECLARE_T_FOREACH_I(5);
	PP_DECLARE_T_FOREACH_I(6);
	PP_DECLARE_T_FOREACH_I(7);
	PP_DECLARE_T_FOREACH_I(8);
	PP_DECLARE_T_FOREACH_I(9);
	PP_DECLARE_T_FOREACH_I(10);
	PP_DECLARE_T_FOREACH_I(11);
	PP_DECLARE_T_FOREACH_I(12);
	PP_DECLARE_T_FOREACH_I(13);
	PP_DECLARE_T_FOREACH_I(14);
	PP_DECLARE_T_FOREACH_I(15);
	PP_DECLARE_T_FOREACH_I(16);
	PP_DECLARE_T_FOREACH_I(17);
	PP_DECLARE_T_FOREACH_I(18);
	PP_DECLARE_T_FOREACH_I(19);
	PP_DECLARE_T_FOREACH_I(20);
	PP_DECLARE_T_FOREACH_I(21);
	PP_DECLARE_T_FOREACH_I(22);
	PP_DECLARE_T_FOREACH_I(23);
	PP_DECLARE_T_FOREACH_I(24);
	PP_DECLARE_T_FOREACH_I(25);
	PP_DECLARE_T_FOREACH_I(26);
	PP_DECLARE_T_FOREACH_I(27);
	PP_DECLARE_T_FOREACH_I(28);
	PP_DECLARE_T_FOREACH_I(29);
	PP_DECLARE_T_FOREACH_I(30);
	PP_DECLARE_T_FOREACH_I(31);
	PP_DECLARE_T_FOREACH_I(32);

	template<int count, int start_i>
	struct t_for_each_i
	{
		template<typename TFunc>
		inline t_for_each_i(const TFunc & func)
		{
			Traits::t_for_each_i<count / 2, start_i> (func) , Traits::t_for_each_i<count / 2 + count % 2, start_i + count / 2> (func);
		}
		template<typename TFunc>
		inline t_for_each_i(const TFunc & func, int start_ii)
		{
			Traits::t_for_each_i<count / 2>(func, start_ii), 
			Traits::t_for_each_i<count / 2 + count % 2>(func, start_ii + count / 2);
		}
	};


	template<unsigned long N>
	struct bin {
		static const int value = (N % 16) + 2 * bin<N / 16>::value;
	};

	template<>
	struct bin<0> {
		static const int value = 0;
	};

#ifdef _MANAGED
	template<typename C>
	auto RemoveManagedConst(const C ^ const %)->C ^;
	template<typename C>
	auto RemoveManagedConst(const C %)->C;

	#define PP_REMOVE_MANAGED_CONST(...) decltype(Traits::RemoveManagedConst(__VA_ARGS__))
#else
	#define PP_REMOVE_MANAGED_CONST(...) decltype(__VA_ARGS__)
#endif

	template<typename T>
	struct TestIsNative
	{
#ifdef _MANAGED
		typedef typename std::remove_const<typename std::remove_reference<T>::type>::type type;
		template<typename O> static auto CheckNative(O % t, decltype(t.GetType()))->std::false_type;
		template<typename O> static auto CheckNative(O ^% t, decltype(t->GetType()))->std::false_type;
		static auto CheckNative(...)->std::true_type;
		static const bool value = decltype(CheckNative(*cli::interior_ptr<type>(nullptr), System::Type::typeid))::value;
#else
		static const bool value = true;
#endif
	};
	template<typename T>
	struct TestIsManaged
	{
		const static bool value = !TestIsNative<T>::value;
	};

	template<bool b1, bool b2>
	struct BothTrue
	{
		const static bool value = b1 & b2;
	};
	
} //namespace Traits
