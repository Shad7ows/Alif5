#include "alif.h"
#include "ModSupport.h"
#include "AlifCore_Namespace.h"
#include "AlifCore_Memory.h"

class AlifNamespaceObject {
public:
	ALIFOBJECT_HEAD
	AlifObject* dict;
} ;


static AlifObject* namespace_new(AlifTypeObject* type, AlifObject* args, AlifObject* kwds)
{
	AlifObject* self;

	self = type->alloc_(type, 0);
	if (self != NULL) {
		AlifNamespaceObject* ns = (AlifNamespaceObject*)self;
		ns->dict = alifNew_dict();
		if (ns->dict == NULL) {
			ALIF_DECREF(ns);
			return NULL;
		}
	}
	return self;
}

AlifTypeObject _alifNamespaceType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
	L"types.SimpleNamespace",                    /* tp_name */
	sizeof(AlifNamespaceObject),                 /* tp_basicsize */
	0,                                          /* tp_itemsize */
	0, //(destructor)namespace_dealloc,              /* tp_dealloc */
	0,                                          /* tp_vectorcall_offset */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0, //(reprfunc)namespace_repr,                   /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	0, //PyObject_GenericGetAttr,                    /* tp_getattro */
	0, //PyObject_GenericSetAttr,                    /* tp_setattro */
	0,                                          /* tp_as_buffer */
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC |
		ALIFTPFLAGS_BASETYPE,                    /* tp_flags */
	0, //namespace_doc,                              /* tp_doc */
	0,// (traverseproc)namespace_traverse,           /* tp_traverse */
	0,//(inquiry)namespace_clear,                   /* tp_clear */
	0,//namespace_richcompare,                      /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	0,//namespace_methods,                          /* tp_methods */
	0,//namespace_members,                          /* tp_members */
	0,                                          /* tp_getset */
	0,                                          /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,//offsetof(_PyNamespaceObject, ns_dict),      /* tp_dictoffset */
	0,//(initproc)namespace_init,                   /* tp_init */
	0,//PyType_GenericAlloc,                        /* tp_alloc */
	0,//(newfunc)namespace_new,                     /* tp_new */
	0,//PyObject_GC_Del,                            /* tp_free */
};


AlifObject* alifNew_namespace(AlifObject* kwds)
{
	AlifObject* ns = namespace_new(&_alifNamespaceType_, NULL, NULL);
	if (ns == NULL)
		return NULL;

	if (kwds == NULL)
		return ns;
	if (alifDict_update(((AlifNamespaceObject*)ns)->dict, kwds) != 0) {
		ALIF_DECREF(ns);
		return NULL;
	}

	return (AlifObject*)ns;
}
