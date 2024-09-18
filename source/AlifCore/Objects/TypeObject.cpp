#include "alif.h"

#include "AlifCore_Dict.h"
#include "AlifCore_Lock.h"
#include "AlifCore_Object.h"
#include "AlifCore_ObjectAlloc.h"
#include "AlifCore_State.h"
#include "AlifCore_TypeObject.h"
#include "AlifCore_CriticalSection.h" // ربما يمكن حذفه بعد الإنتهاء من تطوير اللغة






#ifdef ALIF_GIL_DISABLED // 57

// 64
#define TYPE_LOCK &alifInterpreter_get()->types.mutex
#define BEGIN_TYPE_LOCK() ALIF_BEGIN_CRITICAL_SECTION_MUT(TYPE_LOCK)
#define END_TYPE_LOCK() ALIF_END_CRITICAL_SECTION()

#define BEGIN_TYPE_DICT_LOCK(d) \
    ALIF_BEGIN_CRITICAL_SECTION2_MUT(TYPE_LOCK, &ALIFOBJECT_CAST(d)->mutex)

#define END_TYPE_DICT_LOCK() ALIF_END_CRITICAL_SECTION2()

#else

#define BEGIN_TYPE_LOCK()
#define END_TYPE_LOCK()
#define BEGIN_TYPE_DICT_LOCK(d)
#define END_TYPE_DICT_LOCK()

#endif // 84



static inline AlifUSizeT managedStatic_typeIndexGet(AlifTypeObject* _self) { // 130
	return (AlifUSizeT)_self->subclasses - 1;
}

static ManagedStaticTypeState* managedStatic_typeStateGet(AlifInterpreter* _interp,
	AlifTypeObject* _self) { // 176
	AlifUSizeT index = managedStatic_typeIndexGet(_self);
	ManagedStaticTypeState* state =
		&(_interp->types.builtins.initialized[index]);
	if (state->type == _self) {
		return state;
	}
	if (index > ALIFMAX_MANAGED_STATIC_EXT_TYPES) {
		return state;
	}
	return &(_interp->types.forExtensions.initialized[index]);
}

ManagedStaticTypeState* alifStaticType_getState(AlifInterpreter* _interp, AlifTypeObject* _self) { // 193
	return managedStatic_typeStateGet(_interp, _self);
}

static inline void start_readying(AlifTypeObject* _type) { // 356
	if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = alifInterpreter_get();
		ManagedStaticTypeState* state = managedStatic_typeStateGet(interp, _type);
		state->readying = 1;
		return;
	}
	_type->flags |= ALIF_TPFLAGS_READYING;
}

static inline void stop_readying(AlifTypeObject* _type) { // 371 
	if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = alifInterpreter_get();
		ManagedStaticTypeState* state = managedStatic_typeStateGet(interp, _type);
		state->readying = 0;
		return;
	}
	_type->flags &= ~ALIF_TPFLAGS_READYING;
}

static inline AlifObject* lookup_tpDict(AlifTypeObject* _self) { // 401
	if (_self->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = alifInterpreter_get();
		ManagedStaticTypeState* state = alifStaticType_getState(interp, _self);
		return state->dict;
	}
	return _self->dict;
}

AlifObject* alifType_getDict(AlifTypeObject* _self) { // 413
	return lookup_tpDict(_self);
}

static inline void set_tpDict(AlifTypeObject* _self, AlifObject* _dict) { // 427
	if (_self->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = alifInterpreter_get();
		ManagedStaticTypeState* state = alifStaticType_getState(interp, _self);
		state->dict = _dict;
		return;
	}
	_self->dict = _dict;
}


AlifObject* alifType_allocNoTrack(AlifTypeObject* _type, AlifSizeT _nitems) { // 2217
	AlifObject* obj_{};
	size_t size = alifObject_varSize(_type, _nitems + 1);

	const size_t preSize = alifType_preHeaderSize(_type);
	if (_type->flags & ALIF_TPFLAGS_INLINE_VALUES) {
		size += alifInline_valuesSize(_type);
	}
	char* alloc = (char*)alifObject_mallocWithType(_type, size + preSize);
	if (alloc == nullptr) {
		//return alifErr_noMemory();
		return nullptr;
	}
	obj_ = (AlifObject*)(alloc + preSize);
	if (preSize) {
		((AlifObject**)alloc)[0] = nullptr;
		((AlifObject**)alloc)[1] = nullptr;
	}
	if (ALIFTYPE_IS_GC(_type)) {
		alifObject_gcLink(obj_);
	}
	memset(obj_, '\0', size);

	if (_type->itemSize == 0) {
		alifObject_init(obj_, _type);
	}
	else {
		alifObject_initVar((AlifVarObject*)obj_, _type, _nitems);
	}
	if (_type->flags & ALIF_TPFLAGS_INLINE_VALUES) {
		alifObject_initInlineValues(obj_, _type);
	}
	return obj_;
}




static AlifIntT typeIsSubType_baseChain(AlifTypeObject* _a, AlifTypeObject* _b) { // 2636
	do {
		if (_a == _b)
			return 1;
		_a = _a->base;
	} while (_a != nullptr);

	return (_b == &_alifBaseObjectType_);
}


static AlifIntT isSubType_withMethResOrder(AlifObject* _methResOrder,
	AlifTypeObject* _a, AlifTypeObject* _b) { // 2648
	AlifIntT res{};
	if (_methResOrder != nullptr) {
		AlifSizeT i, n;
		n = ALIFTUPLE_GET_SIZE(_methResOrder);
		res = 0;
		for (i = 0; i < n; i++) {
			if (ALIFTUPLE_GET_ITEM(_methResOrder, i) == (AlifObject*)_b) {
				res = 1;
				break;
			}
		}
	}
	else {
		/* a is not completely initialized yet; follow base */
		res = typeIsSubType_baseChain(_a, _b);
	}
	return res;
}


AlifIntT alifType_isSubType(AlifTypeObject* a, AlifTypeObject* b) { // 2673
	return isSubType_withMethResOrder(a->methResOrder, a, b);
}







static void type_dealloc(AlifObject* self) { // 5911
	AlifTypeObject* type = (AlifTypeObject*)self;

	ALIFOBJECT_GC_UNTRACK(type);
	//type_deallocCommon(type);

	//alifObject_clearWeakRefs((AlifObject*)type);

	ALIF_XDECREF(type->base);
	//ALIF_XDECREF(type->dict);
	//ALIF_XDECREF(type->bases);
	//ALIF_XDECREF(type->mro);
	//ALIF_XDECREF(type->cache);
	//clear_tpSubClasses(type);	

	AlifHeapTypeObject* et = (AlifHeapTypeObject*)type;
	ALIF_XDECREF(et->name);
	ALIF_XDECREF(et->qualname);
	ALIF_XDECREF(et->slots);
	if (et->cachedKeys) {
		alifDictKeys_decRef(et->cachedKeys);
	}
	ALIF_XDECREF(et->module_);
	alifMem_objFree(et->name);

	alifType_releaseID(et);

	ALIF_TYPE(type)->free((AlifObject*)type);
}








AlifTypeObject _alifTypeType_ = { // 6195
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نوع",
	.basicSize = sizeof(AlifHeapTypeObject),
	.itemSize = sizeof(AlifMemberDef),
	.dealloc = type_dealloc,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_TYPE_SUBCLASS |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_ITEMS_AT_END,

	.base = 0,
};

















AlifTypeObject _alifBaseObjectType_ = { // 7453
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "كائن",              
	.basicSize = sizeof(AlifObject),
	.itemSize = 0,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE,
};


static AlifIntT typeReady_preChecks(AlifTypeObject* _type) { // 7902
	if (_type->name == nullptr) {
		//alifErr_format(_alifExcSystemError_,
			//"Type does not define the tp_name field.");
		return -1;
	}
	return 0;
}

static AlifIntT typeReady_setBase(AlifTypeObject* _type) { // 7933

	AlifTypeObject* base = _type->base;
	if (base == nullptr and _type != &_alifBaseObjectType_) {
		base = &_alifBaseObjectType_;
		if (_type->flags & ALIF_TPFLAGS_HEAPTYPE) {
			_type->base = (AlifTypeObject*)ALIF_NEWREF((AlifObject*)base);
		}
		else {
			_type->base = base;
		}
	}

	if (base != nullptr and !alifType_isReady(base)) {
		if (alifType_ready(base) < 0) {
			return -1;
		}
	}

	return 0;
}

static AlifIntT typeReady_setDict(AlifTypeObject* _type) { // 8009
	if (lookup_tpDict(_type) != nullptr) {
		return 0;
	}

	AlifObject* dict = alifDict_new();
	if (dict == nullptr) {
		return -1;
	}
	set_tpDict(_type, dict);
	return 0;
}


static AlifIntT type_ready(AlifTypeObject* _type, AlifTypeObject* _def, AlifIntT _initial) { // 8383

	start_readying(_type);

	if (typeReady_preChecks(_type) < 0) {
		goto error;
	}

	if (typeReady_setDict(_type) < 0) {
		goto error;
	}
	if (typeReady_setBase(_type) < 0) {
		goto error;
	}
	if (typeReady_setType(_type) < 0) {
		goto error;
	}
	//if (typeReady_setBases(_type, _initial) < 0) {
	//	goto error;
	//}
	//if (typeReady_mro(_type, _initial) < 0) {
	//	goto error;
	//}
	//if (typeReady_setNew(_type, _initial) < 0) {
	//	goto error;
	//}
	//if (typeReady_fillDict(_type, _def) < 0) {
	//	goto error;
	//}
	//if (_initial) {
	//	if (typeReady_inherit(_type) < 0) {
	//		goto error;
	//	}
	//	if (typeReady_preheader(_type) < 0) {
	//		goto error;
	//	}
	//}
	//if (typeReady_setHash(_type) < 0) {
	//	goto error;
	//}
	//if (typeReady_addSubclasses(_type) < 0) {
	//	goto error;
	//}
	//if (_initial) {
	//	if (typeReady_managedDict(_type) < 0) {
	//		goto error;
	//	}
	//	if (typeReady_postChecks(_type) < 0) {
	//		goto error;
	//	}
	//}

	_type->flags |= ALIF_TPFLAGS_READY;
	stop_readying(_type);

	return 0;

error:
	stop_readying(_type);
	return -1;
}

AlifIntT alifType_ready(AlifTypeObject* _type) { // 8462
	if (_type->flags & ALIF_TPFLAGS_READY) {
		return 0;
	}

	if (!(_type->flags & ALIF_TPFLAGS_HEAPTYPE)) {
		_type->flags |= ALIF_TPFLAGS_IMMUTABLETYPE;
		alif_setImmortalUntracked((AlifObject*)_type);
	}

	AlifIntT res{};
	BEGIN_TYPE_LOCK();
	if (!(_type->flags & ALIF_TPFLAGS_READY)) {
		res = type_ready(_type, nullptr, 1);
	}
	else {
		res = 0;
	}
	END_TYPE_LOCK();
	return res;
}
