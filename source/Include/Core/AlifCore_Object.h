#pragma once


#include <stdbool.h>
#include "AlifCore_GC.h"            
#include "AlifCore_AlifState.h"

#define ALIFSUBOBJECT_HEAD_INIT(_type)       \
    {                                        \
         ALIF_IMMORTAL_REFCENT,    \
         (_type),                 \
    } \

#define ALIFSUBVAROBJECT_HEAD_INIT(_type, _size)    \
    {                                               \
        ._base_ = ALIFSUBOBJECT_HEAD_INIT(_type),   \
        .size_ = _size                              \
    } \                                              


static inline void alifSub_RefcntAdd(AlifObject* _op, int64_t _n)
{
	if (alif_isImmortal(_op)) {
		return;
	}
#ifdef ALIF_REF_DEBUG
	alifSub_AddRefTotal(alifInterpreterState_get(), n);
#endif
#if !defined(ALIF_GIL_DISABLED)
	_op->ref_ += _n;
#else
	if (__IsOwnedByCurrentThread(op)) {
		uint32_t local = op->ob_ref_local;
		_ssize_t refcnt = (_ssize_t)local + n;
#  if PY_SSIZE_T_MAX > UINT32_MAX
		if (refcnt > (_ssize_t)UINT32_MAX) {
			// Make the object immortal if the 32-bit local reference count
			// would overflow.
			refcnt = __IMMORTAL_REFCNT_LOCAL;
		}
#  endif
		__atomic_store_uint32_relaxed(&op->ob_ref_local, (uint32_t)refcnt);
	}
	else {
		__atomic_add_ssize(&op->ob_ref_shared, (n << __REF_SHARED_SHIFT));
	}
#endif
}
#define ALIFSUB_REFCNTADD(_op, _n) alifSub_RefcntAdd(ALIFSUBOBJECT_CAST(_op), _n)

extern void alifSub_setImmortal(AlifObject*);
extern void alifSub_setImmortalUntracked(AlifObject*);
static inline void alifSub_setMortal(AlifObject* _op, int64_t _ref)
{
    if (_op) {
        _op->ref_ = _ref;
    }
}

static inline void alifSub_clearImmortal(AlifObject* _op)
{
    if (_op) {
        alifSub_setMortal(_op, 1);
        ALIF_DECREF(_op);
    }
}
#define ALIFSUB_CLEARIMMORTAL(_op) \
    do { \
        alifSub_clearImmortal(ALIFSUBOBJECT_CAST(op)); \
        op = nullptr; \
    } while (0); \

static inline void alifSub_decref_specialized(AlifObject* _op, const Destructor _destruct)
{
    if (ALIFSUB_ISIMMORTAL(_op)) {
        return;
    }

    if (--_op->ref_ != 0) {
    }
    else {
        _destruct(_op);
    }
}

static inline int
alifSubType_hasFeature(AlifTypeObject* _type, unsigned long _feature) {
    return ((_type->flags_ & _feature) != 0);
}

static inline void alifSubObject_init(AlifObject* _op, AlifTypeObject* _typeObj)
{
    ALIFSET_TYPE(_op, _typeObj);
    ALIF_INCREF(_typeObj);
    alifSub_newReference(_op);
}

static inline void alifSubObject_initVar(AlifVarObject* _op, AlifTypeObject* _typeObj, int64_t _size)
{
    alifSubObject_init((AlifObject*)_op, _typeObj);
    ALIFSET_SIZE(_op, _size);
}

static inline void alifSubObjectGC_track(
#ifndef NDEBUG
	const char* _filename, int _lineno,
#endif
	AlifObject* _op)
{

#ifdef ALIF_GIL_DISABLED
	op->ogciBts |= ALIFSUBGC_BITS_TRACKED;
#else
	AlifGCHead* gc_ = alifSub_asGC(_op);

	AlifInterpreter* interp_ = alifInterpreter_get();
	AlifGCHead* generation0_ = &interp_->gc.young.head;
	AlifGCHead* last_ = (AlifGCHead*)(generation0_->gcPrev);
	alifSubGCHead_set_next(last_, gc_);
	alifSubGCHead_set_prev(gc_, last_);
	gc_->gcNext = ((uintptr_t)generation0_) | interp_->gc.visitedSpace;
	generation0_->gcPrev = (uintptr_t)gc_;
#endif
}

static inline void alifSubObjectGC_unTrack(
#ifndef NDEBUG
	const char* _filename, int _lineno,
#endif
	AlifObject* op)
{

#ifdef ALIF_GIL_DISABLED
	op->ogciBts |= ALIFSUBGC_BITS_TRACKED;
#else
	AlifGCHead* gc_ = alifSub_asGC(op);
	AlifGCHead* prev_ = alifSubGCHead_prev(gc_);
	AlifGCHead* next_ = alifSubGCHead_next(gc_);
	alifSubGCHead_set_next(prev_, next_);
	alifSubGCHead_set_prev(next_, prev_);
	gc_->gcNext = 0;
	gc_->gcPrev &= ALIFSUBGC_PREV_MASK_FINALIZED;
#endif
}

#ifdef NDEBUG
#  define ALIFSUBOBJECT_GC_TRACK(op) \
        alifSubObjectGC_track(ALIFSUBOBJECT_CAST(op))
#  define ALIFSUBOBJECT_GC_UNTRACK(op) \
        alifSubObjectGC_unTrack(ALIFSUBOBJECT_CAST(op))
#else
#  define ALIFSUBOBJECT_GC_TRACK(op) \
        alifSubObjectGC_track(__FILE__, __LINE__, ALIFSUBOBJECT_CAST(op))
#  define ALIFSUBOBJECT_GC_UNTRACK(op) \
        alifSubObjectGC_unTrack(__FILE__, __LINE__, ALIFSUBOBJECT_CAST(op))
#endif


void alifObjectGC_link(AlifObject*); // 642
