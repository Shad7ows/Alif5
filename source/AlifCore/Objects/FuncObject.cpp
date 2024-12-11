#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_Long.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"



static void notify_funcWatchers(AlifInterpreter* _interp, AlifFunctionWatchEvent _event,
	AlifFunctionObject* _func, AlifObject* _newValue) { // 25
	uint8_t bits = _interp->activeFuncWatchers;
	AlifIntT i_ = 0;
	while (bits) {
		if (bits & 1) {
			AlifFunctionWatchCallback cb_ = _interp->funcWatchers[i_];
		
			if (cb_(_event, _func, _newValue) < 0) {
				//alifErr_formatUnraisable(
					//"Exception ignored in %s watcher callback for function %U at %p",
					//func_eventName(event), func->funcQualname, func);
			}
		}
		i_++;
		bits >>= 1;
	}
}

static inline void handle_funcEvent(AlifFunctionWatchEvent _event, AlifFunctionObject* _func,
	AlifObject* _newValue) { // 48
	AlifInterpreter* interp = alifInterpreter_get();
	if (interp->activeFuncWatchers) {
		notify_funcWatchers(interp, _event, _func, _newValue);
	}
	switch (_event) {
	case AlifFunctionWatchEvent::AlifFunction_Event_Modify_Code:
	case AlifFunctionWatchEvent::AlifFunction_Event_Modify_Defaults:
	case AlifFunctionWatchEvent::AlifFunction_Event_Modify_KWDefaults:
		RARE_EVENT_INTERP_INC(interp, funcModification);
		break;
	default:
		break;
	}
}


AlifFunctionObject* _alifFunction_fromConstructor(AlifFrameConstructor* _constr) { // 103
	AlifObject* module{};
	if (alifDict_getItemRef(_constr->globals, &ALIF_ID(__name__), &module) < 0) {
		return nullptr;
	}

	AlifFunctionObject* op_ = ALIFOBJECT_GC_NEW(AlifFunctionObject, &_alifFunctionType_);
	if (op_ == nullptr) {
		ALIF_XDECREF(module);
		return nullptr;
	}
	op_->globals = ALIF_NEWREF(_constr->globals);
	op_->builtins = ALIF_NEWREF(_constr->builtins);
	op_->name = ALIF_NEWREF(_constr->name);
	op_->qualname = ALIF_NEWREF(_constr->qualname);
	op_->code = ALIF_NEWREF(_constr->code);
	op_->defaults = ALIF_XNEWREF(_constr->defaults);
	op_->kwDefaults = ALIF_XNEWREF(_constr->kwDefaults);
	op_->closure = ALIF_XNEWREF(_constr->closure);
	op_->doc = ALIF_NEWREF(ALIF_NONE);
	op_->dict = nullptr;
	op_->weakRefList = nullptr;
	op_->module = module;
	op_->annotations = nullptr;
	op_->annotate = nullptr;
	op_->typeParams = nullptr;
	op_->vectorCall = alifFunction_vectorCall;
	op_->version = 0;

	ALIFOBJECT_GC_TRACK(op_);
	handle_funcEvent(AlifFunction_Event_Create, op_, nullptr);
	return op_;
}







AlifTypeObject _alifFunctionType_ = { // 1105
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "دالة",
	.basicSize = sizeof(AlifFunctionObject),
	//.dealloc = (Destructor)func_dealloc,
	.vectorCallOffset = offsetof(AlifFunctionObject, vectorCall),

	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_METHOD_DESCRIPTOR,
};
