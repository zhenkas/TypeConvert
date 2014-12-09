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
#define CANCEL_FINALIZE(finalizerName)		finalizer_state_ ## finalizerName = EFinalizerState::FINALIZER_STATE_CANCEL

#define FINALIZE_0()						auto PP_CAT(finalizer_ , __LINE__) = ScopeFinalizerCreator() = [&]()
#define FINALIZE_1(finalizerName)			EFinalizerState PP_CAT(finalizer_state_ , finalizerName) = EFinalizerState::FINALIZER_STATE_READY; auto PP_CAT(finalizer_ , finalizerName) = ScopeFinalizerWithCancelCreator(PP_CAT(finalizer_state_ , finalizerName)) = [&]()

template<typename TLambda>
struct ScopeFinalizer
{
	inline ScopeFinalizer(TLambda & lambda) : m_lambda(lambda){}
	inline ScopeFinalizer(ScopeFinalizer<TLambda> & fin) : m_lambda(fin.m_lambda) {}
    inline ~ScopeFinalizer()    { m_lambda(); }
    TLambda & m_lambda;
};

struct ScopeFinalizerCreator
{
	template<typename TLambda>
	inline ScopeFinalizer<TLambda> operator = (TLambda & lambda) {
		return ScopeFinalizer<TLambda>(lambda);
	}
};

enum EFinalizerState: unsigned char {
	FINALIZER_STATE_READY,
	FINALIZER_STATE_CANCEL,
	FINALIZER_STATE_FINALIZED,
};

template<typename TLambda>
struct ScopeFinalizerWithCancel
{
	inline ScopeFinalizerWithCancel(TLambda & lambda, EFinalizerState & state) : m_lambda(lambda), m_state(state) {}
	inline ScopeFinalizerWithCancel(ScopeFinalizerWithCancel<TLambda> & fin) : m_lambda(fin.m_lambda), m_state(fin.m_state) {}
	inline ~ScopeFinalizerWithCancel()    { 
		if (m_state == EFinalizerState::FINALIZER_STATE_READY) {
			m_lambda();
			m_state = EFinalizerState::FINALIZER_STATE_FINALIZED;
		}
	}
	TLambda & m_lambda;
	EFinalizerState & m_state;
};


struct ScopeFinalizerWithCancelCreator
{
	ScopeFinalizerWithCancelCreator(EFinalizerState && _state): m_state(_state) {}
    template<typename TLambda>
    inline ScopeFinalizerWithCancel<TLambda> operator = (TLambda & lambda) {
        return ScopeFinalizerWithCancel<TLambda>(lambda, m_state);
    }
	EFinalizerState & m_state;
};

