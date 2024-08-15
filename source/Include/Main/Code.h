#pragma once


#define  ALIF_MONITORING_LOCAL_EVENTS 10
/* Count of all "real" monitoring events (not derived from other events) */
#define ALIF_MONITORING_UNGROUPED_EVENTS 15
/* Count of all  monitoring events */
#define ALIF_MONITORING_EVENTS 17

class AlifLocalMonitors {
public:
	uint8_t tools[ALIF_MONITORING_LOCAL_EVENTS];
} ;

class AlifGlobalMonitors {
public:
	uint8_t tools[ALIF_MONITORING_UNGROUPED_EVENTS];
} ;

class AlifCoLineInstrumentationData {
public:
	uint8_t original_opcode;
	int8_t line_delta;
} ;

class AlifCoMonitoringData{
public:
	/* Monitoring specific to this code object */
	AlifLocalMonitors local_monitors;
	/* Monitoring that is active on this code object */
	AlifLocalMonitors active_monitors;
	/* The tools that are to be notified for events for the matching code unit */
	uint8_t* tools;
	/* Information to support line events */
	AlifCoLineInstrumentationData* lines;
	/* The tools that are to be notified for line events for the matching code unit */
	uint8_t* line_tools;
	/* Information to support instruction events */
	/* The underlying instructions, which can themselves be instrumented */
	uint8_t* per_instruction_opcodes;
	/* The tools that are to be notified for instruction events for the matching code unit */
	uint8_t* per_instruction_tools;
} ;





class AlifCodeObject {  
public:
	ALIFOBJECT_VAR_HEAD;
	AlifObject* consts{};
	AlifObject* names{};

	AlifIntT flags;

	AlifIntT args{};
	AlifIntT posOnlyArgCount;
	AlifIntT kwOnlyArgCount;
	AlifIntT stackSize{};
	AlifIntT firstLineNo{};
	AlifIntT nLocalsPlus{};
	AlifIntT frameSize{};
	AlifIntT nLocals{};
	AlifIntT version{};

	AlifObject* localsPlusNames{};
	AlifObject* localsPlusKinds{};

	AlifObject* fileName{};
	AlifObject* name{};
	AlifObject* qualName{};
	AlifObject* lineTable{};
	AlifCoMonitoringData* coMonitoring; /* Monitoring data */              

	AlifIntT firstTraceable{};

	wchar_t codeAdaptive[1]; // changed to wchar_t
};

#define CO_OPTIMIZED    0x0001
#define CO_NEWLOCALS    0x0002
#define CO_VARARGS      0x0004
#define CO_VARKEYWORDS  0x0008
#define CO_NESTED       0x0010
#define CO_GENERATOR    0x0020

#define CO_COROUTINE            0x0080
#define CO_ITERABLE_COROUTINE   0x0100
#define CO_ASYNC_GENERATOR      0x0200


#define CO_FUTURE_DIVISION      0x20000
#define CO_FUTURE_ABSOLUTE_IMPORT 0x40000 /* do absolute imports by default */
#define CO_FUTURE_WITH_STATEMENT  0x80000
#define CO_FUTURE_PRINT_FUNCTION  0x100000
#define CO_FUTURE_UNICODE_LITERALS 0x200000

#define CO_FUTURE_BARRY_AS_BDFL  0x400000
#define CO_FUTURE_GENERATOR_STOP  0x800000
#define CO_FUTURE_ANNOTATIONS    0x1000000

#define CO_NO_MONITORING_EVENTS 0x2000000



/* Masks for flags above */
#define CO_OPTIMIZED    0x0001
#define CO_NEWLOCALS    0x0002
#define CO_VARARGS      0x0004
#define CO_VARKEYWORDS  0x0008
#define CO_NESTED       0x0010
#define CO_GENERATOR    0x0020

#define CO_MAXBLOCKS 21 /* Max static block nesting within a function */ 

extern AlifTypeObject _alifCodeType_;


#define ALIFCODE_CODE(_co) ALIF_RVALUE((AlifCodeUnit*)(_co)->codeAdaptive)
