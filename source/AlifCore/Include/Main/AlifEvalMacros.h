#pragma once








 // 73
#if USE_COMPUTED_GOTOS
#  define TARGET(op) TARGET_##op:
#  define DISPATCH_GOTO() goto *opcode_targets[opcode]
#else
#  define TARGET(_op) case _op: TARGET_##_op:
#  define DISPATCH_GOTO() goto dispatch_opcode
#endif


 // 101
#ifdef ALIF_GIL_DISABLED
#define QSBR_QUIESCENT_STATE(_thread) _alifQSBR_quiescentState(((AlifThreadImpl*)_thread)->qsbr)
#else
#define QSBR_QUIESCENT_STATE(_thread)
#endif

 // 109
#define DISPATCH() \
    { \
        NEXTOPARG(); \
        /*PRE_DISPATCH_GOTO();*/ \
        DISPATCH_GOTO(); \
    }



#define GETITEM(_v, _i) ALIFTUPLE_GET_ITEM(_v, _i) // 139






 // 154
#define NEXTOPARG()  do { \
        AlifCodeUnit word  = {.cache = alifAtomic_loadUint16Relaxed(&*(uint16_t*)nextInstr)}; \
        opcode = word.op.code; \
        oparg = word.op.arg; \
    } while (0)

 // 193
#define PREDICT_ID(_op)          PRED_##_op
#define PREDICTED(_op)           PREDICT_ID(_op):




/* Data access macros */
#define FRAME_CO_CONSTS (_alifFrame_getCode(_frame)->consts) // 243
#define FRAME_CO_NAMES  (_alifFrame_getCode(_frame)->names)


static inline AlifIntT _alif_enterRecursiveAlif(AlifThread* _thread) { // 368
	return (_thread->alifRecursionRemaining-- <= 0) and
		_alif_checkRecursiveCallAlif(_thread);
}




#define MAX_STACKREF_SCRATCH 10 // 442

 // 445
#define STACKREFS_TO_ALIFOBJECTS(_args, _argCount, _name) \
    /* +1 because vectorcall might use -1 to write self */ \
    AlifObject *_name##_temp[MAX_STACKREF_SCRATCH+1]; \
    AlifObject **_name = _alifObjectArray_fromStackRefArray(_args, _argCount, _name##_temp + 1);

 // 456
#define STACKREFS_TO_ALIFOBJECTS_CLEANUP(_name) \
    /* +1 because we +1 previously */ \
    _alifObjectArray_free(_name - 1, _name##_temp);

#define CONVERSION_FAILED(_name) (_name == nullptr) // 465
