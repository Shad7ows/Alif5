#pragma once

#ifdef LLTRACE
#define PRE_DISPATCH_GOTO() if (lltrace >= 5) { \
    lltrace_instruction(frame, stack_pointer, next_instr, opcode, oparg); }
#else
#define PRE_DISPATCH_GOTO() ((void)0)
#endif

#  define TARGET(_op) case _op: TARGET_##_op:
#  define DISPATCH_GOTO() goto dispatchOpCode 

#define DISPATCH_SAME_OPARG() \
    { \
        opCode = nextInstr->op.code; \
        PRE_DISPATCH_GOTO(); \
        DISPATCH_GOTO(); \
    }

#define DISPATCH_INLINED(NEW_FRAME)                     \
    do {                                                \
        alifFrame_setStackPointer(_frame, stackPtr); \
        (NEW_FRAME)->previous = _frame;                  \
        _frame = _thread->currentFrame = (NEW_FRAME);     \
        goto startFrame;                               \
    } while (0)

#define NEXTOPARG()  do { \
        AlifCodeUnit word  = {*(uint16_t*)nextInstr}; \
        opCode = word.op.code; \
        opArg = word.op.arg; \
    } while (0)



#define DISPATCH() { NEXTOPARG(); DISPATCH_GOTO(); }



#define GETITEM(_v, _i) ALIFTUPLE_GET_ITEM((_v), (_i))




#define PREDICT_ID(op)          PRED_##op
#define PREDICTED(op)           PREDICT_ID(op):



#define FRAME_CO_CONSTS (alifFrame_getCode(_frame)->consts)
#define FRAME_CO_NAMES  (alifFrame_getCode(_frame)->names)

#define GO_TO_INSTRUCTION(op) goto PREDICT_ID(op)


#define DEOPT_IF(COND, INSTNAME)                            \
    if ((COND)) {                                           \
        GO_TO_INSTRUCTION(INSTNAME);                        \
    }

//294
#define GLOBALS() _frame->globals
#define BUILTINS() _frame->builtins
#define LOCALS() _frame->locals
#define CONSTS() alifFrame_getCode(_frame)->consts
#define NAMES() alifFrame_getCode(_frame)->names


#define ADAPTIVE_COUNTER_TRIGGERS(_counter) \
    backoff_counterTriggers(forge_backoffCounter((_counter)))

#define ADVANCE_ADAPTIVE_COUNTER(_counter) \
    do { \
        _counter = advance_backoffCounter(_counter); \
    } while (0);


#define LOAD_IP(_offset) do { \
        nextInstr = _frame->instrPtr + (_offset); \
    } while (0)

#define LOAD_SP() stackPtr = alifFrame_getStackPointer(_frame);

#define DECREF_INPUTS_AND_REUSE_FLOAT(left, right, dval, result) \
do { \
    if (ALIF_REFCNT(left) == 1) { \
        ((AlifFloatObject *)left)->digits_ = (dval); \
        alifSub_decref_specialized(right, alifFloat_exactDealloc);\
        result = (left); \
    } \
    else if (ALIF_REFCNT(right) == 1)  {\
        ((AlifFloatObject *)right)->digits_ = (dval); \
        alifDecref_no_dealloc(left); \
        result = (right); \
    }\
    else { \
        result = alifFloat_fromDouble(dval); \
        alifDecref_no_dealloc(left); \
        alifDecref_no_dealloc(right); \
    } \
} while (0)


#define STACKREFS_TO_ALIFOBJECTS(ARGS, ARG_COUNT, NAME) \
    AlifObject **NAME = (AlifObject **)ARGS; \

#define CONVERSION_FAILED(NAME) (0)
