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

//Use FINALIZE() to force the execution on scope exit
//Use FINALIZE(finalizeName) to force the execution on scope exit with ability to cancel it by CANCEL_FINALIZE(finalizeName)
#define FINALIZE(...)						PP_CAT (FINALIZE_, PP_NUM_ARGS(__VA_ARGS__) (##__VA_ARGS__))

#define FINALIZE_0()						ScopeFinalizerCreator PP_CAT(finalizer_ , __LINE__) = [&]()
#define FINALIZE_1(finalizerName)			ScopeFinalizerWithCancelCreator PP_CAT(finalizer_ , finalizerName) = [&]()
#define CANCEL_FINALIZE(finalizerName)		\
	static_assert(std::is_same<EFinalizerState, decltype(PP_CAT(finalizer_ , finalizerName).m_state)>::value, "The finalizer is not declared in this scope");\
	PP_CAT(finalizer_ , finalizerName).m_state = EFinalizerState::FINALIZER_STATE_CANCEL

template<typename TLambda>
void CallFinalizeStatic(void * lambda)
{
	(*reinterpret_cast<TLambda *>(lambda))();
}

typedef void (*TCallFinalizeStatic)(void * lambda);

struct ScopeFinalizerCreator
{
	template<typename TLambda>
	inline ScopeFinalizerCreator(TLambda && lambda) 
		: m_callLambda(CallFinalizeStatic<TLambda>)
		, m_lambda(reinterpret_cast<void *>(&lambda)) 
	{}
	inline ~ScopeFinalizerCreator()
	{
		m_callLambda(m_lambda);
	}
	TCallFinalizeStatic m_callLambda;
	void * m_lambda;
};

enum EFinalizerState: unsigned char {
	FINALIZER_STATE_READY,
	FINALIZER_STATE_CANCEL,
	FINALIZER_STATE_FINALIZED,
};

struct ScopeFinalizerWithCancelCreator
{
	template<typename TLambda>
	inline ScopeFinalizerWithCancelCreator(TLambda && lambda) 
		: m_state(EFinalizerState::FINALIZER_STATE_READY)
		, m_callLambda(CallFinalizeStatic<TLambda>)
		, m_lambda(reinterpret_cast<void *>(&lambda))
	{}
	inline ~ScopeFinalizerWithCancelCreator()
	{
		if (m_state == EFinalizerState::FINALIZER_STATE_READY)
			m_callLambda(m_lambda);
	}
	TCallFinalizeStatic m_callLambda;
	void * m_lambda;
	EFinalizerState m_state;
};

