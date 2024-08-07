#pragma once

extern AlifTypeObject _alifMemoryViewType_;
extern AlifTypeObject _alifManagedBufferType_;


#define ALIFMEMORYVIEW_CHECK(op) ALIF_IS_TYPE((op), &_alifMemoryViewType_)

AlifObject* alifMemoryView_fromBuffer(const AlifBuffer* );


class AlifManagedBufferObject {
public:
	ALIFOBJECT_HEAD
		int flags;          /* state flags */
	int64_t exports; /* number of direct memoryview exports */
	AlifBuffer master; /* snapshot buffer obtained from the original exporter */
};


#define ALIFMEMORYVIEW_RELEASED    0x001  /* access to master buffer blocked */
#define ALIFMEMORYVIEW_C           0x002  /* C-contiguous layout */
#define ALIFMEMORYVIEW_FORTRAN     0x004  /* Fortran contiguous layout */
#define ALIFMEMORYVIEW_SCALAR      0x008  /* scalar: ndim = 0 */
#define ALIFMEMORYVIEW_PIL         0x010  /* PIL-style layout */
#define ALIFMEMORYVIEW_RESTRICTED  0x020  /* Disallow new references to the memoryview's buffer */


class AlifMemoryViewObject{
public:
	ALIFOBJECT_VAR_HEAD
		AlifManagedBufferObject* mbuf; /* managed buffer */
	size_t hash;               /* hash value for read-only views */
	int flags;                    /* state flags */
	int64_t exports;           /* number of buffer re-exports */
	AlifBuffer view;               /* private copy of the exporter's view */
	AlifObject* weakreFlist;
	int64_t obArray[1];       /* shape, strides, suboffsets */
} ;
