#pragma once

#  define TARGET(_op) case _op: TARGET_##_op:
#  define DISPATCH_GOTO() goto dispatchOpCode // 78



// 164
#define NEXTOPARG()  do { \
        AlifCodeUnit word  = {*(uint16_t*)nextInstr}; \
        opCode = word.op.code; \
        opArg = word.op.arg; \
    } while (0)


// 109
#define DISPATCH() { NEXTOPARG(); DISPATCH_GOTO(); }


// 149
#define GETITEM(_v, _i) ALIFTUPLE_GET_ITEM((_v), (_i))







// 251
#define FRAME_CO_CONSTS (alifFrame_getCode(_frame)->consts)
#define FRAME_CO_NAMES  (alifFrame_getCode(_frame)->names)

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

// 396
#define LOAD_IP(_offset) do { \
        nextInstr = _frame->instrPtr + (_offset); \
    } while (0)
// 402
#define LOAD_SP() stackPtr = alifFrame_getStackPointer(_frame);
