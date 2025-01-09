#pragma once











AlifInterpreter* alifInterpreter_get(); // 26



AlifThread* alifThread_get(); // 60






/* ----------------------------------------------------------------------------------------------------------- */



class AlifErrStackItem { // 32
public:
	AlifObject* excValue{};
	AlifErrStackItem* previousItem{};
};



class AlifStackChunk { // 52
public:
	AlifStackChunk* previous{};
	AlifUSizeT size{};
	AlifUSizeT top{};
	AlifObject* data[1]{}; /* Variable sized */
};



class AlifThread { // 59
public:
	AlifThread* prev{};
	AlifThread* next{};
	AlifInterpreter* interpreter{};

	uintptr_t evalBreaker{};

	class {
	public:
		AlifUIntT initialized : 1;

		AlifUIntT bound : 1;
		AlifUIntT boundGILState : 1;
		AlifUIntT active : 1;

		AlifUIntT finalizing : 1;
		AlifUIntT cleared : 1;
		AlifUIntT finalized : 1;
		AlifUIntT holdsGIL : 1;
		/* padding to align to 4 bytes */
		AlifUIntT : 4;
	} status{};

	AlifIntT state{};

	class AlifInterpreterFrame* currentFrame{};

	AlifIntT alifRecursionRemaining{};
	AlifIntT alifRecursionLimit{};

	AlifIntT cppRecursionRemaining{};
	AlifIntT recursionHeadroom{}; /* Allow 50 more calls to handle any errors. */

	AlifIntT tracing{};


	AlifObject* currentException{};

	AlifErrStackItem* excInfo{};



	AlifUSizeT threadID{};

	unsigned long nativeThreadID{};

	AlifObject* deleteLater{};

	uintptr_t criticalSection{};

	AlifSizeT id{};

	AlifStackChunk* dataStackChunk{};
	AlifObject** dataStackTop{};
	AlifObject** dataStackLimit{};
	uint64_t dictGlobalVersion{};

};





extern void alifThread_detach(AlifThread*); // 157





#define ALIFCPP_RECURSION_LIMIT 3000 // 214



AlifInterpreter* alifInterpreter_head(); // 263

typedef AlifObject* (*AlifFrameEvalFunction)(AlifThread*,
	AlifInterpreterFrame*, AlifIntT); // 271
