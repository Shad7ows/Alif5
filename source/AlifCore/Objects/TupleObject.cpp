#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_GC.h"
#include "AlifCore_Object.h"



static inline AlifIntT maybe_freeListPush(AlifTupleObject*); // 21



static AlifTupleObject* tuple_alloc(AlifSizeT _size) { // 34
	if (_size < 0) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	AlifSizeT index = _size - 1;
	if (index < ALIFTUPLE_MAXSAVESIZE) {
		AlifTupleObject* op_ = ALIF_FREELIST_POP(AlifTupleObject, tuples[index]);
		if (op_ != nullptr) {
			return op_;
		}
	}
	if ((AlifUSizeT)_size > ((AlifUSizeT)ALIF_SIZET_MAX - (sizeof(AlifTupleObject) -
		sizeof(AlifObject*))) / sizeof(AlifObject*)) {
		return nullptr;
		//return (AlifTupleObject*)alifErr_noMemory();
	}
	return ALIFOBJECT_GC_NEWVAR(AlifTupleObject, &_alifTupleType_, _size);
}

static inline AlifObject* tuple_getEmpty(void) { // 62
	return (AlifObject*)&ALIF_SINGLETON(tupleEmpty);
}

AlifObject* alifTuple_new(AlifSizeT _size) { // 68
	AlifTupleObject* op_{};
	if (_size == 0) {
		return tuple_getEmpty();
	}
	op_ = tuple_alloc(_size);
	if (op_ == nullptr) {
		return nullptr;
	}
	for (AlifSizeT i_ = 0; i_ < _size; i_++) {
		op_->item[i_] = nullptr;
	}
	ALIFOBJECT_GC_TRACK(op_);
	return (AlifObject*)op_;
}


AlifSizeT alifTuple_size(AlifObject* _op) { // 85
	if (!ALIFTUPLE_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	else
		return ALIF_SIZE(_op);
}


AlifObject* alifTuple_getItem(AlifObject* _op, AlifSizeT _i) { // 96
	if (!ALIFTUPLE_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	if (_i < 0 or _i >= ALIF_SIZE(_op)) {
		//alifErr_setString(_alifExcIndexError_, "tuple index out of range");
		return nullptr;
	}
	return ((AlifTupleObject*)_op)->item[_i];
}



AlifObject* alifTuple_pack(AlifSizeT _n, ...) { // 153
	AlifSizeT i_{};
	AlifObject* o_{};
	AlifObject** items{};
	va_list vArgs{};

	if (_n == 0) {
		return tuple_getEmpty();
	}

	va_start(vArgs, _n);
	AlifTupleObject* result = tuple_alloc(_n);
	if (result == nullptr) {
		va_end(vArgs);
		return nullptr;
	}
	items = result->item;
	for (i_ = 0; i_ < _n; i_++) {
		o_ = va_arg(vArgs, AlifObject*);
		items[i_] = ALIF_NEWREF(o_);
	}
	va_end(vArgs);
	ALIFOBJECT_GC_TRACK(result);
	return (AlifObject*)result;
}




static void tuple_dealloc(AlifTupleObject* _op) { // 184
	if (ALIF_SIZE(_op) == 0) {
		if (_op == &ALIF_SINGLETON(tupleEmpty)) {
			return;
		}
	}

	alifObject_gcUnTrack(_op);
	ALIF_TRASHCAN_BEGIN(_op, tuple_dealloc)

		AlifSizeT i = ALIF_SIZE(_op);
	while (--i >= 0) {
		ALIF_XDECREF(_op->item[i]);
	}
	if (!maybe_freeListPush(_op)) {
		ALIF_TYPE(_op)->free((AlifObject*)_op);
	}

	ALIF_TRASHCAN_END
}



#if SIZEOF_ALIF_UHASH_T > 4
#define ALIFHASH_XXPRIME_1 ((AlifUHashT)11400714785074694791ULL)
#define ALIFHASH_XXPRIME_2 ((AlifUHashT)14029467366897019727ULL)
#define ALIFHASH_XXPRIME_5 ((AlifUHashT)2870177450012600261ULL)
#define ALIFHASH_XXROTATE(x) ((x << 31) | (x >> 33))  /* Rotate left 31 bits */
#else
#define ALIFHASH_XXPRIME_1 ((AlifUHashT)2654435761UL)
#define ALIFHASH_XXPRIME_2 ((AlifUHashT)2246822519UL)
#define ALIFHASH_XXPRIME_5 ((AlifUHashT)374761393UL)
#define ALIFHASH_XXROTATE(x) ((x << 13) | (x >> 19))  /* Rotate left 13 bits */
#endif


static AlifHashT tuple_hash(AlifTupleObject* _v) { // 318
	AlifSizeT i{}, len = ALIF_SIZE(_v);
	AlifObject** item = _v->item;

	AlifUHashT acc = ALIFHASH_XXPRIME_5;
	for (i = 0; i < len; i++) {
		AlifUHashT lane = alifObject_hash(item[i]);
		if (lane == (AlifUHashT)-1) {
			return -1;
		}
		acc += lane * ALIFHASH_XXPRIME_2;
		acc = ALIFHASH_XXROTATE(acc);
		acc *= ALIFHASH_XXPRIME_1;
	}

	/* Add input length, mangled to keep the historical value of hash(()). */
	acc += len ^ (ALIFHASH_XXPRIME_5 ^ 3527539UL);

	if (acc == (AlifUHashT)-1) {
		return 1546275796;
	}
	return acc;
}



AlifObject* alifTuple_fromArray(AlifObject* const* _src, AlifSizeT _n) { // 371
	if (_n == 0) {
		return tuple_getEmpty();
	}

	AlifTupleObject* tuple = tuple_alloc(_n);
	if (tuple == nullptr) {
		return nullptr;
	}
	AlifObject** dst = tuple->item;
	for (AlifSizeT i_ = 0; i_ < _n; i_++) {
		AlifObject* item = _src[i_];
		dst[i_] = ALIF_NEWREF(item);
	}
	ALIFOBJECT_GC_TRACK(tuple);
	return (AlifObject*)tuple;
}



AlifObject* alifTuple_fromStackRefSteal(const AlifStackRef* _src, AlifSizeT _n) { // 392
	if (_n == 0) {
		return tuple_getEmpty();
	}
	AlifTupleObject* tuple = tuple_alloc(_n);
	if (tuple == nullptr) {
		for (AlifSizeT i = 0; i < _n; i++) {
			alifStackRef_close(_src[i]);
		}
		return nullptr;
	}
	AlifObject** dst_ = tuple->item;
	for (AlifSizeT i = 0; i < _n; i++) {
		dst_[i] = alifStackRef_asAlifObjectSteal(_src[i]);
	}
	ALIFOBJECT_GC_TRACK(tuple);
	return (AlifObject*)tuple;
}


static AlifObject* tuple_iter(AlifObject* _seq); // 863

AlifTypeObject _alifTupleType_ = { // 865
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مترابطة",
	.basicSize = sizeof(AlifTupleObject) - sizeof(AlifObject*),
	.itemSize = sizeof(AlifObject*),
	.dealloc = (Destructor)tuple_dealloc,
	.hash = (HashFunc)tuple_hash,

	.getAttro = alifObject_genericGetAttr,

	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_TUPLE_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF | ALIF_TPFLAGS_SEQUENCE,

	.iter = tuple_iter,

	.free = alifObject_gcDel,
};


AlifIntT alifTuple_resize(AlifObject** _pv, AlifSizeT _newSize) { // 918
	AlifTupleObject* v_{};
	AlifTupleObject* sv_{};
	AlifSizeT i_{};
	AlifSizeT oldSize{};

	v_ = (AlifTupleObject*)*_pv;
	if (v_ == nullptr or !ALIF_IS_TYPE(v_, &_alifTupleType_) ||
		(ALIF_SIZE(v_) != 0 and ALIF_REFCNT(v_) != 1)) {
		*_pv = 0;
		ALIF_XDECREF(v_);
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	oldSize = ALIF_SIZE(v_);
	if (oldSize == _newSize) {
		return 0;
	}
	if (_newSize == 0) {
		ALIF_DECREF(v_);
		*_pv = tuple_getEmpty();
		return 0;
	}
	if (oldSize == 0) {
		ALIF_DECREF(v_);
		*_pv = alifTuple_new(_newSize);
		return *_pv == nullptr ? -1 : 0;
	}

	if (ALIFOBJECT_GC_IS_TRACKED(v_)) {
		ALIFOBJECT_GC_UNTRACK(v_);
	}
	for (i_ = _newSize; i_ < oldSize; i_++) {
		ALIF_CLEAR(v_->item[i_]);
	}
	sv_ = ALIFOBJECT_GC_RESIZE(AlifTupleObject, v_, _newSize);
	if (sv_ == nullptr) {
		*_pv = nullptr;
		alifObject_gcDel(v_);
		return -1;
	}
	alif_newReferenceNoTotal((AlifObject*)sv_);
	if (_newSize > oldSize)
		memset(&sv_->item[oldSize], 0,
			sizeof(*sv_->item) * (_newSize - oldSize));
	*_pv = (AlifObject*)sv_;
	ALIFOBJECT_GC_TRACK(sv_);
	return 0;
}





/* -------------------------------  Tuple Iterator ------------------------------- */

static void tupleIter_dealloc(AlifTupleIterObject* _it) { // 986
	ALIFOBJECT_GC_UNTRACK(_it);
	ALIF_XDECREF(_it->seq);
	alifObject_gcDel(_it);
}

static AlifObject* tupleIter_next(AlifTupleIterObject* _it) { // 1001
	AlifTupleObject* seq{};
	AlifObject* item{};

	seq = _it->seq;
	if (seq == nullptr)
		return nullptr;

	if (_it->index < ALIFTUPLE_GET_SIZE(seq)) {
		item = ALIFTUPLE_GET_ITEM(seq, _it->index);
		++_it->index;
		return ALIF_NEWREF(item);
	}

	_it->seq = nullptr;
	ALIF_DECREF(seq);
	return nullptr;
}

AlifTypeObject _alifTupleIterType_ = { // 1076
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مترابطة_تكرار",
	.basicSize = sizeof(AlifTupleIterObject),
	/* methods */
	.dealloc = (Destructor)tupleIter_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = (TraverseProc)tupleIter_traverse,
	.iter = alifObject_selfIter,
	.iterNext = (IterNextFunc)tupleIter_next,
	//.methods = tupleIter_methods,
};



static AlifObject* tuple_iter(AlifObject* _seq) { // 1109
	AlifTupleIterObject* it{};

	if (!ALIFTUPLE_CHECK(_seq)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	it = ALIFOBJECT_GC_NEW(AlifTupleIterObject, &_alifTupleIterType_);
	if (it == nullptr)
		return nullptr;
	it->index = 0;
	it->seq = (AlifTupleObject*)ALIF_NEWREF(_seq);
	ALIFOBJECT_GC_TRACK(it);
	return (AlifObject*)it;
}




static inline AlifIntT maybe_freeListPush(AlifTupleObject* _op) { // 1132
	if (!ALIF_IS_TYPE(_op, &_alifTupleType_)) {
		return 0;
	}
	AlifSizeT index = ALIF_SIZE(_op) - 1;
	if (index < ALIFTUPLE_MAXSAVESIZE) {
		return ALIF_FREELIST_PUSH(tuples[index], _op, ALIFTUPLE_MAXFREELIST);
	}
	return 0;
}
