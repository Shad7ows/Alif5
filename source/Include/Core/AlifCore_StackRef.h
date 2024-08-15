#pragma once


union AlifStackRef {
	uintptr_t bits;
};

static const AlifStackRef ALIFSTACKREF_NULL = { 0 };


#   define ALIFSTACKREF_ASALIFOBJECTBORROW(stackref) ((AlifObject *)(stackref).bits)

#   define ALIFSTACKREF_ASALIFOBJECTSTEAL(stackref) ALIFSTACKREF_ASALIFOBJECTBORROW(stackref)

#   define ALIFSTACKREF_FROMALIFOBJECTSTEAL(obj) ((uintptr_t)(obj))

#   define ALIFSTACKREF_CLOSE(stackref) ALIF_DECREF(ALIFSTACKREF_ASALIFOBJECTBORROW(stackref));
