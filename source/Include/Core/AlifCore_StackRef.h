#pragma once


union AlifStackRef {
public:
	uintptr_t bits;
};

static const AlifStackRef ALIFSTACKREF_NULL = { 0 };



#define ALIFSTACKREF_ISNULL(stackref) ((stackref).bits == ALIFSTACKREF_NULL.bits)

#   define ALIFSTACKREF_TRUE (((uintptr_t)&alifTrue))

#   define ALIFSTACKREF_FALSE (((uintptr_t)&alifFalse))

#   define ALIFSTACKREF_NONE (((uintptr_t)&_alifNoneStruct_))

#define ALIFSTACKREF_IS(a, b) ((a).bits == (b).bits)


#   define ALIFSTACKREF_ASALIFOBJECTBORROW(stackref) ((AlifObject *)(stackref).bits)

#   define ALIFSTACKREF_ASALIFOBJECTSTEAL(stackref) ALIFSTACKREF_ASALIFOBJECTBORROW(stackref)

#   define ALIFSTACKREF_FROMALIFOBJECTSTEAL(obj) ((uintptr_t)(obj))

#   define ALIFSTACKREF_FROMALIFOBJECTNEW(obj) ( (uintptr_t)(ALIF_NEWREF(obj)))

#define ALIFSTACKREF_TYPE(stackref) ALIF_TYPE(ALIFSTACKREF_ASALIFOBJECTBORROW(stackref))

#   define ALIFSTACKREF_CLOSE(stackref) ALIF_DECREF(ALIFSTACKREF_ASALIFOBJECTBORROW(stackref));
