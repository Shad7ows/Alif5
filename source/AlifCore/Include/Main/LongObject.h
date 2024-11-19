#pragma once









 // 12
#define ALIFLONG_CHECK(_op) \
        ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_LONG_SUBCLASS)
#define ALIFLONG_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifLongType_)


AlifObject* alifLong_fromLong(long); // 16
AlifObject* alifLong_fromUnsignedLong(unsigned long); // 17

AlifObject* alifLong_fromSizeT(AlifSizeT); // 19
AlifObject* alifLong_fromDouble(double); // 20

long alifLong_asLong(AlifObject* ); // 22

AlifSizeT alifLong_asSizeT(AlifObject*); // 24


double alifLong_asDouble(AlifObject*); // 86
AlifObject* alifLong_fromVoidPtr(void*); // 87
AlifObject* alifLong_fromUnsignedLongLong(unsigned long long); // 91
AlifObject* alifLong_fromString(const char*, char**, AlifIntT); // 97

unsigned long alifOS_strToULong(const char*, char**, AlifIntT); // 102
long alifOS_strToLong(const char*, char**, AlifIntT); // 103

/* -------------------------------------------------------------------------------------------------------------------------------------- */






AlifUSizeT _alifLong_numBits(AlifObject*); // 77
