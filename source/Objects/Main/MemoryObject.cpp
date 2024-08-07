#include "alif.h"
#include "AlifCore_Memory.h"
#include "AlifCore_Object.h"        

static inline AlifManagedBufferObject* mbuf_alloc(void)
{
	AlifManagedBufferObject* mbuf;

	mbuf = (AlifManagedBufferObject*)
		ALIFOBJECT_GC_NEW(AlifManagedBufferObject, &_alifManagedBufferType_);
	if (mbuf == NULL)
		return NULL;
	mbuf->flags = 0;
	mbuf->exports = 0;
	mbuf->master.obj = NULL;
	ALIFOBJECT_GC_TRACK(mbuf);

	return mbuf;
}

AlifTypeObject _alifManagedBufferType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
	L"managedbuffer",
	sizeof(AlifManagedBufferObject),
	0,
	0, //mbuf_dealloc,                            /* dealloc */
	0,                                       /* vectorcall_offset */
	0,                                       /* getattr */
	0,                                       /* setattr */
	0,                                       /* repr */
	0,                                       /* as_number */
	0,                                       /* as_sequence */
	0,                                       /* as_mapping */
	0,                                       /* hash */
	0,                                       /* call */
	0,                                       /* str */
	alifObject_genericGetAttr,                 /* getattro */
	0,                                       /* setattro */
	0,                                       /* as_buffer */
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC, /* flags */
	0,                                       /* doc */
	//mbuf_traverse,                           /* traverse */
	//mbuf_clear                               /* clear */
};

// 242
#define MV_CONTIGUOUS_NDIM1(view) ((view)->shape[0] == 1 || (view)->strides[0] == (view)->itemSize)

static inline void init_strides_from_shape(AlifBuffer* view)
{
	int64_t i;


	view->strides[view->nDim - 1] = view->itemSize;
	for (i = view->nDim - 2; i >= 0; i--)
		view->strides[i] = view->strides[i + 1] * view->shape[i + 1];
}

static inline void init_shared_values(AlifBuffer* dest, const AlifBuffer* src) // 544
{
	dest->obj = src->obj;
	dest->buf = src->buf;
	dest->len = src->len;
	dest->itemSize = src->itemSize;
	dest->readonly = src->readonly;
	dest->format = src->format ? src->format : (wchar_t*)L"B";
	dest->internal = src->internal;
}

static void init_shape_strides(AlifBuffer* dest, const AlifBuffer* src)
{
	int64_t i;

	if (src->nDim == 0) {
		dest->shape = NULL;
		dest->strides = NULL;
		return;
	}
	if (src->nDim == 1) {
		dest->shape[0] = src->shape ? src->shape[0] : src->len / src->itemSize;
		dest->strides[0] = src->strides ? src->strides[0] : src->itemSize;
		return;
	}

	for (i = 0; i < src->nDim; i++)
		dest->shape[i] = src->shape[i];
	if (src->strides) {
		for (i = 0; i < src->nDim; i++)
			dest->strides[i] = src->strides[i];
	}
	else {
		init_strides_from_shape(dest);
	}
}

static inline void init_suboffsets(AlifBuffer* dest, const AlifBuffer* src)
{
	int64_t i;

	if (src->subOffSets == NULL) {
		dest->subOffSets = NULL;
		return;
	}
	for (i = 0; i < src->nDim; i++)
		dest->subOffSets[i] = src->subOffSets[i];
}

/* Initialize memoryview buffer properties. */
static void init_flags(AlifMemoryViewObject* mv)
{
	const AlifBuffer* view = &mv->view;
	int flags = 0;

	switch (view->nDim) {
	case 0:
		flags |= (ALIFMEMORYVIEW_SCALAR | ALIFMEMORYVIEW_C |
			ALIFMEMORYVIEW_FORTRAN);
		break;
	case 1:
		if (MV_CONTIGUOUS_NDIM1(view))
			flags |= (ALIFMEMORYVIEW_C | ALIFMEMORYVIEW_FORTRAN);
		break;
	default:
		if (alifBuffer_isContiguous(view, 'C'))
			flags |= ALIFMEMORYVIEW_C;
		if (alifBuffer_isContiguous(view, 'F'))
			flags |= ALIFMEMORYVIEW_FORTRAN;
		break;
	}

	if (view->subOffSets) {
		flags |= ALIFMEMORYVIEW_PIL;
		flags &= ~(ALIFMEMORYVIEW_C | ALIFMEMORYVIEW_FORTRAN);
	}

	mv->flags = flags;
}


static inline AlifMemoryViewObject* memory_alloc(int ndim)// 645
{
	AlifMemoryViewObject* mv;

	mv = (AlifMemoryViewObject*)
		ALIFOBJECT_GC_NEWVAR(AlifMemoryViewObject, &_alifMemoryViewType_, 3 * ndim);
	if (mv == NULL)
		return NULL;

	mv->mbuf = NULL;
	mv->hash = -1;
	mv->flags = 0;
	mv->exports = 0;
	mv->view.nDim = ndim;
	mv->view.shape = mv->obArray;
	mv->view.strides = mv->obArray + ndim;
	mv->view.subOffSets = mv->obArray + 2 * ndim;
	mv->weakreFlist = NULL;

	ALIFOBJECT_GC_TRACK(mv);
	return mv;
}

static AlifObject* mbuf_add_view(AlifManagedBufferObject* mbuf, const AlifBuffer* src) // 677
{
	AlifMemoryViewObject* mv;
	AlifBuffer* dest;

	if (src == NULL)
		src = &mbuf->master;

	if (src->nDim > ALIFBUF_MAX_NDIM) {
		return NULL;
	}

	mv = memory_alloc(src->nDim);
	if (mv == NULL)
		return NULL;

	dest = &mv->view;
	init_shared_values(dest, src);
	init_shape_strides(dest, src);
	init_suboffsets(dest, src);
	init_flags(mv);

	mv->mbuf = (AlifManagedBufferObject*)ALIF_NEWREF(mbuf);
	mbuf->exports++;

	return (AlifObject*)mv;
}

AlifObject* alifMemoryView_fromBuffer(const AlifBuffer* info) // 740
{
	AlifManagedBufferObject* mbuf;
	AlifObject* mv;

	if (info->buf == NULL) {

		return NULL;
	}

	mbuf = mbuf_alloc();
	if (mbuf == NULL)
		return NULL;

	mbuf->master = *info;
	mbuf->master.obj = NULL;

	mv = mbuf_add_view(mbuf, NULL);
	ALIF_DECREF(mbuf);

	return mv;
}

AlifTypeObject _alifMemoryViewType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
	L"memoryview",                             /* tp_name */
	offsetof(AlifMemoryViewObject, obArray),   /* tp_basicsize */
	sizeof(int64_t),                       /* tp_itemsize */
	0, // memory_dealloc,                           /* tp_dealloc */
	0,                                        /* tp_vectorcall_offset */
	0,                                        /* tp_getattr */
	0,                                        /* tp_setattr */
	0, // memory_repr,                              /* tp_repr */
	0,                                        /* tp_as_number */
	0, // &memory_as_sequence,                      /* tp_as_sequence */
	0, //&memory_as_mapping,                       /* tp_as_mapping */
	0, //memory_hash,                              /* tp_hash */
	0,                                        /* tp_call */
	0,                                        /* tp_str */
	alifObject_genericGetAttr,                  /* tp_getattro */
	0,                                        /* tp_setattro */
	0, //&memory_as_buffer,                        /* tp_as_buffer */
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC |
	   ALIFTPFLAGS_SEQUENCE,                   /* tp_flags */
	0, //memoryview__doc__,                        /* tp_doc */
	0,//memory_traverse,                          /* tp_traverse */
	0,//memory_clear,                             /* tp_clear */
	0,//memory_richcompare,                       /* tp_richcompare */
	offsetof(AlifMemoryViewObject, weakreFlist),/* tp_weaklistoffset */
	0, //memory_iter,                              /* tp_iter */
	0,                                        /* tp_iternext */
	0, //memory_methods,                           /* tp_methods */
	0,                                        /* tp_members */
	0, //memory_getsetlist,                        /* tp_getset */
	0,                                        /* tp_base */
	0,                                        /* tp_dict */
	0,                                        /* tp_descr_get */
	0,                                        /* tp_descr_set */
	0,                                        /* tp_dictoffset */
	0,                                        /* tp_init */
	0,                                        /* tp_alloc */
	//memoryview,                               /* tp_new */
};
