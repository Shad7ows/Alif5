#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Backoff.h"
#include "AlifCore_Call.h"
#include "AlifCore_AlifEval.h"
#include "AlifCore_Code.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpCodeData.h"
#include "AlifCore_OpCodeUtils.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_Function.h"
#include "AlifCore_Instruments.h"
#include "AlifCore_Integer.h"
#include "AlifCore_Tuple.h"
#include "AlifCore_Frame.h"
#include "AlifCore_Dict.h"
#include "AlifCore_StackRef.h"
#include "DictObject.h"
#include "OpCode.h"
#include "AlifCore_Code.h"
#include "AlifEvalMacros.h"


AlifIntT alif_checkRecursiveCall(AlifThread* _thread, const wchar_t* _where)
{
#ifdef USE_STACKCHECK
	if (alifOS_checkStack()) {
		++_thread->recursionRemaining;
		alifErr_setString(_thread, alifExcMemoryError, "Stack overflow");
		return -1;
	}
#endif
	if (_thread->recursionHeadroom) {
		if (_thread->recursionRemaining < -50) {
			//alif_fatalError("Cannot recover from stack overflow.");
			return -1; // 
		}
	}
	else {
		if (_thread->recursionRemaining <= 0) {
			_thread->recursionHeadroom++;
			//alifErr_format(_thread, alifExcRecursionError,
			//	"maximum recursion depth exceeded%s", _where);
			_thread->recursionHeadroom--;
			++_thread->recursionRemaining;
			return -1;
		}
	}
	return 0;
}

const BinaryFunc alifEvalBinaryOps[] = { 
	alifNumber_add, // [NB_ADD]
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	alifNumber_subtract, // [NB_SUBTRACT]
	0,
	0,
	alifNumber_inPlaceAdd, // [NB_INPLACE_ADD]
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	alifNumber_inPlaceSubtract, // [NB_INPLACE_SUBTRACT]
};

AlifObject* alifEval_evalCode(AlifObject* _co, AlifObject* _globals, AlifObject* _locals) { 
	AlifThread* thread = alifThread_get();
	if (_locals == nullptr) {
		_locals = _globals;
	}
	AlifObject* builtins = alifEval_builtinsFromGlobals(thread, _globals);
	if (builtins == nullptr) return nullptr;

	AlifFrameConstructor desc = {
		_globals,
		builtins,
		((AlifCodeObject*)_co)->name,
		((AlifCodeObject*)_co)->name,
		_co,
		nullptr,
		nullptr,
		nullptr
	};
	AlifFunctionObject* func = alifFunction_fromConstructor(&desc); // need review
	if (func == nullptr) return nullptr;

	AlifObject* res = alifEval_vector(thread, func, _locals, nullptr, 0, nullptr);
	ALIF_DECREF(func);
	return res;
}

/*
	تحتاج إلا مراجعة
	فقد تم إسناد قيم ثنائية نظرا الى
	أن المصفوفة في cpp
	لا تقبل تهيئة العناصر بشكل مفرد
	وبالاخص اذا كان العنصر الذي يتم تهيئته هو union
*/
static const AlifCodeUnit alifInterpreterTrampolineInstructions[] = {
	0b0000000000011110, // { NOP, 0},
	0b0000000000010110, // {INTERPRETER_EXIT, 0 },
	0b0000000000011110, // {NOP, 0 },
	0b0000000000010110, // {INTERPRETER_EXIT, 0 },
	0b0000010010010101 // {RESUME, RESUME_OPARG_DEPTH1_MASK | RESUME_AT_FUNC_START }
};

#define ALIFEVAL_CSTACK_UNITS 2

AlifObject* alifEval_evalFrameDefault(AlifThread* _thread,
	AlifInterpreterFrame* _frame, AlifIntT _throwFlag) { 

	uint16_t opCode{};
	AlifIntT opArg{};

	AlifInterpreterFrame entryFrame{};

	entryFrame.executable = ALIF_NONE;
	entryFrame.instrPtr = (AlifCodeUnit*)alifInterpreterTrampolineInstructions + 1;
	entryFrame.stacktop = 0;
	entryFrame.owner = FrameOwner::FRAME_OWNED_BY_CSTACK;
	entryFrame.returnOffset = 0;
	entryFrame.previous = _thread->currentFrame;
	_frame->previous = &entryFrame;
	_thread->currentFrame = _frame;

	_thread->recursionRemaining -= (ALIFEVAL_CSTACK_UNITS - 1);


	AlifCodeUnit* nextInstr{};
	AlifObject** stackPtr{};

startFrame:

	nextInstr = _frame->instrPtr;
	AlifCodeUnit buffer[20];
	memcpy(buffer, _frame->instrPtr, 20);
resumeFrame:
	stackPtr = alifFrame_getStackPointer(_frame);
	

	DISPATCH();

dispatchOpCode:
	switch (opCode)
	{
//#include "OpCodeCases.h"


	TARGET(RESUME) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		AlifCodeUnit* thisInstr = nextInstr - 1;
		if (_thread->tracing == 0) {
			AlifCodeObject* code = alifFrame_getCode(_frame);
			if (thisInstr->op.code == RESUME) {
#if ENABLE_SPECIALIZATION
				thisInstr->op.code = RESUME_CHECK;
#endif  /* ENABLE_SPECIALIZATION */
			}
		}
		DISPATCH();
	}
	TARGET(RESERVED) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		//INSTRUCTION_STATS(RESERVED);
		//Py_FatalError("Executing RESERVED instruction.");
		DISPATCH();
	}
	TARGET(BUILD_MAP) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		//INSTRUCTION_STATS(BUILD_MAP);
		AlifStackRef* values{};
		AlifStackRef map{};
		values->bits = (uintptr_t)& stackPtr[-opArg * 2];
		STACKREFS_TO_ALIFOBJECTS(values, opArg * 2, values_o);
		if ((values_o)) {
			for (int _i = opArg * 2; --_i >= 0;) {
				ALIFSTACKREF_CLOSE(values[_i]);
			}
			if (true) {
				stackPtr += -opArg * 2;
				//goto error;
				exit(-1);

			}
		}
		AlifObject* map_o = alifDict_fromItems(
			values_o, 2,
			values_o + 1, 2,
			opArg);
		//STACKREFS_TO_PYOBJECTS_CLEANUP(values_o);
		for (int _i = opArg * 2; --_i >= 0;) {
			ALIFSTACKREF_CLOSE(values[_i]);
		}
		if (map_o == NULL) {
			stackPtr += -opArg * 2;
			//goto error;
			exit(-1);

		}
		map.bits = ALIFSTACKREF_FROMALIFOBJECTSTEAL(map_o);
		stackPtr[-opArg * 2] = (AlifObject*)map.bits;
		stackPtr += 1 - opArg * 2;
		DISPATCH();
	}
	TARGET(LOAD_CONST) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		AlifObject* value;
		value = GETITEM(FRAME_CO_CONSTS, opArg);
		ALIF_INCREF(value);
		stackPtr[0] = value;
		stackPtr += 1;
		DISPATCH();
	}
	TARGET(LOAD_NAME) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		AlifObject* v{};
		AlifObject* modOrClassDict = LOCALS();
		if (modOrClassDict == nullptr) {
			// error
			exit(-1);

			//goto error;
		}
		AlifObject* name = GETITEM(FRAME_CO_NAMES, opArg);
		if (alifMapping_getOptionalItem(modOrClassDict, name, &v) < 0) {
			//goto error;
			exit(-1);

		}
		if (v == nullptr) {
			if (alifDict_getItemRef(GLOBALS(), name, &v) < 0) {
				//goto error;
				exit(-1);

			}
			if (v == nullptr) {
				if (alifMapping_getOptionalItem(BUILTINS(), name, &v) < 0) {
					//goto error;
					exit(-1);

				}
				//if (v == nullptr) {
				//	alifEval_formatExcCheckArg(_thread, alifExcNameError, NAME_ERROR_MSG, name);
				//	goto error;
				exit(-1);

				//}
			}
		}
		stackPtr[0] = v;
		stackPtr += 1;
		DISPATCH();
	}
	TARGET(STORE_GLOBAL) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		//INSTRUCTION_STATS(STORE_GLOBAL);
		AlifStackRef v;
		v.bits = (uintptr_t)stackPtr[-1];
		AlifObject* name = GETITEM(FRAME_CO_NAMES, opArg);
		int err = alifDict_setItem(GLOBALS(), name, ALIFSTACKREF_ASALIFOBJECTBORROW(v));
		ALIFSTACKREF_CLOSE(v);
		if (err) {
			exit(-1);
		}// goto pop_1_error;
		stackPtr += -1;
		DISPATCH();
	}
	TARGET(STORE_NAME) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		AlifObject* v{};
		v = stackPtr[-1];
		AlifObject* name = GETITEM(FRAME_CO_NAMES, opArg);
		AlifObject* ns = LOCALS();
		int err;
		if (ns == nullptr) {
			// error
			exit(-1);

			ALIF_DECREF(v);
			//if (true) goto pop_1_error;
		}
		if (ALIFDICT_CHECKEXACT(ns))
			err = alifDict_setItem(ns, name, v);
		else
			err = alifObject_setItem(ns, name, v);
		ALIF_DECREF(v);
		if (err) /*goto pop_1_error*/return nullptr;
		stackPtr -= 1;
		DISPATCH();
	}
	TARGET(BINARY_OP) {
		_frame->instrPtr = nextInstr;
		//nextInstr += 2; // need review !important
		//AlifCodeUnit* thisInstr = nextInstr - 2; // need review !important
		PREDICTED(BINARY_OP);
		nextInstr += 1;
		AlifCodeUnit* thisInstr = nextInstr - 1;
		AlifObject* rhs{};
		AlifObject* lhs{};
		AlifObject* res{};
		// _SPECIALIZE_BINARY_OP
		rhs = stackPtr[-1];
		lhs = stackPtr[-2];
//		{
//			uint16_t counter = read_u16(&thisInstr[1].cache);
//#if ENABLE_SPECIALIZATION
//			if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
//				//nextInstr = thisInstr;
//				//alifSpecialize_binaryOp(lhs, rhs, nextInstr, opArg, LOCALS_ARRAY);
//				//DISPATCH_SAME_OPARG();
//			}
//			ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
//#endif  /* ENABLE_SPECIALIZATION */
//		}
		// _BINARY_OP
		{
			res = alifEvalBinaryOps[opArg](lhs, rhs);
			ALIF_DECREF(lhs);
			ALIF_DECREF(rhs);
			if (res == nullptr) return nullptr /*goto pop_2_error*/;
		}
		stackPtr[-2] = res;
		stackPtr -= 1;
		DISPATCH();
	}
	TARGET(BINARY_OP_ADD_FLOAT) {
		_frame->instrPtr = nextInstr;
		nextInstr += 2;
		//INSTRUCTION_STATS(BINARY_OP_ADD_FLOAT);
		AlifStackRef left;
		AlifStackRef right;
		AlifStackRef res;
		// _GUARD_BOTH_FLOAT
		right.bits = (uintptr_t)stackPtr[-1];
		left.bits = (uintptr_t)stackPtr[-2];
		{
			AlifObject* left_o = ALIFSTACKREF_ASALIFOBJECTBORROW(left);
			AlifObject* right_o = ALIFSTACKREF_ASALIFOBJECTBORROW(right);
			DEOPT_IF(!ALIFFLOAT_CHECKEXACT(left_o), BINARY_OP);
			DEOPT_IF(!ALIFFLOAT_CHECKEXACT(right_o), BINARY_OP);
		}
		/* Skip 1 cache entry */
		// _BINARY_OP_ADD_FLOAT
		{
			AlifObject* left_o = ALIFSTACKREF_ASALIFOBJECTBORROW(left);
			AlifObject* right_o = ALIFSTACKREF_ASALIFOBJECTBORROW(right);
			//STAT_INC(BINARY_OP, hit);
			double dres =
				((AlifFloatObject*)left_o)->digits_ +
				((AlifFloatObject*)right_o)->digits_;
			AlifObject* res_o;
			DECREF_INPUTS_AND_REUSE_FLOAT(left_o, right_o, dres, res_o);
			res.bits = ALIFSTACKREF_FROMALIFOBJECTSTEAL(res_o);
		}
		stackPtr[-2] = (AlifObject*)res.bits;
		stackPtr += -1;
		DISPATCH();
	}
	TARGET(PUSH_NULL) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		AlifObject* res{};
		stackPtr[0] = res;
		stackPtr += 1;
		DISPATCH();
	}
	TARGET(MAKE_FUNCTION) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		//INSTRUCTION_STATS(MAKE_FUNCTION);
		AlifStackRef codeobj_st;
		AlifStackRef func;
		codeobj_st.bits = (uintptr_t)stackPtr[-1];
		AlifObject* codeobj = ALIFSTACKREF_ASALIFOBJECTBORROW(codeobj_st);
		AlifFunctionObject* funcObj = (AlifFunctionObject*)
			alifNew_function(codeobj, GLOBALS());
		ALIFSTACKREF_CLOSE(codeobj_st);
		if (funcObj == NULL) {
			//goto error;
			1 + 3;
			exit(-1);
		}
		//alifFunction_SetVersion(
			//funcObj, ((AlifCodeObject*)codeobj)->co_version);
		func.bits = (uintptr_t)((AlifObject*)funcObj);
		stackPtr[-1] = (AlifObject*)func.bits;
		DISPATCH();
	}
	TARGET(LOAD_BUILD_CLASS) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		//INSTRUCTION_STATS(LOAD_BUILD_CLASS);
		AlifStackRef bc{};
		AlifObject* bc_o;
		AlifObject* str = alifUStr_fromString(L"__build_class__");
		if (alifMapping_getOptionalItem(BUILTINS(), str, &bc_o) < 0) {
			//goto error;
			exit(-1);
		}
		//if (bc_o == NULL) {
			//exit(-1);
			//if (true) goto error;
		//}
		bc.bits = ALIFSTACKREF_FROMALIFOBJECTSTEAL(bc_o);
		stackPtr[0] = (AlifObject*)bc.bits;
		stackPtr += 1;
		DISPATCH();
	}
	TARGET(POP_TOP) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		AlifObject* value;
		value = stackPtr[-1];
		ALIF_DECREF(value);
		stackPtr += -1;
		DISPATCH();
	}
	TARGET(CACHE) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		//INSTRUCTION_STATS(CACHE);
		//ALIF_FatalError("Executing a cache.");
		DISPATCH();
	}
	TARGET(CALL) {
		_frame->instrPtr = nextInstr;
		//nextInstr += 4;
		//AlifCodeUnit* thisInstr = nextInstr - 4;
		nextInstr += 1;
		AlifCodeUnit* thisInstr = nextInstr - 1;
		AlifObject** args{};
		AlifObject* selfOrNull{};
		AlifObject* callable{};
		AlifObject* res{};
		// _SPECIALIZE_CALL
		args = &stackPtr[-opArg];
		selfOrNull = stackPtr[-1 - opArg];
		callable = stackPtr[-2 - opArg];
//		{
//			uint16_t counter = read_u16(&thisInstr[1].cache);
//			(void)counter;
//#if ENABLE_SPECIALIZATION
//			if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
//			//	nextInstr = thisInstr;
//			//	alifSpecialize_call(callable, nextInstr, opArg + (selfOrNull != nullptr));
//			//	DISPATCH_SAME_OPARG();
//			}
//			ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
//#endif	/* ENABLE_SPECIALIZATION */
//		}
		/* Skip 2 cache entries */
		// _CALL
		{
			// opArg counts all of the args, but *not* self:
			int totalArgs = opArg;
			if (selfOrNull != nullptr) {
				args--;
				totalArgs++;
			}
			else if (ALIF_TYPE(callable) == &_alifMethodType_) {
				args--;
				totalArgs++;
				AlifObject* self = ((AlifMethodObject*)callable)->self;
				args[0] = ALIF_NEWREF(self);
				AlifObject* method = ((AlifMethodObject*)callable)->func;
				args[-1] = ALIF_NEWREF(method);
				ALIF_DECREF(callable);
				callable = method;
			}
			 //Check if the call can be inlined or not
			if (ALIF_TYPE(callable) == &_alifFunctionType_ and
				_thread->interpreter->evalFrame == nullptr and
				((AlifFunctionObject*)callable)->vectorCall == alifFunction_vectorCall)
			{
				int code_flags = ((AlifCodeObject*)ALIFFUNCTION_GET_CODE(callable))->flags;
				AlifObject* locals = code_flags & CO_OPTIMIZED ? nullptr : ALIF_NEWREF(ALIFFUNCTION_GET_GLOBALS(callable));
				AlifInterpreterFrame* newFrame = alifEvalFrame_initAndPush(
					_thread, (AlifFunctionObject*)callable, locals,
					args, totalArgs, nullptr
				);
				// Manipulate stack directly since we leave using DISPATCH_INLINED().
				stackPtr += -(opArg + 2);
				// The frame has stolen all the arguments from the stack,
				// so there is no need to clean them up.
				//if (newFrame == nullptr) {
					//goto error;
				//}
				_frame->returnOffset = (uint16_t)(nextInstr - thisInstr);
				DISPATCH_INLINED(newFrame);
			}
			/* Callable is not a normal Python function */
			res = alifObject_vectorCall(callable, args, totalArgs | ALIF_VECTORCALL_ARGUMENTS_OFFSET, nullptr);
			if (opCode == INSTRUMENTED_CALL) {
				AlifObject* arg = totalArgs == 0 ?
					&_alifInstrumentationMissing_ : args[0];
				if (res == nullptr) {
					alifCall_instrumentationExc2(
						_thread, 15,
						_frame, thisInstr, callable, arg);
				}
				else {
					int err = alifCall_instrumentation2Args(
						_thread, 15,
						_frame, thisInstr, callable, arg);
					//if (err < 0) {
						//ALIF_CLEAR(res);
					//}
				}
			}
			ALIF_DECREF(callable);
			for (int i = 0; i < totalArgs; i++) {
				ALIF_DECREF(args[i]);
			}
			if (res == nullptr) { stackPtr += -2 - opArg; /*goto error*/; }
		}
		stackPtr[-2 - opArg] = res;
		stackPtr += -1 - opArg;
		//CHECK_EVAL_BREAKER();
		DISPATCH();
	}
	TARGET(CALL_INTRINSIC_2) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		//INSTRUCTION_STATS(CALL_INTRINSIC_2);
		AlifStackRef value2_st;
		AlifStackRef value1_st;
		AlifStackRef res;
		value1_st.bits = (uintptr_t)stackPtr[-1];
		value2_st.bits = (uintptr_t)stackPtr[-2];
		AlifObject* value1 = ALIFSTACKREF_ASALIFOBJECTBORROW(value1_st);
		AlifObject* value2 = ALIFSTACKREF_ASALIFOBJECTBORROW(value2_st);
		//AlifObject* res_o = alifIntrinsics_binaryFunctions[opArg].func(_thread, value2, value1);
		ALIFSTACKREF_CLOSE(value2_st);
		ALIFSTACKREF_CLOSE(value1_st);
		//if (res_o == NULL) goto pop_2_error;
		//res.bits = ALIFSTACKREF_FROMALIFOBJECTSTEAL(res_o);
		stackPtr[-2] = (AlifObject*)res.bits;
		stackPtr += -1;
		DISPATCH();
	}
	TARGET(RETURN_CONST) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		AlifObject* value;
		AlifObject* retval;
		// _LOAD_CONST
		{
			value = GETITEM(FRAME_CO_CONSTS, opArg);
			ALIF_INCREF(value);
		}
		// _POP_FRAME
		retval = value;
		{
			_frame->stacktop = (int)(stackPtr - _frame->localsPlus); // alifFrame_setStackPointer(_frame, stackPtr);
			_thread->recursionRemaining++; //alif_leaveRecursiveCallAlif(_thread);
			AlifInterpreterFrame* dying = _frame;
			_frame = _thread->currentFrame = dying->previous;
			//alifEval_frameClearAndPop(_thread, dying);
			alifFrame_stackPush(_frame, retval);
			LOAD_SP();
			LOAD_IP(_frame->returnOffset);
			//LLTRACE_RESUME_FRAME();
		}
		DISPATCH();
	}
	TARGET(CALL_NON_PY_GENERAL) {
		_frame->instrPtr = nextInstr;
		nextInstr += 4;
		//INSTRUCTION_STATS(CALL_NON_PY_GENERAL);
		AlifStackRef callable;
		AlifStackRef self_or_null;
		AlifStackRef* args;
		AlifStackRef res;
		callable = stack_pointer[-2 - oparg];
		{
			PyObject* callable_o = PyStackRef_AsPyObjectBorrow(callable);
			DEOPT_IF(PyFunction_Check(callable_o), CALL);
			DEOPT_IF(Py_TYPE(callable_o) == &PyMethod_Type, CALL);
		}
		args = &stack_pointer[-oparg];
		self_or_null = stack_pointer[-1 - oparg];
		{
			PyObject* callable_o = PyStackRef_AsPyObjectBorrow(callable);
			PyObject* self_or_null_o = PyStackRef_AsPyObjectBorrow(self_or_null);
			int total_args = oparg;
			if (self_or_null_o != NULL) {
				args--;
				total_args++;
			}
			STACKREFS_TO_PYOBJECTS(args, total_args, args_o);
			if (CONVERSION_FAILED(args_o)) {
				PyStackRef_CLOSE(callable);
				PyStackRef_CLOSE(self_or_null);
				for (int _i = oparg; --_i >= 0;) {
					PyStackRef_CLOSE(args[_i]);
				}
				if (true) {
					stack_pointer += -2 - oparg;
					goto error;
				}
			}
			PyObject* res_o = PyObject_Vectorcall(
				callable_o, args_o,
				total_args | PY_VECTORCALL_ARGUMENTS_OFFSET,
				NULL);
			STACKREFS_TO_PYOBJECTS_CLEANUP(args_o);
			PyStackRef_CLOSE(callable);
			for (int i = 0; i < total_args; i++) {
				PyStackRef_CLOSE(args[i]);
			}
			if (res_o == NULL) {
				stack_pointer += -2 - oparg;
				assert(WITHIN_STACK_BOUNDS());
				goto error;
			}
			res = PyStackRef_FromPyObjectSteal(res_o);
		}
		// _CHECK_PERIODIC
		{
		}
		stack_pointer[-2 - oparg] = res;
		stack_pointer += -1 - oparg;
		//CHECK_EVAL_BREAKER();
		DISPATCH();
	}

	TARGET(LOAD_SUPER_ATTR) {
		_frame->instrPtr = nextInstr;
		nextInstr += 2;
		//INSTRUCTION_STATS(LOAD_SUPER_ATTR);
		PREDICTED(LOAD_SUPER_ATTR);
		AlifCodeUnit* this_instr = nextInstr - 2;
		(void)this_instr;
		AlifStackRef global_super_st;
		AlifStackRef class_st;
		AlifStackRef self_st;
		AlifStackRef attr;
		AlifStackRef null = ALIFSTACKREF_NULL;
		class_st.bits = (uintptr_t)stackPtr[-2];
		global_super_st.bits = (uintptr_t)stackPtr[-3];
		{
			uint16_t counter = read_u16(&this_instr[1].cache);
			(void)counter;
#if ENABLE_SPECIALIZATION
			int load_method = opArg & 1;
			if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
				nextInstr = this_instr;
				alifSpecialize_loadSuperAttr(global_super_st, class_st, nextInstr, load_method);
				DISPATCH_SAME_OPARG();
			}
			//STAT_INC(LOAD_SUPER_ATTR, deferred);
			ADVANCE_ADAPTIVE_COUNTER(this_instr[1].counter);
#endif  /* ENABLE_SPECIALIZATION */
		}
		self_st.bits = (uintptr_t)stackPtr[-1];
		{
			AlifObject* global_super = ALIFSTACKREF_ASALIFOBJECTBORROW(global_super_st);
			AlifObject* class_ = ALIFSTACKREF_ASALIFOBJECTBORROW(class_st);
			AlifObject* self = ALIFSTACKREF_ASALIFOBJECTBORROW(self_st);
			if (opCode == INSTRUMENTED_LOAD_SUPER_ATTR) {
				AlifObject* arg = opArg & 2 ? class_ : &_alifInstrumentationMissing_;
				//int err = alifCall_instrumentation2Args(
					//_thread, ALIF_MONITORING_EVENT_CALL,
					//frame, this_instr, global_super, arg);
				//if (err) goto pop_3_error;
			}
			AlifObject* stack[] = { class_, self };
			AlifObject* super = alifObject_vectorCall(global_super, stack, opArg & 2, NULL);
			if (opCode == INSTRUMENTED_LOAD_SUPER_ATTR) {
				AlifObject* arg = opArg & 2 ? class_ : &_alifInstrumentationMissing_;
				if (super == NULL) {
					alifCall_instrumentationExc2(
						_thread, 15,
						_frame, this_instr, global_super, arg);
				}
				else {
					int err = alifCall_instrumentation2Args(
						_thread, 15,
						_frame, this_instr, global_super, arg);
//					if (err < 0) {
//						ALIF_CLEAR(super);
//					}
				}
			}
			ALIFSTACKREF_CLOSE(global_super_st);
			ALIFSTACKREF_CLOSE(class_st);
			ALIFSTACKREF_CLOSE(self_st);
//			if (super == NULL) goto pop_3_error;
			AlifObject* name = GETITEM(FRAME_CO_NAMES, opArg >> 2);
			attr.bits = ALIFSTACKREF_FROMALIFOBJECTSTEAL(alifObject_getAttr(super, name));
			ALIF_DECREF(super);
//			if (PyStackRef_IsNull(attr)) goto pop_3_error;
			null = ALIFSTACKREF_NULL;
		}
		stackPtr[-3] = (AlifObject*)attr.bits;
		if (opArg & 1) stackPtr[-2] = nullptr;
		stackPtr += -2 + (opArg & 1);
		DISPATCH();
	}
	TARGET(LOAD_SUPER_ATTR_ATTR) {
		_frame->instrPtr = nextInstr;
		nextInstr += 2;
		//INSTRUCTION_STATS(LOAD_SUPER_ATTR_ATTR);
		AlifStackRef global_super_st;
		AlifStackRef class_st;
		AlifStackRef self_st;
		AlifStackRef attr_st;
		/* Skip 1 cache entry */
		self_st.bits = (uintptr_t)stackPtr[-1];
		class_st.bits = (uintptr_t)stackPtr[-2];
		global_super_st.bits = (uintptr_t)stackPtr[-3];
		AlifObject* global_super = ALIFSTACKREF_ASALIFOBJECTBORROW(global_super_st);
		AlifObject* class_ = ALIFSTACKREF_ASALIFOBJECTBORROW(class_st);
		AlifObject* self = ALIFSTACKREF_ASALIFOBJECTBORROW(self_st);
		DEOPT_IF(global_super != (AlifObject*)&_alifSuperType_, LOAD_SUPER_ATTR);
		DEOPT_IF(!ALIFTYPE_CHECK(class_), LOAD_SUPER_ATTR);
		//STAT_INC(LOAD_SUPER_ATTR, hit);
		AlifObject* name = GETITEM(FRAME_CO_NAMES, opArg >> 2);
		AlifObject* attr = alifSuper_lookup((AlifTypeObject*)class_, self, name, NULL);
		ALIFSTACKREF_CLOSE(global_super_st);
		ALIFSTACKREF_CLOSE(class_st);
		ALIFSTACKREF_CLOSE(self_st);
		// if (attr == NULL) goto pop_3_error;
		attr_st.bits = ALIFSTACKREF_FROMALIFOBJECTSTEAL(attr);
		stackPtr[-3] = (AlifObject*)attr_st.bits;
		stackPtr += -2;
		DISPATCH();
	}
	TARGET(SET_FUNCTION_ATTRIBUTE) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		//INSTRUCTION_STATS(SET_FUNCTION_ATTRIBUTE);
		AlifStackRef attr_st;
		AlifStackRef func_st;
		func_st.bits = (uintptr_t)stackPtr[-1];
		attr_st.bits = (uintptr_t)stackPtr[-2];
		AlifObject* func = ALIFSTACKREF_ASALIFOBJECTBORROW(func_st);
		AlifObject* attr = ALIFSTACKREF_ASALIFOBJECTBORROW(attr_st);
		AlifFunctionObject* func_obj = (AlifFunctionObject*)func;
		switch (opArg) {
		case MAKE_FUNCTION_CLOSURE:
			func_obj->funcClosure = attr;
			break;
		//case MAKE_FUNCTION_ANNOTATIONS:
			//func_obj->func_annotations = attr;
			//break;
		case MAKE_FUNCTION_KWDEFAULTS:
			func_obj->funcKwdefaults = attr;
			break;
		case MAKE_FUNCTION_DEFAULTS:
			func_obj->funcDefaults = attr;
			break;
		//case MAKE_FUNCTION_ANNOTATE:
			//func_obj->func_annotate = attr;
			//break;
		//default:
			//ALIF_UNREACHABLE();
		}
		stackPtr[-2] = (AlifObject*)func_st.bits;
		stackPtr += -1;
		DISPATCH();
	}
	TARGET(SETUP_ANNOTATIONS) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		//INSTRUCTION_STATS(SETUP_ANNOTATIONS);
		int err;
		AlifObject* ann_dict;
		if (LOCALS() == NULL) {
			//if (true) goto error;
		}
		/* check if __annotations__ in locals()... */
		AlifObject* str = alifUStr_fromString(L"__annotations__");
		if (alifMapping_getOptionalItem(LOCALS(), str, &ann_dict) < 0)
			//goto error;
		if (ann_dict == NULL) {
			ann_dict = alifNew_dict();
			//if (ann_dict == NULL)
				//goto error;
			err = alifObject_setItem(LOCALS(), str,
				ann_dict);
			ALIF_DECREF(ann_dict);
			//if (err) goto error;
		}
		else {
			ALIF_DECREF(ann_dict);
		}
		DISPATCH();
	}
	TARGET(INSTRUMENTED_CALL) {
		AlifCodeUnit* this_instr = _frame->instrPtr = nextInstr;
		(void)this_instr;
		nextInstr += 4;
		//INSTRUCTION_STATS(INSTRUMENTED_CALL);
		AlifStackRef callable;
		AlifStackRef self_or_null;
		AlifStackRef* args{};
		AlifStackRef func;
		AlifStackRef maybe_self;
		AlifStackRef res;
		/* Skip 3 cache entries */
		// _MAYBE_EXPAND_METHOD
		args->bits = (uintptr_t)&stackPtr[-opArg];
		self_or_null.bits = (uintptr_t)stackPtr[-1 - opArg];
		callable.bits = (uintptr_t)stackPtr[-2 - opArg];
		{
			args->bits = (uintptr_t)(&stackPtr[-opArg]);
			if (ALIFSTACKREF_TYPE(callable) == &_alifMethodType_ && ALIFSTACKREF_ISNULL(self_or_null)) {
				AlifObject* callable_o = ALIFSTACKREF_ASALIFOBJECTBORROW(callable);
				AlifObject* self = ((AlifMethodObject*)callable_o)->self;
				maybe_self.bits = ALIFSTACKREF_FROMALIFOBJECTNEW(self);
				AlifObject* method = ((AlifMethodObject*)callable_o)->func;
				func.bits = ALIFSTACKREF_FROMALIFOBJECTNEW(method);
				/* Make sure that callable and all args are in memory */
				args[-2] = func;
				args[-1] = maybe_self;
				ALIFSTACKREF_CLOSE(callable);
			}
			else {
				func = callable;
				maybe_self = self_or_null;
			}
		}
		args->bits = (uintptr_t) & stackPtr[-opArg];
		{
			int is_meth = !ALIFSTACKREF_ISNULL(maybe_self);
			AlifObject* function = ALIFSTACKREF_ASALIFOBJECTBORROW(func);
			AlifObject* arg0;
			if (is_meth) {
				arg0 = ALIFSTACKREF_ASALIFOBJECTBORROW(maybe_self);
			}
			else if (opArg) {
				arg0 = ALIFSTACKREF_ASALIFOBJECTBORROW(args[0]);
			}
			else {
				arg0 = &_alifInstrumentationMissing_;
			}
			int err = alifCall_instrumentation2Args(
				_thread, 15,
				_frame, this_instr, function, arg0
			);
			//if (err) goto error;
		}
		args->bits = (uintptr_t) & stackPtr[-opArg];
		self_or_null = maybe_self;
		callable = func;
		{
			AlifObject* callable_o = ALIFSTACKREF_ASALIFOBJECTBORROW(callable);
			int total_args = opArg;
			if (!ALIFSTACKREF_ISNULL(self_or_null)) {
				args--;
				total_args++;
			}
			if (ALIF_TYPE(callable_o) == &_alifFunctionType_ &&
				_thread->interpreter->evalFrame == NULL &&
				((AlifFunctionObject*)callable_o)->vectorCall == alifFunction_vectorCall)
			{
				int code_flags = ((AlifCodeObject*)ALIFFUNCTION_GET_CODE(callable_o))->flags;
				AlifObject* locals = code_flags & CO_OPTIMIZED ? NULL : ALIF_NEWREF(ALIFFUNCTION_GET_GLOBALS(callable_o));
				AlifInterpreterFrame* new_frame = alifEvalFrame_initAndPush(
					_thread, (AlifFunctionObject*)(callable.bits), locals,
					(AlifObject* const*)args->bits, total_args, NULL
				);
				stackPtr += -(opArg + 2);
				//if (new_frame == NULL) {
					//goto error;
				//}
				_frame->returnOffset = (uint16_t)(nextInstr - this_instr);
				DISPATCH_INLINED(new_frame);
			}
			/* Callable is not a normal Python function */
			STACKREFS_TO_ALIFOBJECTS(args, total_args, args_o);
			if (CONVERSION_FAILED(args_o)) {
				ALIFSTACKREF_CLOSE(callable);
				ALIFSTACKREF_CLOSE(self_or_null);
				for (int _i = opArg; --_i >= 0;) {
					ALIFSTACKREF_CLOSE(args[_i]);
				}
				if (true) {
					stackPtr += -2 - opArg;
					//goto error;
				}
			}
			AlifObject* res_o = alifObject_vectorCall(
				callable_o, args_o,
				total_args | ALIF_VECTORCALL_ARGUMENTS_OFFSET,
				NULL);
			//STACKREFS_TO_ALIFOBJECTS_CLEANUP(args_o);
			if (opCode == INSTRUMENTED_CALL) {
				AlifObject* arg = total_args == 0 ?
					&_alifInstrumentationMissing_ : ALIFSTACKREF_ASALIFOBJECTBORROW(args[0]);
				if (res_o == NULL) {
					alifCall_instrumentationExc2(
						_thread, 15,
						_frame, this_instr, callable_o, arg);
				}
				else {
					int err = alifCall_instrumentation2Args(
						_thread, 15,
						_frame, this_instr, callable_o, arg);
					//if (err < 0) {
						//ALIF_CLEAR(res_o);
					//}
				}
			}
			ALIFSTACKREF_CLOSE(callable);
			for (int i = 0; i < total_args; i++) {
				ALIFSTACKREF_CLOSE(args[i]);
			}
			if (res_o == NULL) {
				stackPtr += -2 - opArg;
				//goto error;
			}
			res.bits = ALIFSTACKREF_FROMALIFOBJECTSTEAL(res_o);
		}
		// _CHECK_PERIODIC
		{
		}
		stackPtr[-2 - opArg] = (AlifObject*)res.bits;
		stackPtr += -1 - opArg;
		//CHECK_EVAL_BREAKER();
		DISPATCH();
	}
	TARGET(INSTRUMENTED_RETURN_CONST) {
		AlifCodeUnit* this_instr = _frame->instrPtr = nextInstr;
		(void)this_instr;
		nextInstr += 1;
		//INSTRUCTION_STATS(INSTRUMENTED_RETURN_CONST);
		AlifStackRef value;
		AlifStackRef val;
		AlifStackRef retval;
		AlifStackRef res;
		// _LOAD_CONST
		{
			value.bits = ALIFSTACKREF_FROMALIFOBJECTNEW(GETITEM(FRAME_CO_CONSTS, opArg));
		}
		// _RETURN_VALUE_EVENT
		val = value;
		{
			int err = alifCall_instrumentationArg(
				_thread, 2,
				_frame, this_instr, ALIFSTACKREF_ASALIFOBJECTBORROW(val));
			//if (err) goto error;
		}
		// _RETURN_VALUE
		retval = val;
		{
#if TIER_ONE
#endif
			alifFrame_setStackPointer(_frame, stackPtr);
			//alif_leaveRecursiveCallPy(tstate);
			// GH-99729: We need to unlink the frame *before* clearing it:
			AlifInterpreterFrame* dying = _frame;
			_frame = _thread->currentFrame = dying->previous;
			//alifEval_frameClearAndPop(_thread, dying);
			LOAD_SP();
			LOAD_IP(_frame->returnOffset);
			res = retval;
			//LLTRACE_RESUME_FRAME();
		}
		stackPtr[0] = (AlifObject*)res.bits;
		stackPtr += 1;
		DISPATCH();
	}
	TARGET(POP_JUMP_IF_NONE) {
		AlifCodeUnit* this_instr = _frame->instrPtr = nextInstr;
		(void)this_instr;
		nextInstr += 2;
		//INSTRUCTION_STATS(POP_JUMP_IF_NONE);
		AlifStackRef value;
		AlifStackRef b;
		AlifStackRef cond;
		value.bits = (uintptr_t)stackPtr[-1];
		{
			if (value.bits == ALIFSTACKREF_NONE) {
				b.bits = ALIFSTACKREF_TRUE;
			}
			else {
				b.bits = ALIFSTACKREF_FALSE;
				ALIFSTACKREF_CLOSE(value);
			}
		}
		cond = b;
		{
			int flag = (cond.bits == ALIFSTACKREF_TRUE);
#if ENABLE_SPECIALIZATION
			this_instr[1].cache = (this_instr[1].cache << 1) | flag;
#endif
			nextInstr += (opArg * flag);
		}
		stackPtr += -1;
		DISPATCH();
	}
	TARGET(INTERPRETER_EXIT) {
		_frame->instrPtr = nextInstr;
		nextInstr += 1;
		AlifObject* retval;
		retval = stackPtr[-1];
		/* Restore previous frame and return. */
		_thread->currentFrame = _frame->previous;
		_thread->recursionRemaining += ALIFEVAL_CSTACK_UNITS;
		return retval;
	}
	//default:
	//	break;
	}


	return nullptr; // temp
}



static AlifIntT positionalOnly_passedAsKeyword(AlifThread* _thread, AlifCodeObject* _co,
	AlifSizeT _kwCount, AlifObject* _kwNames, AlifObject* qualname) { 

	int posOnlyConflicts = 0;
	AlifObject* posOnlyNames = alifNew_list(0);
	if (posOnlyNames == nullptr) goto fail;
	
	for (int k = 0; k < _co->posOnlyArgCount; k++) {
		AlifObject* posOnlyName = ALIFTUPLE_GET_ITEM(_co->localsPlusNames, k);

		for (int k2 = 0; k2 < _kwCount; k2++) {
			AlifObject* kwName = ALIFTUPLE_GET_ITEM(_kwNames, k2);
			if (kwName == posOnlyName) {
				if (alifList_append(posOnlyNames, kwName) != 0) {
					goto fail;
				}
				posOnlyConflicts++;
				continue;
			}

			int cmp = alifObject_richCompareBool(posOnlyName, kwName, ALIF_EQ);

			if (cmp > 0) {
				if (alifList_append(posOnlyNames, kwName) != 0) {
					goto fail;
				}
				posOnlyConflicts++;
			}
			else if (cmp < 0) {
				goto fail;
			}

		}
	}
	if (posOnlyConflicts) {
		AlifObject* comma = alifUStr_fromString(L", ");
		if (comma == nullptr) {
			goto fail;
		}
		AlifObject* errorNames = alifUStr_join(comma, posOnlyNames);
		ALIF_DECREF(comma);
		if (errorNames == nullptr) {
			goto fail;
		}
		// error
		ALIF_DECREF(errorNames);
		goto fail;
	}

	ALIF_DECREF(posOnlyNames);
	return 0;

fail:
	ALIF_XDECREF(posOnlyNames);
	return 1;

}


static AlifIntT initialize_locals(AlifThread* _thread, AlifFunctionObject* _func,
	AlifObject** _localsPlus, AlifObject* const* _args, AlifSizeT _argCount, AlifObject* _kwNames) { 

	AlifCodeObject* co = (AlifCodeObject*)_func->funcCode;
	const AlifSizeT totalArgs = co->args + co->kwOnlyArgCount;

	AlifObject* kwDict{};
	AlifSizeT i{};
	if (co->flags & CO_VARKEYWORDS) {
		kwDict = alifNew_dict();
		if (kwDict == nullptr) {
			goto fail_pre_positional;
		}
		i = totalArgs;
		if (co->flags & CO_VARARGS) {
			i++;
		}
		_localsPlus[i] = kwDict;
	}
	else {
		kwDict = nullptr;
	}

	AlifSizeT j, n;
	if (_argCount > co->args) {
		n = co->args;
	}
	else {
		n = _argCount;
	}
	for (j = 0; j < n; j++) {
		AlifObject* x = _args[j];
		_localsPlus[j] = x;
	}

	if (co->flags & CO_VARARGS) {
		AlifObject* u = nullptr;
		if (_argCount == n) {
			u = (AlifObject*)&ALIF_SINGLETON(tupleEmpty);
		}
		else {
			u = alifTuple_fromArraySteal(_args + n, _argCount - n);
		}
		if (u == nullptr) {
			goto fail_post_positional;
		}
		_localsPlus[totalArgs] = u;
	}
	else if (_argCount > n) {
		for (j = n; j < _argCount; j++) {
			ALIF_DECREF(_args[j]);
		}
	}

	if (_kwNames != nullptr) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (i = 0; i < kwcount; i++) {
			AlifObject** co_varnames;
			AlifObject* keyword = ALIFTUPLE_GET_ITEM(_kwNames, i);
			AlifObject* value = _args[i + _argCount];
			AlifSizeT j;

			if (keyword == nullptr or !ALIFUSTR_CHECK(keyword)) {
				// error
				goto kw_fail;
			}

			co_varnames = ((AlifTupleObject*)(co->localsPlusNames))->items_;
			for (j = co->posOnlyArgCount; j < totalArgs; j++) {
				AlifObject* varname = co_varnames[j];
				if (varname == keyword) {
					goto kw_found;
				}
			}

			for (j = co->posOnlyArgCount; j < totalArgs; j++) {
				AlifObject* varname = co_varnames[j];
				int cmp = alifObject_richCompareBool(keyword, varname, ALIF_EQ);
				if (cmp > 0) {
					goto kw_found;
				}
				else if (cmp < 0) {
					goto kw_fail;
				}
			}

			if (kwDict == nullptr) {

				if (co->posOnlyArgCount and positionalOnly_passedAsKeyword(_thread, co,
						kwcount, _kwNames,
						_func->funcQualname))
				{
					goto kw_fail;
				}

				AlifObject* suggestion_keyword = nullptr;
				if (totalArgs > co->posOnlyArgCount) {
					AlifObject* possible_keywords = alifNew_list(totalArgs - co->posOnlyArgCount);

					if (!possible_keywords) {
						//error
					}
					else {
						for (AlifSizeT k = co->posOnlyArgCount; k < totalArgs; k++) {
							ALIFLIST_SETITEM(possible_keywords, k - co->posOnlyArgCount, co_varnames[k]);
						}

						//suggestion_keyword = alif_calculateSuggestions(possible_keywords, keyword);
						ALIF_DECREF(possible_keywords);
					}
				}

				if (suggestion_keyword) {
					// error
					ALIF_DECREF(suggestion_keyword);
				}
				else {
					// error
				}

				goto kw_fail;
			}

			//if (alifDict_setItem(kwDict, keyword, value) == -1) {
			//	goto kw_fail;
			//}
			ALIF_DECREF(value);
			continue;

		kw_fail:
			for (; i < kwcount; i++) {
				AlifObject* value = _args[i + _argCount];
				ALIF_DECREF(value);
			}
			goto fail_post_args;

		kw_found:
			if (_localsPlus[j] != nullptr) {
				// errro
				goto kw_fail;
			}
			_localsPlus[j] = value;
		}
	}

	if ((_argCount > co->args) and !(co->flags & CO_VARARGS)) {
		//too_many_positional(_thread, co, _argCount, _func->funcDefaults, _localsPlus, _func->funcQualname);
		goto fail_post_args;
	}

	if (_argCount < co->args) {
		AlifSizeT defcount = _func->funcDefaults == nullptr ? 0 : ALIFTUPLE_GET_SIZE(_func->funcDefaults);
		AlifSizeT m = co->args - defcount;
		AlifSizeT missing = 0;
		for (i = _argCount; i < m; i++) {
			if (_localsPlus[i] == nullptr) {
				missing++;
			}
		}
		if (missing) {
			//missing_arguments(_thread, co, missing, defcount, _localsPlus, _func->funcQualname);
			goto fail_post_args;
		}
		if (n > m)
			i = n - m;
		else
			i = 0;
		if (defcount) {
			AlifObject** defs = &ALIFTUPLE_GET_ITEM(_func->funcDefaults, 0);
			for (; i < defcount; i++) {
				if (_localsPlus[m + i] == nullptr) {
					AlifObject* def = defs[i];
					_localsPlus[m + i] = ALIF_NEWREF(def);
				}
			}
		}
	}

	if (co->kwOnlyArgCount > 0) {
		AlifSizeT missing = 0;
		for (i = co->args; i < totalArgs; i++) {
			if (_localsPlus[i] != nullptr)
				continue;
			AlifObject* varname = ALIFTUPLE_GET_ITEM(co->localsPlusNames, i);
			if (_func->funcKwdefaults != nullptr) {
				AlifObject* def;
				if (alifDict_getItemRef(_func->funcKwdefaults, varname, &def) < 0) {
					goto fail_post_args;
				}
				if (def) {
					_localsPlus[i] = def;
					continue;
				}
			}
			missing++;
		}
		if (missing) {
			//missing_arguments(_thread, co, missing, -1, _localsPlus, _func->funcQualname);
			goto fail_post_args;
		}
	}
	return 0;

fail_pre_positional:
	for (j = 0; j < _argCount; j++) {
		ALIF_DECREF(_args[j]);
	}

fail_post_positional:
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (j = _argCount; j < _argCount + kwcount; j++) {
			ALIF_DECREF(_args[j]);
		}
	}

fail_post_args:
	return -1;
}

static void clear_threadFrame(AlifThread* _thread, AlifInterpreterFrame* _frame)
{
	_thread->recursionRemaining--;
	//alifFrame_clearExceptCode(_frame);
	//ALIF_DECREF(_frame->f_executable);
	_thread->recursionRemaining++;
	//alifThread_popFrame(_thread, _frame);
}

AlifInterpreterFrame* alifEvalFrame_initAndPush(AlifThread* _thread, AlifFunctionObject* _func,
	AlifObject* _locals, AlifObject* const* _args, AlifUSizeT _argCount, AlifObject* _kwNames) { 

	AlifCodeObject* code = (AlifCodeObject*)_func->funcCode;
	AlifInterpreterFrame* frame = alifThread_pushFrame(_thread, code->frameSize);
	if (frame == nullptr) goto fail;

	alifFrame_initialize(frame, _func, _locals, code, 0);
	if (initialize_locals(_thread, _func, frame->localsPlus, _args, _argCount, _kwNames)) {
		clear_threadFrame(_thread, frame);
		return nullptr;
	}
	return frame;
fail:
	ALIF_DECREF(_func);
	ALIF_XDECREF(_locals);
	for (size_t i = 0; i < _argCount; i++) {
		ALIF_DECREF(_args[i]);
	}
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (AlifSizeT i = 0; i < kwcount; i++) {
			ALIF_DECREF(_args[i + _argCount]);
		}
	}
	// memory error
	return nullptr;
}


AlifObject* alifEval_vector(AlifThread* _thread, AlifFunctionObject* _func, AlifObject* _locals,
	AlifObject* const* _args, AlifUSizeT _argcount, AlifObject* _kwnames) { 

	ALIF_INCREF(_func);
	ALIF_XINCREF(_locals);
	for (AlifUSizeT i = 0; i < _argcount; i++) {
		ALIF_INCREF(_args[i]);
	}
	if (_kwnames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwnames);
		for (AlifSizeT i = 0; i < kwcount; i++) {
			ALIF_INCREF(_args[i + _argcount]);
		}
	}
	AlifInterpreterFrame* frame = alifEvalFrame_initAndPush(
		_thread, _func, _locals, _args, _argcount, _kwnames);
	if (frame == nullptr) return nullptr;

	return alifEval_evalFrame(_thread, frame, 0);
}




AlifObject* alifEval_getBuiltins(AlifThread* _thread) { 
	AlifInterpreterFrame* frame = alifThread_getFrame(_thread);
	if (frame != nullptr) {
		return frame->builtins;
	}
	return _thread->interpreter->builtins;
}
