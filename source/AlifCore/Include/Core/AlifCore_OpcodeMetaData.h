#pragma once

#include "OpcodeIDs.h"


 // 20
#define IS_PSEUDO_INSTR(_op)  ( \
    ((_op) == LOAD_CLOSURE) or \
    ((_op) == STORE_FAST_MAYBE_NULL) or \
    ((_op) == JUMP) or \
    ((_op) == JUMP_NO_INTERRUPT) or \
    ((_op) == SETUP_FINALLY) or \
    ((_op) == SETUP_CLEANUP) or \
    ((_op) == SETUP_WITH) or \
    ((_op) == POP_BLOCK) or \
    0)


extern AlifIntT _alifOpcode_numPopped(AlifIntT, AlifIntT); // 32
#ifdef NEED_OPCODE_METADATA
AlifIntT _alifOpcode_numPopped(AlifIntT _opcode, AlifIntT _oparg) {
	switch (_opcode) {
	case BINARY_OP:
		return 2;
	//case BINARY_OP_ADD_FLOAT:
	//	return 2;
	//case BINARY_OP_ADD_INT:
	//	return 2;
	//case BINARY_OP_ADD_UNICODE:
	//	return 2;
	//case BINARY_OP_INPLACE_ADD_UNICODE:
	//	return 2;
	//case BINARY_OP_MULTIPLY_FLOAT:
	//	return 2;
	//case BINARY_OP_MULTIPLY_INT:
	//	return 2;
	//case BINARY_OP_SUBTRACT_FLOAT:
	//	return 2;
	//case BINARY_OP_SUBTRACT_INT:
	//	return 2;
	//case BINARY_SLICE:
	//	return 3;
	//case BINARY_SUBSCR:
	//	return 2;
	//case BINARY_SUBSCR_DICT:
	//	return 2;
	//case BINARY_SUBSCR_GETITEM:
	//	return 2;
	//case BINARY_SUBSCR_LIST_INT:
	//	return 2;
	//case BINARY_SUBSCR_STR_INT:
	//	return 2;
	//case BINARY_SUBSCR_TUPLE_INT:
		return 2;
	case BUILD_LIST:
		return _oparg;
	case BUILD_MAP:
		return _oparg * 2;
	//case BUILD_SET:
	//	return _oparg;
	//case BUILD_SLICE:
	//	return 2 + ((_oparg == 3) ? 1 : 0);
	case BUILD_STRING:
		return _oparg;
	case BUILD_TUPLE:
		return _oparg;
	case CACHE:
		return 0;
	case CALL:
		return 2 + _oparg;
	//case CALL_ALLOC_AND_ENTER_INIT:
	//	return 2 + _oparg;
	//case CALL_BOUND_METHOD_EXACT_ARGS:
	//	return 2 + _oparg;
	//case CALL_BOUND_METHOD_GENERAL:
	//	return 2 + _oparg;
	//case CALL_BUILTIN_CLASS:
	//	return 2 + _oparg;
	//case CALL_BUILTIN_FAST:
	//	return 2 + _oparg;
	//case CALL_BUILTIN_FAST_WITH_KEYWORDS:
	//	return 2 + _oparg;
	//case CALL_BUILTIN_O:
	//	return 2 + _oparg;
	//case CALL_FUNCTION_EX:
	//	return 3 + (_oparg & 1);
	case CALL_INTRINSIC_1:
		return 1;
	//case CALL_INTRINSIC_2:
	//	return 2;
	//case CALL_ISINSTANCE:
	//	return 2 + _oparg;
	//case CALL_KW:
	//	return 3 + _oparg;
	//case CALL_KW_BOUND_METHOD:
	//	return 3 + _oparg;
	//case CALL_KW_NON_PY:
	//	return 3 + _oparg;
	//case CALL_KW_PY:
	//	return 3 + _oparg;
	//case CALL_LEN:
	//	return 2 + _oparg;
	//case CALL_LIST_APPEND:
	//	return 3;
	//case CALL_METHOD_DESCRIPTOR_FAST:
	//	return 2 + _oparg;
	//case CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS:
	//	return 2 + _oparg;
	//case CALL_METHOD_DESCRIPTOR_NOARGS:
	//	return 2 + _oparg;
	//case CALL_METHOD_DESCRIPTOR_O:
	//	return 2 + _oparg;
	//case CALL_NON_PY_GENERAL:
	//	return 2 + _oparg;
	//case CALL_PY_EXACT_ARGS:
	//	return 2 + _oparg;
	//case CALL_PY_GENERAL:
	//	return 2 + _oparg;
	//case CALL_STR_1:
	//	return 3;
	//case CALL_TUPLE_1:
	//	return 3;
	//case CALL_TYPE_1:
	//	return 3;
	//case CHECK_EG_MATCH:
	//	return 2;
	//case CHECK_EXC_MATCH:
	//	return 2;
	//case CLEANUP_THROW:
		return 3;
	case COMPARE_OP:
		return 2;
	//case COMPARE_OP_FLOAT:
	//	return 2;
	//case COMPARE_OP_INT:
	//	return 2;
	//case COMPARE_OP_STR:
	//	return 2;
	//case CONTAINS_OP:
	//	return 2;
	//case CONTAINS_OP_DICT:
	//	return 2;
	//case CONTAINS_OP_SET:
	//	return 2;
	case CONVERT_VALUE:
		return 1;
	case COPY:
		return 1 + (_oparg - 1);
	case COPY_FREE_VARS:
		return 0;
	case DELETE_ATTR:
		return 1;
	case DELETE_DEREF:
		return 0;
	case DELETE_FAST:
		return 0;
	case DELETE_GLOBAL:
		return 0;
	case DELETE_NAME:
		return 0;
	//case DELETE_SUBSCR:
	//	return 2;
	//case DICT_MERGE:
	//	return 5 + (_oparg - 1);
	case DICT_UPDATE:
		return 2 + (_oparg - 1);
	//case END_ASYNC_FOR:
	//	return 2;
	//case END_FOR:
	//	return 1;
	//case END_SEND:
	//	return 2;
	//case ENTER_EXECUTOR:
	//	return 0;
	//case EXIT_INIT_CHECK:
	//	return 1;
	case EXTENDED_ARG:
		return 0;
	case FORMAT_SIMPLE:
		return 1;
	case FORMAT_WITH_SPEC:
		return 2;
	case FOR_ITER:
		return 1;
	//case FOR_ITER_GEN:
	//	return 1;
	//case FOR_ITER_LIST:
	//	return 1;
	//case FOR_ITER_RANGE:
	//	return 1;
	//case FOR_ITER_TUPLE:
	//	return 1;
	//case GET_AITER:
	//	return 1;
	//case GET_ANEXT:
	//	return 1;
	//case GET_AWAITABLE:
	//	return 1;
	//case GET_ITER:
	//	return 1;
	//case GET_LEN:
	//	return 1;
	//case GET_YIELD_FROM_ITER:
	//	return 1;
	//case IMPORT_FROM:
	//	return 1;
	//case IMPORT_NAME:
	//	return 2;
	//case INSTRUMENTED_CALL:
	//	return 2 + _oparg;
	//case INSTRUMENTED_CALL_FUNCTION_EX:
	//	return 0;
	//case INSTRUMENTED_CALL_KW:
	//	return 0;
	//case INSTRUMENTED_END_FOR:
	//	return 2;
	//case INSTRUMENTED_END_SEND:
	//	return 2;
	//case INSTRUMENTED_FOR_ITER:
	//	return 0;
	//case INSTRUMENTED_INSTRUCTION:
	//	return 0;
	//case INSTRUMENTED_JUMP_BACKWARD:
	//	return 0;
	//case INSTRUMENTED_JUMP_FORWARD:
	//	return 0;
	//case INSTRUMENTED_LINE:
	//	return 0;
	//case INSTRUMENTED_LOAD_SUPER_ATTR:
	//	return 3;
	//case INSTRUMENTED_POP_JUMP_IF_FALSE:
	//	return 0;
	//case INSTRUMENTED_POP_JUMP_IF_NONE:
	//	return 0;
	//case INSTRUMENTED_POP_JUMP_IF_NOT_NONE:
	//	return 0;
	//case INSTRUMENTED_POP_JUMP_IF_TRUE:
	//	return 0;
	//case INSTRUMENTED_RESUME:
	//	return 0;
	//case INSTRUMENTED_RETURN_CONST:
	//	return 0;
	//case INSTRUMENTED_RETURN_VALUE:
	//	return 1;
	//case INSTRUMENTED_YIELD_VALUE:
	//	return 1;
	//case INTERPRETER_EXIT:
	//	return 1;
	case IS_OP:
		return 2;
	case JUMP:
		return 0;
	case JUMP_BACKWARD:
		return 0;
	case JUMP_BACKWARD_NO_INTERRUPT:
		return 0;
	case JUMP_FORWARD:
		return 0;
	case JUMP_NO_INTERRUPT:
		return 0;
	case LIST_APPEND:
		return 2 + (_oparg - 1);
	case LIST_EXTEND:
		return 2 + (_oparg - 1);
	case LOAD_ATTR:
		return 1;
	//case LOAD_ATTR_CLASS:
	//	return 1;
	//case LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN:
	//	return 1;
	//case LOAD_ATTR_INSTANCE_VALUE:
	//	return 1;
	//case LOAD_ATTR_METHOD_LAZY_DICT:
	//	return 1;
	//case LOAD_ATTR_METHOD_NO_DICT:
	//	return 1;
	//case LOAD_ATTR_METHOD_WITH_VALUES:
	//	return 1;
	//case LOAD_ATTR_MODULE:
	//	return 1;
	//case LOAD_ATTR_NONDESCRIPTOR_NO_DICT:
	//	return 1;
	//case LOAD_ATTR_NONDESCRIPTOR_WITH_VALUES:
	//	return 1;
	//case LOAD_ATTR_PROPERTY:
	//	return 1;
	//case LOAD_ATTR_SLOT:
	//	return 1;
	//case LOAD_ATTR_WITH_HINT:
	//	return 1;
	//case LOAD_BUILD_CLASS:
	//	return 0;
	//case LOAD_CLOSURE:
	//	return 0;
	//case LOAD_COMMON_CONSTANT:
	//	return 0;
	case LOAD_CONST:
		return 0;
	case LOAD_DEREF:
		return 0;
	case LOAD_FAST:
		return 0;
	case LOAD_FAST_AND_CLEAR:
		return 0;
	case LOAD_FAST_CHECK:
		return 0;
	//case LOAD_FAST_LOAD_FAST:
	//	return 0;
	case LOAD_FROM_DICT_OR_DEREF:
		return 1;
	case LOAD_FROM_DICT_OR_GLOBALS:
		return 1;
	case LOAD_GLOBAL:
		return 0;
	//case LOAD_GLOBAL_BUILTIN:
	//	return 0;
	//case LOAD_GLOBAL_MODULE:
	//	return 0;
	case LOAD_LOCALS:
		return 0;
	case LOAD_NAME:
		return 0;
	//case LOAD_SPECIAL:
	//	return 1;
	case LOAD_SUPER_ATTR:
		return 3;
	//case LOAD_SUPER_ATTR_ATTR:
	//	return 3;
	//case LOAD_SUPER_ATTR_METHOD:
	//	return 3;
	case MAKE_CELL:
		return 0;
	//case MAKE_FUNCTION:
	//	return 1;
	case MAP_ADD:
		return 3 + (_oparg - 1);
	//case MATCH_CLASS:
	//	return 3;
	//case MATCH_KEYS:
	//	return 2;
	//case MATCH_MAPPING:
	//	return 1;
	//case MATCH_SEQUENCE:
	//	return 1;
	case NOP:
		return 0;
	case POP_BLOCK:
		return 0;
	//case POP_EXCEPT:
	//	return 1;
	case POP_JUMP_IF_FALSE:
		return 1;
	case POP_JUMP_IF_NONE:
		return 1;
	case POP_JUMP_IF_NOT_NONE:
		return 1;
	case POP_JUMP_IF_TRUE:
		return 1;
	case POP_TOP:
		return 1;
	//case PUSH_EXC_INFO:
	//	return 1;
	case PUSH_NULL:
		return 0;
	case RAISE_VARARGS:
		return _oparg;
	case RERAISE:
		return 1 + _oparg;
	//case RESERVED:
	//	return 0;
	case RESUME:
		return 0;
	//case RESUME_CHECK:
	//	return 0;
	case RETURN_CONST:
		return 0;
	case RETURN_GENERATOR:
		return 0;
	case RETURN_VALUE:
		return 1;
	//case SEND:
	//	return 2;
	//case SEND_GEN:
	//	return 2;
	//case SETUP_ANNOTATIONS:
	//	return 0;
	case SETUP_CLEANUP:
		return 0;
	case SETUP_FINALLY:
		return 0;
	case SETUP_WITH:
		return 0;
	case SET_ADD:
		return 2 + (_oparg - 1);
	//case SET_FUNCTION_ATTRIBUTE:
	//	return 2;
	//case SET_UPDATE:
	//	return 2 + (_oparg - 1);
	case STORE_ATTR:
		return 2;
	//case STORE_ATTR_INSTANCE_VALUE:
	//	return 2;
	//case STORE_ATTR_SLOT:
	//	return 2;
	//case STORE_ATTR_WITH_HINT:
	//	return 2;
	case STORE_DEREF:
		return 1;
	case STORE_FAST:
		return 1;
	//case STORE_FAST_LOAD_FAST:
	//	return 1;
	case STORE_FAST_MAYBE_NULL:
		return 1;
	case STORE_FAST_STORE_FAST:
		return 2;
	case STORE_GLOBAL:
		return 1;
	case STORE_NAME:
		return 1;
	//case STORE_SLICE:
	//	return 4;
	//case STORE_SUBSCR:
	//	return 3;
	//case STORE_SUBSCR_DICT:
	//	return 3;
	//case STORE_SUBSCR_LIST_INT:
	//	return 3;
	case SWAP:
		return 2 + (_oparg - 2);
	case TO_BOOL:
		return 1;
	//case TO_BOOL_ALWAYS_TRUE:
	//	return 1;
	//case TO_BOOL_BOOL:
	//	return 1;
	//case TO_BOOL_INT:
	//	return 1;
	//case TO_BOOL_LIST:
	//	return 1;
	//case TO_BOOL_NONE:
	//	return 1;
	//case TO_BOOL_STR:
	//	return 1;
	case UNARY_INVERT:
		return 1;
	case UNARY_NEGATIVE:
		return 1;
	case UNARY_NOT:
		return 1;
	case UNPACK_EX:
		return 1;
	case UNPACK_SEQUENCE:
		return 1;
	//case UNPACK_SEQUENCE_LIST:
	//	return 1;
	//case UNPACK_SEQUENCE_TUPLE:
	//	return 1;
	//case UNPACK_SEQUENCE_TWO_TUPLE:
	//	return 1;
	//case WITH_EXCEPT_START:
	//	return 5;
	//case YIELD_VALUE:
	//	return 1;
	//case _DO_CALL_FUNCTION_EX:
	//	return 3 + (_oparg & 1);
	default:
		return -1;
	}
}

#endif








extern AlifIntT _alifOpcode_numPushed(AlifIntT _opcode, AlifIntT _oparg); // 487
#ifdef NEED_OPCODE_METADATA
AlifIntT _alifOpcode_numPushed(AlifIntT _opcode, AlifIntT _oparg) {
	switch (_opcode) {
	case BINARY_OP:
		return 1;
	//case BINARY_OP_ADD_FLOAT:
	//	return 1;
	//case BINARY_OP_ADD_INT:
	//	return 1;
	//case BINARY_OP_ADD_UNICODE:
	//	return 1;
	//case BINARY_OP_INPLACE_ADD_UNICODE:
	//	return 0;
	//case BINARY_OP_MULTIPLY_FLOAT:
	//	return 1;
	//case BINARY_OP_MULTIPLY_INT:
	//	return 1;
	//case BINARY_OP_SUBTRACT_FLOAT:
	//	return 1;
	//case BINARY_OP_SUBTRACT_INT:
	//	return 1;
	//case BINARY_SLICE:
	//	return 1;
	//case BINARY_SUBSCR:
	//	return 1;
	//case BINARY_SUBSCR_DICT:
	//	return 1;
	//case BINARY_SUBSCR_GETITEM:
	//	return 0;
	//case BINARY_SUBSCR_LIST_INT:
	//	return 1;
	//case BINARY_SUBSCR_STR_INT:
	//	return 1;
	//case BINARY_SUBSCR_TUPLE_INT:
	//	return 1;
	case BUILD_LIST:
		return 1;
	case BUILD_MAP:
		return 1;
	//case BUILD_SET:
	//	return 1;
	//case BUILD_SLICE:
	//	return 1;
	case BUILD_STRING:
		return 1;
	case BUILD_TUPLE:
		return 1;
	case CACHE:
		return 0;
	case CALL:
		return 1;
	//case CALL_ALLOC_AND_ENTER_INIT:
	//	return 0;
	//case CALL_BOUND_METHOD_EXACT_ARGS:
	//	return 0;
	//case CALL_BOUND_METHOD_GENERAL:
	//	return 0;
	//case CALL_BUILTIN_CLASS:
	//	return 1;
	//case CALL_BUILTIN_FAST:
	//	return 1;
	//case CALL_BUILTIN_FAST_WITH_KEYWORDS:
	//	return 1;
	//case CALL_BUILTIN_O:
	//	return 1;
	//case CALL_FUNCTION_EX:
	//	return 1;
	case CALL_INTRINSIC_1:
		return 1;
	//case CALL_INTRINSIC_2:
	//	return 1;
	//case CALL_ISINSTANCE:
	//	return 1;
	//case CALL_KW:
	//	return 1;
	//case CALL_KW_BOUND_METHOD:
	//	return 0;
	//case CALL_KW_NON_PY:
	//	return 1;
	//case CALL_KW_PY:
	//	return 0;
	//case CALL_LEN:
	//	return 1;
	//case CALL_LIST_APPEND:
	//	return 0;
	//case CALL_METHOD_DESCRIPTOR_FAST:
	//	return 1;
	//case CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS:
	//	return 1;
	//case CALL_METHOD_DESCRIPTOR_NOARGS:
	//	return 1;
	//case CALL_METHOD_DESCRIPTOR_O:
	//	return 1;
	//case CALL_NON_PY_GENERAL:
	//	return 1;
	//case CALL_PY_EXACT_ARGS:
	//	return 0;
	//case CALL_PY_GENERAL:
	//	return 0;
	//case CALL_STR_1:
	//	return 1;
	//case CALL_TUPLE_1:
	//	return 1;
	//case CALL_TYPE_1:
	//	return 1;
	//case CHECK_EG_MATCH:
	//	return 2;
	//case CHECK_EXC_MATCH:
	//	return 2;
	//case CLEANUP_THROW:
	//	return 2;
	case COMPARE_OP:
		return 1;
	//case COMPARE_OP_FLOAT:
	//	return 1;
	//case COMPARE_OP_INT:
	//	return 1;
	//case COMPARE_OP_STR:
	//	return 1;
	case CONTAINS_OP:
		return 1;
	//case CONTAINS_OP_DICT:
	//	return 1;
	//case CONTAINS_OP_SET:
	//	return 1;
	case CONVERT_VALUE:
		return 1;
	case COPY:
		return 2 + (_oparg - 1);
	case COPY_FREE_VARS:
		return 0;
	case DELETE_ATTR:
		return 0;
	case DELETE_DEREF:
		return 0;
	case DELETE_FAST:
		return 0;
	case DELETE_GLOBAL:
		return 0;
	case DELETE_NAME:
		return 0;
	//case DELETE_SUBSCR:
	//	return 0;
	//case DICT_MERGE:
	//	return 4 + (_oparg - 1);
	case DICT_UPDATE:
		return 1 + (_oparg - 1);
	//case END_ASYNC_FOR:
	//	return 0;
	//case END_FOR:
	//	return 0;
	//case END_SEND:
	//	return 1;
	//case ENTER_EXECUTOR:
	//	return 0;
	//case EXIT_INIT_CHECK:
	//	return 0;
	case EXTENDED_ARG:
		return 0;
	case FORMAT_SIMPLE:
		return 1;
	case FORMAT_WITH_SPEC:
		return 1;
	case FOR_ITER:
		return 2;
	//case FOR_ITER_GEN:
	//	return 1;
	//case FOR_ITER_LIST:
	//	return 2;
	//case FOR_ITER_RANGE:
	//	return 2;
	//case FOR_ITER_TUPLE:
	//	return 2;
	//case GET_AITER:
	//	return 1;
	//case GET_ANEXT:
	//	return 2;
	//case GET_AWAITABLE:
	//	return 1;
	//case GET_ITER:
	//	return 1;
	//case GET_LEN:
	//	return 2;
	//case GET_YIELD_FROM_ITER:
	//	return 1;
	//case IMPORT_FROM:
	//	return 2;
	//case IMPORT_NAME:
	//	return 1;
	//case INSTRUMENTED_CALL:
	//	return 1;
	//case INSTRUMENTED_CALL_FUNCTION_EX:
	//	return 0;
	//case INSTRUMENTED_CALL_KW:
	//	return 0;
	//case INSTRUMENTED_END_FOR:
	//	return 1;
	//case INSTRUMENTED_END_SEND:
	//	return 1;
	//case INSTRUMENTED_FOR_ITER:
	//	return 0;
	//case INSTRUMENTED_INSTRUCTION:
	//	return 0;
	//case INSTRUMENTED_JUMP_BACKWARD:
	//	return 0;
	//case INSTRUMENTED_JUMP_FORWARD:
	//	return 0;
	//case INSTRUMENTED_LINE:
	//	return 0;
	//case INSTRUMENTED_LOAD_SUPER_ATTR:
	//	return 1 + (_oparg & 1);
	//case INSTRUMENTED_POP_JUMP_IF_FALSE:
	//	return 0;
	//case INSTRUMENTED_POP_JUMP_IF_NONE:
	//	return 0;
	//case INSTRUMENTED_POP_JUMP_IF_NOT_NONE:
	//	return 0;
	//case INSTRUMENTED_POP_JUMP_IF_TRUE:
	//	return 0;
	//case INSTRUMENTED_RESUME:
	//	return 0;
	//case INSTRUMENTED_RETURN_CONST:
	//	return 1;
	//case INSTRUMENTED_RETURN_VALUE:
	//	return 1;
	//case INSTRUMENTED_YIELD_VALUE:
	//	return 1;
	//case INTERPRETER_EXIT:
	//	return 0;
	case IS_OP:
		return 1;
	case JUMP:
		return 0;
	case JUMP_BACKWARD:
		return 0;
	case JUMP_BACKWARD_NO_INTERRUPT:
		return 0;
	case JUMP_FORWARD:
		return 0;
	case JUMP_NO_INTERRUPT:
		return 0;
	case LIST_APPEND:
		return 1 + (_oparg - 1);
	case LIST_EXTEND:
		return 1 + (_oparg - 1);
	case LOAD_ATTR:
		return 1 + (_oparg & 1);
	//case LOAD_ATTR_CLASS:
	//	return 1 + (_oparg & 1);
	//case LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN:
	//	return 1;
	//case LOAD_ATTR_INSTANCE_VALUE:
	//	return 1 + (_oparg & 1);
	//case LOAD_ATTR_METHOD_LAZY_DICT:
	//	return 2;
	//case LOAD_ATTR_METHOD_NO_DICT:
	//	return 2;
	//case LOAD_ATTR_METHOD_WITH_VALUES:
	//	return 2;
	//case LOAD_ATTR_MODULE:
	//	return 1 + (_oparg & 1);
	//case LOAD_ATTR_NONDESCRIPTOR_NO_DICT:
	//	return 1;
	//case LOAD_ATTR_NONDESCRIPTOR_WITH_VALUES:
	//	return 1;
	//case LOAD_ATTR_PROPERTY:
	//	return 0;
	//case LOAD_ATTR_SLOT:
	//	return 1 + (_oparg & 1);
	//case LOAD_ATTR_WITH_HINT:
	//	return 1 + (_oparg & 1);
	//case LOAD_BUILD_CLASS:
	//	return 1;
	//case LOAD_CLOSURE:
	//	return 1;
	//case LOAD_COMMON_CONSTANT:
	//	return 1;
	case LOAD_CONST:
		return 1;
	case LOAD_DEREF:
		return 1;
	case LOAD_FAST:
		return 1;
	case LOAD_FAST_AND_CLEAR:
		return 1;
	case LOAD_FAST_CHECK:
		return 1;
	//case LOAD_FAST_LOAD_FAST:
	//	return 2;
	case LOAD_FROM_DICT_OR_DEREF:
		return 1;
	case LOAD_FROM_DICT_OR_GLOBALS:
		return 1;
	case LOAD_GLOBAL:
		return 1 + (_oparg & 1);
	//case LOAD_GLOBAL_BUILTIN:
	//	return 1 + (_oparg & 1);
	//case LOAD_GLOBAL_MODULE:
	//	return 1 + (_oparg & 1);
	case LOAD_LOCALS:
		return 1;
	case LOAD_NAME:
		return 1;
	//case LOAD_SPECIAL:
	//	return 2;
	case LOAD_SUPER_ATTR:
		return 1 + (_oparg & 1);
	//case LOAD_SUPER_ATTR_ATTR:
	//	return 1;
	//case LOAD_SUPER_ATTR_METHOD:
	//	return 2;
	case MAKE_CELL:
		return 0;
	//case MAKE_FUNCTION:
	//	return 1;
	case MAP_ADD:
		return 1 + (_oparg - 1);
	//case MATCH_CLASS:
	//	return 1;
	//case MATCH_KEYS:
	//	return 3;
	//case MATCH_MAPPING:
	//	return 2;
	//case MATCH_SEQUENCE:
	//	return 2;
	case NOP:
		return 0;
	case POP_BLOCK:
		return 0;
	//case POP_EXCEPT:
	//	return 0;
	case POP_JUMP_IF_FALSE:
		return 0;
	case POP_JUMP_IF_NONE:
		return 0;
	case POP_JUMP_IF_NOT_NONE:
		return 0;
	case POP_JUMP_IF_TRUE:
		return 0;
	case POP_TOP:
		return 0;
	//case PUSH_EXC_INFO:
	//	return 2;
	case PUSH_NULL:
		return 1;
	case RAISE_VARARGS:
		return 0;
	case RERAISE:
		return _oparg;
	//case RESERVED:
	//	return 0;
	case RESUME:
		return 0;
	//case RESUME_CHECK:
	//	return 0;
	case RETURN_CONST:
		return 1;
	case RETURN_GENERATOR:
		return 1;
	case RETURN_VALUE:
		return 1;
	//case SEND:
	//	return 2;
	//case SEND_GEN:
	//	return 1;
	//case SETUP_ANNOTATIONS:
	//	return 0;
	case SETUP_CLEANUP:
		return 2;
	case SETUP_FINALLY:
		return 1;
	case SETUP_WITH:
		return 1;
	case SET_ADD:
		return 1 + (_oparg - 1);
	//case SET_FUNCTION_ATTRIBUTE:
	//	return 1;
	//case SET_UPDATE:
	//	return 1 + (_oparg - 1);
	case STORE_ATTR:
		return 0;
	//case STORE_ATTR_INSTANCE_VALUE:
	//	return 0;
	//case STORE_ATTR_SLOT:
	//	return 0;
	//case STORE_ATTR_WITH_HINT:
	//	return 0;
	case STORE_DEREF:
		return 0;
	case STORE_FAST:
		return 0;
	//case STORE_FAST_LOAD_FAST:
	//	return 1;
	case STORE_FAST_MAYBE_NULL:
		return 0;
	case STORE_FAST_STORE_FAST:
		return 0;
	case STORE_GLOBAL:
		return 0;
	case STORE_NAME:
		return 0;
	//case STORE_SLICE:
	//	return 0;
	//case STORE_SUBSCR:
	//	return 0;
	//case STORE_SUBSCR_DICT:
	//	return 0;
	//case STORE_SUBSCR_LIST_INT:
	//	return 0;
	case SWAP:
		return 2 + (_oparg - 2);
	case TO_BOOL:
		return 1;
	//case TO_BOOL_ALWAYS_TRUE:
	//	return 1;
	//case TO_BOOL_BOOL:
	//	return 1;
	//case TO_BOOL_INT:
	//	return 1;
	//case TO_BOOL_LIST:
	//	return 1;
	//case TO_BOOL_NONE:
	//	return 1;
	//case TO_BOOL_STR:
	//	return 1;
	case UNARY_INVERT:
		return 1;
	case UNARY_NEGATIVE:
		return 1;
	case UNARY_NOT:
		return 1;
	case UNPACK_EX:
		return 1 + (_oparg & 0xFF) + (_oparg >> 8);
	case UNPACK_SEQUENCE:
		return _oparg;
	//case UNPACK_SEQUENCE_LIST:
	//	return _oparg;
	//case UNPACK_SEQUENCE_TUPLE:
	//	return _oparg;
	//case UNPACK_SEQUENCE_TWO_TUPLE:
	//	return 2;
	//case WITH_EXCEPT_START:
	//	return 6;
	//case YIELD_VALUE:
	//	return 1;
	//case _DO_CALL_FUNCTION_EX:
	//	return 1;
	default:
		return -1;
	}
}

#endif



enum InstructionFormat { // 942
	Instr_FMT_IB = 1,
	Instr_FMT_IBC = 2,
	Instr_FMT_IBC00 = 3,
	Instr_FMT_IBC000 = 4,
	Instr_FMT_IBC00000000 = 5,
	Instr_FMT_IX = 6,
	Instr_FMT_IXC = 7,
	Instr_FMT_IXC00 = 8,
	Instr_FMT_IXC000 = 9,
};




 // 958
#define HAS_ARG_FLAG (1)
#define HAS_CONST_FLAG (2)
#define HAS_NAME_FLAG (4)
#define HAS_JUMP_FLAG (8)
#define HAS_FREE_FLAG (16)
#define HAS_LOCAL_FLAG (32)
#define HAS_EVAL_BREAK_FLAG (64)
#define HAS_DEOPT_FLAG (128)
#define HAS_ERROR_FLAG (256)
#define HAS_ESCAPES_FLAG (512)
#define HAS_EXIT_FLAG (1024)
#define HAS_PURE_FLAG (2048)
#define HAS_PASSTHROUGH_FLAG (4096)
#define HAS_OPARG_AND_1_FLAG (8192)
#define HAS_ERROR_NO_POP_FLAG (16384)
#define OPCODE_HAS_ARG(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_ARG_FLAG))
#define OPCODE_HAS_CONST(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_CONST_FLAG))
#define OPCODE_HAS_NAME(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_NAME_FLAG))
#define OPCODE_HAS_JUMP(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_JUMP_FLAG))
#define OPCODE_HAS_FREE(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_FREE_FLAG))
#define OPCODE_HAS_LOCAL(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_LOCAL_FLAG))
#define OPCODE_HAS_EVAL_BREAK(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_EVAL_BREAK_FLAG))
#define OPCODE_HAS_DEOPT(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_DEOPT_FLAG))
#define OPCODE_HAS_ERROR(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_ERROR_FLAG))
#define OPCODE_HAS_ESCAPES(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_ESCAPES_FLAG))
#define OPCODE_HAS_EXIT(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_EXIT_FLAG))
#define OPCODE_HAS_PURE(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_PURE_FLAG))
#define OPCODE_HAS_PASSTHROUGH(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_PASSTHROUGH_FLAG))
#define OPCODE_HAS_OPARG_AND_1(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_OPARG_AND_1_FLAG))
#define OPCODE_HAS_ERROR_NO_POP(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_ERROR_NO_POP_FLAG))

#define OPARG_FULL 0
#define OPARG_CACHE_1 1
#define OPARG_CACHE_2 2
#define OPARG_CACHE_4 4
#define OPARG_TOP 5
#define OPARG_BOTTOM 6
#define OPARG_SAVE_RETURN_OFFSET 7
#define OPARG_REPLACED 9





class OpcodeMetadata { // 998
public:
	uint8_t validEntry{};
	int8_t instrFormat{};
	int16_t flags{};
};

extern const OpcodeMetadata _alifOpcodeOpcodeMetadata_[264]; // 1004
//#ifdef NEED_OPCODE_METADATA
const class OpcodeMetadata _alifOpcodeOpcodeMetadata_[264] = {
	{ true, InstructionFormat::Instr_FMT_IX, 0 }, // CACHE
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // BINARY_SLICE
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // BINARY_SUBSCR
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_LOCAL_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // BINARY_OP_INPLACE_ADD_UNICODE
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CHECK_EG_MATCH
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CHECK_EXC_MATCH
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CLEANUP_THROW
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // DELETE_SUBSCR
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // END_ASYNC_FOR
	{ true, InstructionFormat::Instr_FMT_IX, HAS_PURE_FLAG }, // END_FOR
	{ true, InstructionFormat::Instr_FMT_IX, HAS_PURE_FLAG }, // END_SEND
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // EXIT_INIT_CHECK
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // FORMAT_SIMPLE
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // FORMAT_WITH_SPEC
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // GET_AITER
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // GET_ANEXT
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // GET_ITER
	{ true, InstructionFormat::Instr_FMT_IX, 0 }, // RESERVED
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // GET_LEN
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // GET_YIELD_FROM_ITER
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ESCAPES_FLAG }, // INTERPRETER_EXIT
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_BUILD_CLASS
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_LOCALS
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // MAKE_FUNCTION
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // MATCH_KEYS
	{ true, InstructionFormat::Instr_FMT_IX, 0 }, // MATCH_MAPPING
	{ true, InstructionFormat::Instr_FMT_IX, 0 }, // MATCH_SEQUENCE
	{ true, InstructionFormat::Instr_FMT_IX, HAS_PURE_FLAG }, // NOP
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ESCAPES_FLAG }, // POP_EXCEPT
	{ true, InstructionFormat::Instr_FMT_IX, HAS_PURE_FLAG }, // POP_TOP
	{ true, InstructionFormat::Instr_FMT_IX, 0 }, // PUSH_EXC_INFO
	{ true, InstructionFormat::Instr_FMT_IX, HAS_PURE_FLAG }, // PUSH_NULL
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // RETURN_GENERATOR
	{ true, InstructionFormat::Instr_FMT_IX, 0 }, // RETURN_VALUE
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // SETUP_ANNOTATIONS
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // STORE_SLICE
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // STORE_SUBSCR
	{ true, InstructionFormat::Instr_FMT_IXC00, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // TO_BOOL
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // UNARY_INVERT
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // UNARY_NEGATIVE
	{ true, InstructionFormat::Instr_FMT_IX, HAS_PURE_FLAG }, // UNARY_NOT
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // WITH_EXCEPT_START
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // BINARY_OP
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG }, // BUILD_LIST
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // BUILD_MAP
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // BUILD_SET
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG }, // BUILD_SLICE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG }, // BUILD_STRING
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG }, // BUILD_TUPLE
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL_FUNCTION_EX
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_INTRINSIC_1
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_INTRINSIC_2
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL_KW
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // COMPARE_OP
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CONTAINS_OP
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG }, // CONVERT_VALUE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_PURE_FLAG }, // COPY
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG }, // COPY_FREE_VARS
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // DELETE_ATTR
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_FREE_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // DELETE_DEREF
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_LOCAL_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // DELETE_FAST
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // DELETE_GLOBAL
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // DELETE_NAME
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // DICT_MERGE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // DICT_UPDATE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG }, // EXTENDED_ARG
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // FOR_ITER
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // GET_AWAITABLE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // IMPORT_FROM
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // IMPORT_NAME
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG }, // IS_OP
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG | HAS_EVAL_BREAK_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // JUMP_BACKWARD
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG }, // JUMP_BACKWARD_NO_INTERRUPT
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG }, // JUMP_FORWARD
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG }, // LIST_APPEND
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LIST_EXTEND
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_ATTR
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG }, // LOAD_COMMON_CONSTANT
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_CONST_FLAG | HAS_PURE_FLAG }, // LOAD_CONST
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_FREE_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_DEREF
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_LOCAL_FLAG | HAS_PURE_FLAG }, // LOAD_FAST
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_LOCAL_FLAG }, // LOAD_FAST_AND_CLEAR
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_LOCAL_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_FAST_CHECK
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_LOCAL_FLAG }, // LOAD_FAST_LOAD_FAST
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_FREE_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // LOAD_FROM_DICT_OR_DEREF
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // LOAD_FROM_DICT_OR_GLOBALS
	{ true, InstructionFormat::Instr_FMT_IBC000, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_GLOBAL
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_NAME
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_SPECIAL
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_SUPER_ATTR
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_FREE_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG }, // MAKE_CELL
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // MAP_ADD
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // MATCH_CLASS
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG }, // POP_JUMP_IF_FALSE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG }, // POP_JUMP_IF_NONE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG }, // POP_JUMP_IF_NOT_NONE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG }, // POP_JUMP_IF_TRUE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // RAISE_VARARGS
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // RERAISE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_CONST_FLAG }, // RETURN_CONST
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // SEND
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // SET_ADD
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ESCAPES_FLAG }, // SET_FUNCTION_ATTRIBUTE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // SET_UPDATE
	{ true, InstructionFormat::Instr_FMT_IBC000, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // STORE_ATTR
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_FREE_FLAG | HAS_ESCAPES_FLAG }, // STORE_DEREF
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_LOCAL_FLAG }, // STORE_FAST
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_LOCAL_FLAG }, // STORE_FAST_LOAD_FAST
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_LOCAL_FLAG }, // STORE_FAST_STORE_FAST
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // STORE_GLOBAL

	{ true, InstructionFormat::Instr_FMT_IXC, HAS_EXIT_FLAG }, // BINARY_OP_ADD_FLOAT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_EXIT_FLAG | HAS_ERROR_FLAG }, // BINARY_OP_ADD_INT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_EXIT_FLAG | HAS_ERROR_FLAG }, // BINARY_OP_ADD_UNICODE
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_EXIT_FLAG }, // BINARY_OP_MULTIPLY_FLOAT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_EXIT_FLAG | HAS_ERROR_FLAG }, // BINARY_OP_MULTIPLY_INT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_EXIT_FLAG }, // BINARY_OP_SUBTRACT_FLOAT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_EXIT_FLAG | HAS_ERROR_FLAG }, // BINARY_OP_SUBTRACT_INT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // BINARY_SUBSCR_DICT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_DEOPT_FLAG | HAS_ESCAPES_FLAG }, // BINARY_SUBSCR_GETITEM
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_DEOPT_FLAG }, // BINARY_SUBSCR_LIST_INT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_DEOPT_FLAG }, // BINARY_SUBSCR_STR_INT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_DEOPT_FLAG }, // BINARY_SUBSCR_TUPLE_INT
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL_ALLOC_AND_ENTER_INIT
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG }, // CALL_BOUND_METHOD_EXACT_ARGS
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL_BOUND_METHOD_GENERAL
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_BUILTIN_CLASS
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_BUILTIN_FAST
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_BUILTIN_FAST_WITH_KEYWORDS
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_BUILTIN_O
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL_ISINSTANCE
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL_KW_BOUND_METHOD
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_KW_NON_PY
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL_KW_PY
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL_LEN
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG }, // CALL_LIST_APPEND
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_METHOD_DESCRIPTOR_FAST
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_METHOD_DESCRIPTOR_NOARGS
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_METHOD_DESCRIPTOR_O
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_NON_PY_GENERAL
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG }, // CALL_PY_EXACT_ARGS
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // CALL_PY_GENERAL
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_STR_1
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CALL_TUPLE_1
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_DEOPT_FLAG }, // CALL_TYPE_1
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_EXIT_FLAG }, // COMPARE_OP_FLOAT
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG }, // COMPARE_OP_INT
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_EXIT_FLAG }, // COMPARE_OP_STR
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CONTAINS_OP_DICT
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // CONTAINS_OP_SET
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG }, // ENTER_EXECUTOR
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_DEOPT_FLAG }, // FOR_ITER_GEN
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG | HAS_EXIT_FLAG }, // FOR_ITER_LIST
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG | HAS_EXIT_FLAG | HAS_ERROR_FLAG }, // FOR_ITER_RANGE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG | HAS_EXIT_FLAG }, // FOR_ITER_TUPLE
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // INSTRUMENTED_CALL
	{ true, InstructionFormat::Instr_FMT_IX, 0 }, // INSTRUMENTED_CALL_FUNCTION_EX
	{ true, InstructionFormat::Instr_FMT_IBC00, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // INSTRUMENTED_CALL_KW
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG }, // INSTRUMENTED_END_FOR
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG }, // INSTRUMENTED_END_SEND
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // INSTRUMENTED_FOR_ITER
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // INSTRUMENTED_INSTRUCTION
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // INSTRUMENTED_JUMP_BACKWARD
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG }, // INSTRUMENTED_JUMP_FORWARD
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ESCAPES_FLAG }, // INSTRUMENTED_LINE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG }, // INSTRUMENTED_LOAD_SUPER_ATTR
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG }, // INSTRUMENTED_POP_JUMP_IF_FALSE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG }, // INSTRUMENTED_POP_JUMP_IF_NONE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG }, // INSTRUMENTED_POP_JUMP_IF_NOT_NONE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG }, // INSTRUMENTED_POP_JUMP_IF_TRUE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // INSTRUMENTED_RESUME
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_CONST_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // INSTRUMENTED_RETURN_CONST
	{ true, InstructionFormat::Instr_FMT_IX, HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // INSTRUMENTED_RETURN_VALUE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // INSTRUMENTED_YIELD_VALUE
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_EXIT_FLAG }, // LOAD_ATTR_CLASS
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_DEOPT_FLAG | HAS_ESCAPES_FLAG }, // LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG }, // LOAD_ATTR_INSTANCE_VALUE
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG }, // LOAD_ATTR_METHOD_LAZY_DICT
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_EXIT_FLAG }, // LOAD_ATTR_METHOD_NO_DICT
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG }, // LOAD_ATTR_METHOD_WITH_VALUES
	{ true, Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_DEOPT_FLAG }, // LOAD_ATTR_MODULE
	{ true, Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_EXIT_FLAG }, // LOAD_ATTR_NONDESCRIPTOR_NO_DICT
	{ true, Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG }, // LOAD_ATTR_NONDESCRIPTOR_WITH_VALUES
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG | HAS_ESCAPES_FLAG }, // LOAD_ATTR_PROPERTY
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG }, // LOAD_ATTR_SLOT
	{ true, InstructionFormat::Instr_FMT_IBC00000000, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG }, // LOAD_ATTR_WITH_HINT
	{ true, InstructionFormat::Instr_FMT_IBC000, HAS_ARG_FLAG | HAS_DEOPT_FLAG }, // LOAD_GLOBAL_BUILTIN
	{ true, InstructionFormat::Instr_FMT_IBC000, HAS_ARG_FLAG | HAS_DEOPT_FLAG }, // LOAD_GLOBAL_MODULE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_SUPER_ATTR_ATTR
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // LOAD_SUPER_ATTR_METHOD
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_EVAL_BREAK_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // RESUME
	{ true, InstructionFormat::Instr_FMT_IX, HAS_DEOPT_FLAG }, // RESUME_CHECK
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_DEOPT_FLAG }, // SEND_GEN
	{ true, InstructionFormat::Instr_FMT_IXC000, HAS_EXIT_FLAG }, // STORE_ATTR_INSTANCE_VALUE
	{ true, InstructionFormat::Instr_FMT_IXC000, HAS_EXIT_FLAG }, // STORE_ATTR_SLOT
	{ true, InstructionFormat::Instr_FMT_IBC000, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_DEOPT_FLAG | HAS_EXIT_FLAG | HAS_ESCAPES_FLAG }, // STORE_ATTR_WITH_HINT
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // STORE_NAME
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_DEOPT_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // STORE_SUBSCR_DICT
	{ true, InstructionFormat::Instr_FMT_IXC, HAS_DEOPT_FLAG }, // STORE_SUBSCR_LIST_INT
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_PURE_FLAG }, // SWAP
	{ true, InstructionFormat::Instr_FMT_IXC00, HAS_EXIT_FLAG }, // TO_BOOL_ALWAYS_TRUE
	{ true, InstructionFormat::Instr_FMT_IXC00, HAS_EXIT_FLAG }, // TO_BOOL_BOOL
	{ true, InstructionFormat::Instr_FMT_IXC00, HAS_EXIT_FLAG }, // TO_BOOL_INT
	{ true, InstructionFormat::Instr_FMT_IXC00, HAS_EXIT_FLAG }, // TO_BOOL_LIST
	{ true, InstructionFormat::Instr_FMT_IXC00, HAS_EXIT_FLAG }, // TO_BOOL_NONE
	{ true, InstructionFormat::Instr_FMT_IXC00, HAS_EXIT_FLAG }, // TO_BOOL_STR
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // UNPACK_EX
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // UNPACK_SEQUENCE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_DEOPT_FLAG }, // UNPACK_SEQUENCE_LIST
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_DEOPT_FLAG }, // UNPACK_SEQUENCE_TUPLE
	{ true, InstructionFormat::Instr_FMT_IBC, HAS_ARG_FLAG | HAS_DEOPT_FLAG }, // UNPACK_SEQUENCE_TWO_TUPLE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ESCAPES_FLAG }, // YIELD_VALUE
	{ true, InstructionFormat::Instr_FMT_IB, HAS_ARG_FLAG | HAS_ERROR_FLAG | HAS_ERROR_NO_POP_FLAG | HAS_ESCAPES_FLAG }, // _DO_CALL_FUNCTION_EX
	{ true, -1, HAS_ARG_FLAG | HAS_JUMP_FLAG | HAS_EVAL_BREAK_FLAG | HAS_ERROR_FLAG | HAS_ESCAPES_FLAG }, // JUMP
	{ true, -1, HAS_ARG_FLAG | HAS_JUMP_FLAG }, // JUMP_NO_INTERRUPT
	{ true, -1, HAS_ARG_FLAG | HAS_LOCAL_FLAG | HAS_PURE_FLAG }, // LOAD_CLOSURE
	{ true, -1, HAS_PURE_FLAG }, // POP_BLOCK
	{ true, -1, HAS_PURE_FLAG | HAS_ARG_FLAG }, // SETUP_CLEANUP
	{ true, -1, HAS_PURE_FLAG | HAS_ARG_FLAG }, // SETUP_FINALLY
	{ true, -1, HAS_PURE_FLAG | HAS_ARG_FLAG }, // SETUP_WITH
	{ true, -1, HAS_ARG_FLAG | HAS_LOCAL_FLAG }, // STORE_FAST_MAYBE_NULL
};
#endif



extern const uint8_t _alifOpcodeCaches_[256]; // 1647
#ifdef NEED_OPCODE_METADATA
const uint8_t _alifOpcodeCaches_[256] = {
	0,0,
	1, // BINARY_SUBSCR 2
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	1, // STORE_SUBSCR // 36
	3, // TO_BOOL // 37
	0,0,0,0,
	1, // BINARY_OP // 42
	0,0,0,0,0,0,
	3, // CALL // 49
	0,0,0,
	3, // CALL_KW // 53
	1, // COMPARE_OP // 54
	1, // CONTAINS_OP // 55
	0,0,0,0,0,0,0,0,0,0,0,
	1, // FOR_ITER // 67
	0,0,0,0,
	1, // JUMP_BACKWARD // 72
	0,0,0,0,
	9, // LOAD_ATTR // 77
	0,0,0,0,0,0,0,0,0,
	4, // LOAD_GLOBAL // 87
	0,0,
	1, // LOAD_SUPER_ATTR // 90
	0,0,0,
	1, // POP_JUMP_IF_FALSE // 94
	1, // POP_JUMP_IF_NONE // 95
	1, // POP_JUMP_IF_NOT_NONE // 96
	1, // POP_JUMP_IF_TRUE // 97
	0,0,0,
	1, // SEND // 101
	0,0,0,
	4, // STORE_ATTR // 105
	0,0,0,
	0,0,0,0,0,
	1, // UNPACK_SEQUENCE // 114
};
#endif


extern const uint8_t _alifOpcodeDeopt_[256]; // 1673
//#ifdef NEED_OPCODE_METADATA
const uint8_t _alifOpcodeDeopt_[256] = {
	CACHE, // CACHE // 0
	BINARY_SLICE, // BINARY_SLICE // 1
	BINARY_SUBSCR, // BINARY_SUBSCR // 2
	BINARY_OP, // BINARY_OP_INPLACE_ADD_UNICODE // 3
	CHECK_EG_MATCH, // CHECK_EG_MATCH // 4
	CHECK_EXC_MATCH, // CHECK_EXC_MATCH // 5
	CLEANUP_THROW, // CLEANUP_THROW // 6
	DELETE_SUBSCR, // DELETE_SUBSCR // 7
	END_ASYNC_FOR, // END_ASYNC_FOR // 8
	END_FOR, // END_FOR // 9
	END_SEND, // END_SEND // 10
	EXIT_INIT_CHECK, // EXIT_INIT_CHECK // 11
	FORMAT_SIMPLE, // FORMAT_SIMPLE // 12
	FORMAT_WITH_SPEC, // FORMAT_WITH_SPEC // 13
	GET_AITER, // GET_AITER // 14
	GET_ANEXT, // GET_ANEXT // 15
	GET_ITER, // GET_ITER // 16
	RESERVED, // RESERVED // 17
	GET_LEN, // GET_LEN // 18
	GET_YIELD_FROM_ITER, // GET_YIELD_FROM_ITER // 19
	INTERPRETER_EXIT, // INTERPRETER_EXIT // 20
	LOAD_BUILD_CLASS, // LOAD_BUILD_CLASS // 21
	LOAD_LOCALS, // LOAD_LOCALS // 22
	MAKE_FUNCTION, // MAKE_FUNCTION // 23
	MATCH_KEYS, // MATCH_KEYS // 24
	MATCH_MAPPING, // MATCH_MAPPING // 25
	MATCH_SEQUENCE, // MATCH_SEQUENCE // 26
	NOP, // NOP // 27
	POP_EXCEPT, // POP_EXCEPT // 28
	POP_TOP, // POP_TOP // 29
	PUSH_EXC_INFO, // PUSH_EXC_INFO // 30
	PUSH_NULL, // PUSH_NULL // 31
	RETURN_GENERATOR, // RETURN_GENERATOR // 32
	RETURN_VALUE, // RETURN_VALUE // 33
	SETUP_ANNOTATIONS, // SETUP_ANNOTATIONS // 34
	STORE_SLICE, // STORE_SLICE // 35
	STORE_SUBSCR, // STORE_SUBSCR // 36
	TO_BOOL, // TO_BOOL // 37
	UNARY_INVERT, // UNARY_INVERT // 38
	UNARY_NEGATIVE, // UNARY_NEGATIVE // 39
	UNARY_NOT, // UNARY_NOT // 40
	WITH_EXCEPT_START, // WITH_EXCEPT_START // 41
	BINARY_OP, // BINARY_OP // 42
	BUILD_LIST, // BUILD_LIST // 43
	BUILD_MAP, // BUILD_MAP // 44
	BUILD_SET, // BUILD_SET // 45
	BUILD_SLICE, // BUILD_SLICE // 46
	BUILD_STRING, // BUILD_STRING // 47
	BUILD_TUPLE, // BUILD_TUPLE // 48
	CALL, // CALL // 49
	CALL_FUNCTION_EX, // CALL_FUNCTION_EX // 50
	CALL_INTRINSIC_1, // CALL_INTRINSIC_1 // 51
	CALL_INTRINSIC_2, // CALL_INTRINSIC_2 // 52
	CALL_KW, // CALL_KW // 53
	COMPARE_OP, // COMPARE_OP // 54
	CONTAINS_OP, // CONTAINS_OP // 55
	CONVERT_VALUE, // CONVERT_VALUE // 56
	COPY, // COPY // 57
	COPY_FREE_VARS, // COPY_FREE_VARS // 58
	DELETE_ATTR, // DELETE_ATTR // 59
	DELETE_DEREF, // DELETE_DEREF // 60
	DELETE_FAST, // DELETE_FAST // 61
	DELETE_GLOBAL, // DELETE_GLOBAL // 62
	DELETE_NAME, // DELETE_NAME // 63
	DICT_MERGE, // DICT_MERGE // 64
	DICT_UPDATE, // DICT_UPDATE // 65
	ENTER_EXECUTOR, // ENTER_EXECUTOR
	EXTENDED_ARG, // EXTENDED_ARG // 66
	FOR_ITER, // FOR_ITER // 67
	GET_AWAITABLE, // GET_AWAITABLE // 68
	IMPORT_FROM, // IMPORT_FROM // 69
	IMPORT_NAME, // IMPORT_NAME // 70
	IS_OP, // IS_OP // 71
	JUMP_BACKWARD, // JUMP_BACKWARD // 72
	JUMP_BACKWARD_NO_INTERRUPT, // JUMP_BACKWARD_NO_INTERRUPT // 73
	JUMP_FORWARD, // JUMP_FORWARD // 74
	LIST_APPEND, // LIST_APPEND // 75
	LIST_EXTEND, // LIST_EXTEND // 76
	LOAD_ATTR, // LOAD_ATTR // 77
	LOAD_COMMON_CONSTANT, // LOAD_COMMON_CONSTANT // 78
	LOAD_CONST, // LOAD_CONST // 79
	LOAD_DEREF, // LOAD_DEREF // 80
	LOAD_FAST, // LOAD_FAST // 81
	LOAD_FAST_AND_CLEAR, // LOAD_FAST_AND_CLEAR // 82
	LOAD_FAST_CHECK, // LOAD_FAST_CHECK // 83
	LOAD_FAST_LOAD_FAST, // LOAD_FAST_LOAD_FAST // 84
	LOAD_FROM_DICT_OR_DEREF, // LOAD_FROM_DICT_OR_DEREF // 85
	LOAD_FROM_DICT_OR_GLOBALS, // LOAD_FROM_DICT_OR_GLOBALS // 86
	LOAD_GLOBAL, // LOAD_GLOBAL // 87
	LOAD_NAME, // LOAD_NAME // 88
	LOAD_SPECIAL, // LOAD_SPECIAL // 89
	LOAD_SUPER_ATTR, // LOAD_SUPER_ATTR // 90
	MAKE_CELL, // MAKE_CELL // 91
	MAP_ADD, // MAP_ADD // 92
	MATCH_CLASS, // MATCH_CLASS // 93
	POP_JUMP_IF_FALSE, // POP_JUMP_IF_FALSE // 94
	POP_JUMP_IF_NONE, // POP_JUMP_IF_NONE // 95
	POP_JUMP_IF_NOT_NONE, // POP_JUMP_IF_NOT_NONE // 96
	POP_JUMP_IF_TRUE, // POP_JUMP_IF_TRUE // 97
	RAISE_VARARGS, // RAISE_VARARGS // 98
	RERAISE, // RERAISE // 99
	RETURN_CONST, // RETURN_CONST // 100
	BINARY_OP, // BINARY_OP_ADD_FLOAT
	BINARY_OP, // BINARY_OP_ADD_INT
	BINARY_OP, // BINARY_OP_ADD_UNICODE
	BINARY_OP, // BINARY_OP_MULTIPLY_FLOAT
	BINARY_OP, // BINARY_OP_MULTIPLY_INT
	BINARY_OP, // BINARY_OP_SUBTRACT_FLOAT
	BINARY_OP, // BINARY_OP_SUBTRACT_INT
	BINARY_SUBSCR, // BINARY_SUBSCR_DICT
	BINARY_SUBSCR, // BINARY_SUBSCR_GETITEM
	BINARY_SUBSCR, // BINARY_SUBSCR_LIST_INT
	BINARY_SUBSCR, // BINARY_SUBSCR_STR_INT
	BINARY_SUBSCR, // BINARY_SUBSCR_TUPLE_INT
	CALL, // CALL_ALLOC_AND_ENTER_INIT
	CALL, // CALL_BOUND_METHOD_EXACT_ARGS
	CALL, // CALL_BOUND_METHOD_GENERAL
	CALL, // CALL_BUILTIN_CLASS
	CALL, // CALL_BUILTIN_FAST
	CALL, // CALL_BUILTIN_FAST_WITH_KEYWORDS
	CALL, // CALL_BUILTIN_O
	CALL, // CALL_ISINSTANCE
	CALL_KW, // CALL_KW_BOUND_METHOD
	CALL_KW, // CALL_KW_NON_PY
	CALL_KW, // CALL_KW_PY
	CALL, // CALL_LEN
	CALL, // CALL_LIST_APPEND
	CALL, // CALL_METHOD_DESCRIPTOR_FAST
	CALL, // CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS
	CALL, // CALL_METHOD_DESCRIPTOR_NOARGS
	CALL, // CALL_METHOD_DESCRIPTOR_O
	CALL, // CALL_NON_PY_GENERAL
	CALL, // CALL_PY_EXACT_ARGS
	CALL, // CALL_PY_GENERAL
	CALL, // CALL_STR_1
	CALL, // CALL_TUPLE_1
	CALL, // CALL_TYPE_1
	COMPARE_OP, // COMPARE_OP_FLOAT
	COMPARE_OP, // COMPARE_OP_INT
	COMPARE_OP, // COMPARE_OP_STR
	CONTAINS_OP, // CONTAINS_OP_DICT
	CONTAINS_OP, // CONTAINS_OP_SET
	FOR_ITER, // FOR_ITER_GEN
	FOR_ITER, // FOR_ITER_LIST
	FOR_ITER, // FOR_ITER_RANGE
	FOR_ITER, // FOR_ITER_TUPLE
	INSTRUMENTED_CALL, // INSTRUMENTED_CALL
	INSTRUMENTED_CALL_FUNCTION_EX, // INSTRUMENTED_CALL_FUNCTION_EX
	INSTRUMENTED_CALL_KW, // INSTRUMENTED_CALL_KW
	INSTRUMENTED_END_FOR, // INSTRUMENTED_END_FOR
	INSTRUMENTED_END_SEND, // INSTRUMENTED_END_SEND
	INSTRUMENTED_FOR_ITER, // INSTRUMENTED_FOR_ITER
	INSTRUMENTED_INSTRUCTION, // INSTRUMENTED_INSTRUCTION
	INSTRUMENTED_JUMP_BACKWARD, // INSTRUMENTED_JUMP_BACKWARD
	INSTRUMENTED_JUMP_FORWARD, // INSTRUMENTED_JUMP_FORWARD
	INSTRUMENTED_LINE, // INSTRUMENTED_LINE
	INSTRUMENTED_LOAD_SUPER_ATTR, // INSTRUMENTED_LOAD_SUPER_ATTR
	INSTRUMENTED_POP_JUMP_IF_FALSE, // INSTRUMENTED_POP_JUMP_IF_FALSE
	INSTRUMENTED_POP_JUMP_IF_NONE, // INSTRUMENTED_POP_JUMP_IF_NONE
	INSTRUMENTED_POP_JUMP_IF_NOT_NONE, // INSTRUMENTED_POP_JUMP_IF_NOT_NONE
	INSTRUMENTED_POP_JUMP_IF_TRUE, // INSTRUMENTED_POP_JUMP_IF_TRUE
	INSTRUMENTED_RESUME, // INSTRUMENTED_RESUME
	INSTRUMENTED_RETURN_CONST, // INSTRUMENTED_RETURN_CONST
	INSTRUMENTED_RETURN_VALUE, // INSTRUMENTED_RETURN_VALUE
	INSTRUMENTED_YIELD_VALUE, // INSTRUMENTED_YIELD_VALUE
	LOAD_ATTR, // LOAD_ATTR_CLASS
	LOAD_ATTR, // LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN
	LOAD_ATTR, // LOAD_ATTR_INSTANCE_VALUE
	LOAD_ATTR, // LOAD_ATTR_METHOD_LAZY_DICT
	LOAD_ATTR, // LOAD_ATTR_METHOD_NO_DICT
	LOAD_ATTR, // LOAD_ATTR_METHOD_WITH_VALUES
	LOAD_ATTR, // LOAD_ATTR_MODULE
	LOAD_ATTR, // LOAD_ATTR_NONDESCRIPTOR_NO_DICT
	LOAD_ATTR, // LOAD_ATTR_NONDESCRIPTOR_WITH_VALUES
	LOAD_ATTR, // LOAD_ATTR_PROPERTY
	LOAD_ATTR, // LOAD_ATTR_SLOT
	LOAD_ATTR, // LOAD_ATTR_WITH_HINT
	LOAD_GLOBAL, // LOAD_GLOBAL_BUILTIN
	LOAD_GLOBAL, // LOAD_GLOBAL_MODULE
	LOAD_SUPER_ATTR, // LOAD_SUPER_ATTR_ATTR
	LOAD_SUPER_ATTR, // LOAD_SUPER_ATTR_METHOD
	RESUME, // RESUME
	RESUME, // RESUME_CHECK
	SEND, // SEND
	SEND, // SEND_GEN
	SET_ADD, // SET_ADD
	SET_FUNCTION_ATTRIBUTE, // SET_FUNCTION_ATTRIBUTE
	SET_UPDATE, // SET_UPDATE
	STORE_ATTR, // STORE_ATTR
	STORE_ATTR, // STORE_ATTR_INSTANCE_VALUE
	STORE_ATTR, // STORE_ATTR_SLOT
	STORE_ATTR, // STORE_ATTR_WITH_HINT
	STORE_DEREF, // STORE_DEREF
	STORE_FAST, // STORE_FAST
	STORE_FAST_LOAD_FAST, // STORE_FAST_LOAD_FAST
	STORE_FAST_STORE_FAST, // STORE_FAST_STORE_FAST
	STORE_GLOBAL, // STORE_GLOBAL
	STORE_NAME, // STORE_NAME
	STORE_SUBSCR, // STORE_SUBSCR_DICT
	STORE_SUBSCR, // STORE_SUBSCR_LIST_INT
	SWAP, // SWAP
	TO_BOOL, // TO_BOOL_ALWAYS_TRUE
	TO_BOOL, // TO_BOOL_BOOL
	TO_BOOL, // TO_BOOL_INT
	TO_BOOL, // TO_BOOL_LIST
	TO_BOOL, // TO_BOOL_NONE
	TO_BOOL, // TO_BOOL_STR
	UNPACK_EX, // UNPACK_EX
	UNPACK_SEQUENCE, // UNPACK_SEQUENCE
	UNPACK_SEQUENCE, // UNPACK_SEQUENCE_LIST
	UNPACK_SEQUENCE, // UNPACK_SEQUENCE_TUPLE
	UNPACK_SEQUENCE, // UNPACK_SEQUENCE_TWO_TUPLE
	YIELD_VALUE, // YIELD_VALUE
	_DO_CALL_FUNCTION_EX, // _DO_CALL_FUNCTION_EX
};

#endif // NEED_OPCODE_METADATA
