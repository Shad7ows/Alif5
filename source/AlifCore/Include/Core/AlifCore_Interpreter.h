#pragma once

#include "AlifCore_EvalState.h"
#include "AlifCore_ThreadState.h"


class AlifInterpreter { // 95
public:

	AlifEval eval{};

	AlifInterpreter* next{};

	AlifIntT id_{};

	AlifIntT initialized{};
	AlifIntT ready{};
	AlifIntT finalizing{};

	class AlifThreads {
	public:
		AlifUSizeT nextUniquID{};
		AlifThread* head{};
		AlifThread* main{};
		AlifUSizeT count{};

		AlifSizeT stackSize{};
	} threads;

	class AlifDureRun* dureRun{};
	AlifConfig config{};

	//AlifFrameEvalFunction evalFrame{};

	//AlifGCDureRun gc{};

	AlifObject* builtins{};

	//class ImportState imports;

	AlifMemory* memory_{};

	//AlifObjectState objectState{};

	//class TypesState types;

	AlifThreadImpl initialThread{};
};




extern const AlifConfig* alifInterpreter_getConfig(AlifInterpreter*); // 329



AlifIntT alifInterpreter_new(AlifThread*, AlifInterpreter**); // 399
