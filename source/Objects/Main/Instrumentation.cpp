#include "alif.h"
#include "OpCodeIDs.h"
#include "AlifCore_Call.h"
#include "AlifCore_Code.h"
#include "AlifCore_Frame.h"
#include "AlifCore_OpCodeData.h"
#include "AlifCore_Instruments.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Object.h"

AlifObject _alifInstrumentationDisable_ = ALIFSUBOBJECT_HEAD_INIT(&_alifBaseObjectType_);
AlifObject _alifInstrumentationMissing_ = ALIFSUBOBJECT_HEAD_INIT(&_alifBaseObjectType_);


static const uint8_t deInstrument[256] = { 
    RESUME,
    RETURN_VALUE,
    RETURN_CONST,
    //CALL,
    //CALL_KW,
    //CALL_FUNCTION_EX,
    //YIELD_VALUE,
    JUMP_FORWARD,
    JUMP_BACKWARD,
    POP_JUMP_IF_FALSE,
    POP_JUMP_IF_TRUE,
    POP_JUMP_IF_NONE,
    POP_JUMP_IF_NOT_NONE,
    //FOR_ITER,
    //END_FOR,
    //END_SEND,
    //LOAD_SUPER_ATTR,
};

AlifIntT alif_getBaseOpCode(AlifCodeObject* _code, AlifIntT _i)
{
    AlifIntT opcode = ALIFCODE_CODE(_code)[_i].op.code;
    if (opcode == INSTRUMENTED_LINE) {
        //opcode = _code->monitoring->lines[_i].originalOpCode;
    }
    if (opcode == INSTRUMENTED_INSTRUCTION) {
        //opcode = _code->monitoring->perInstructionOpCodes[_i];
    }

    AlifIntT deinstrumented = deInstrument[opcode];
    if (deinstrumented) {
        return deinstrumented;
    }
    return alifOpCode_deOpt[opcode];
}

static void de_instrument(AlifCodeObject* code, int i, int event)
{

	AlifCodeUnit* instr = &ALIFCODE_CODE(code)[i];
	uint8_t* opcode_ptr = &instr->op.code;
	int opcode = *opcode_ptr;
	if (opcode == INSTRUMENTED_LINE) {
		opcode_ptr = &code->coMonitoring->lines[i].original_opcode;
		opcode = *opcode_ptr;
	}
	if (opcode == INSTRUMENTED_INSTRUCTION) {
		opcode_ptr = &code->coMonitoring->per_instruction_opcodes[i];
		opcode = *opcode_ptr;
	}
	int deinstrumented = deInstrument[opcode];
	if (deinstrumented == 0) {
		return;
	}
	//if (alifOpCode_deOpt[deinstrumented]) {
		//FT_ATOMIC_STORE_UINT16_RELAXED(instr[1].counter.as_counter,
			//adaptive_counter_warmup().as_counter);
	//}
}

static void remove_tools(AlifCodeObject* code, int offset, int event, int tools)
{

	AlifCoMonitoringData* monitoring = code->coMonitoring;
	if (monitoring && monitoring->tools) {
		monitoring->tools[offset] &= ~tools;
		if (monitoring->tools[offset] == 0) {
			de_instrument(code, offset, event);
		}
	}
	else {
		/* Single tool */
		uint8_t single_tool = code->coMonitoring->active_monitors.tools[event];
		if (((single_tool & tools) == single_tool)) {
			de_instrument(code, offset, event);
		}
	}
}


// in file Monitoring.h
#define ALIF_MONITORING_EVENT_ALIF_START 0
#define ALIF_MONITORING_EVENT_ALIF_RESUME 1
#define ALIF_MONITORING_EVENT_ALIF_RETURN 2
#define ALIF_MONITORING_EVENT_ALIF_YIELD 3
#define ALIF_MONITORING_EVENT_CALL 4
#define ALIF_MONITORING_EVENT_LINE 5
#define ALIF_MONITORING_EVENT_INSTRUCTION 6
#define ALIF_MONITORING_EVENT_JUMP 7
#define ALIF_MONITORING_EVENT_BRANCH 8
#define ALIF_MONITORING_EVENT_STOP_ITERATION 9

#define ALIF_MONITORING_IS_INSTRUMENTED_EVENT(ev) \
    ((ev) < 10)

static int
call_one_instrument(
	AlifInterpreter* interp, AlifThread* tstate, AlifObject** args,
	size_t nargsf, int8_t tool, int event)
{
	AlifObject* instrument = interp->monitoringCallables[tool][event];
	if (instrument == NULL) {
		return 0;
	}
	//int old_what = tstate->what_event;
	//tstate->what_event = event;
	tstate->tracing++;
	AlifObject* res = alifObject_vectorCallTState(tstate, instrument, args, nargsf, NULL);
	tstate->tracing--;
	//tstate->whatEvent = old_what;
	if (res == NULL) {
		return -1;
	}
	ALIF_DECREF(res);
	return (res == &_alifInstrumentationDisable_);
}


static const int8_t MOST_SIGNIFICANT_BITS[16] = {
	-1, 0, 1, 1,
	2, 2, 2, 2,
	3, 3, 3, 3,
	3, 3, 3, 3,
};

static inline int most_significant_bit(uint8_t bits) {
	if (bits > 15) {
		return MOST_SIGNIFICANT_BITS[bits >> 4] + 4;
	}
	return MOST_SIGNIFICANT_BITS[bits];
}

static inline uint8_t getTools_forInstruction(AlifCodeObject* code, AlifInterpreter* interp, int i, int event)
{
	uint8_t tools;
	if (event >= 15) {
		event = ALIF_MONITORING_EVENT_CALL;
	}
	if (ALIF_MONITORING_IS_INSTRUMENTED_EVENT(event)) {
		if (code->coMonitoring->tools) {
			tools = code->coMonitoring->tools[i];
		}
		else {
			tools = code->coMonitoring->active_monitors.tools[event];
		}
	}
	else {
		tools = interp->monitors.tools[event];
	}
	return tools;
}

static int
call_instrumentation_vector(
	AlifThread* tstate, int event,
	AlifInterpreterFrame* frame, AlifCodeUnit* instr, int64_t nargs, AlifObject* args[])
{
	if (tstate->tracing) {
		return 0;
	}
	AlifCodeObject* code = (AlifCodeObject*)frame->executable;
	args[1] = (AlifObject*)code;
	int offset = (int)(instr - ALIFCODE_CODE(code));
	int bytes_offset = offset * (int)sizeof(AlifCodeUnit);
	AlifObject* offset_obj = alifInteger_fromLongLong(bytes_offset);
	if (offset_obj == NULL) {
		return -1;
	}
	args[2] = offset_obj;
	AlifInterpreter* interp = tstate->interpreter;
	uint8_t tools = getTools_forInstruction(code, interp, offset, event);
	size_t nargsf = (size_t)nargs | ALIF_VECTORCALL_ARGUMENTS_OFFSET;
	AlifObject** callargs = &args[1];
	int err = 0;
	while (tools) {
		int tool = most_significant_bit(tools);
		tools ^= (1 << tool);
		int res = call_one_instrument(interp, tstate, callargs, nargsf, tool, event);
		if (res == 0) {
		}
		else if (res < 0) {
			err = -1;
			break;
		}
		else {
			if (!ALIF_MONITORING_IS_INSTRUMENTED_EVENT(event)) {
				ALIF_CLEAR(interp->monitoringCallables[tool][event]);
				err = -1;
				break;
			}
			else {
				//LOCK_CODE(code);
				remove_tools(code, offset, event, 1 << tool);
				//UNLOCK_CODE();
			}
		}
	}
	ALIF_DECREF(offset_obj);
	return err;
}

static void callInstrumentation_vectorProtected(
	AlifThread* tstate, int event,
	AlifInterpreterFrame* frame, AlifCodeUnit* instr, int64_t nargs, AlifObject* args[])
{
	//AlifObject* exc = _PyErr_GetRaisedException(tstate);
	int err = call_instrumentation_vector(tstate, event, frame, instr, nargs, args);
	if (err) {
		//ALIF_XDECREF(exc);
	}
	//else {
		//alifErr_SetRaisedException(tstate, exc);
	//}
}

void alifCall_instrumentationExc2(
	AlifThread* tstate, int event,
	AlifInterpreterFrame* frame, AlifCodeUnit* instr, AlifObject* arg0, AlifObject* arg1)
{
	AlifObject* args[5] = { NULL, NULL, NULL, arg0, arg1 };
	callInstrumentation_vectorProtected(tstate, event, frame, instr, 4, args);
}
