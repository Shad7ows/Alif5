#pragma once




class AlifInterpreter { // 95
public:

	//CEval cEval;

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

	//AlifObject* builtins{};

	//class ImportState imports;

	AlifMemory* memory_{};

	//AlifObjectState objectState{};

	//class TypesState types;

	AlifThreadImpl initialThread{};
};





AlifIntT alifInterpreter_new(AlifThread*, AlifInterpreter**); // 399
