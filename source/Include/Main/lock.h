#pragma once

// in AlifAtomic_msc.h file

static inline int alifAtomic_compareExchange_int8(int8_t* obj, int8_t* expected, int8_t value)
{
	int8_t initial = (int8_t)_InterlockedCompareExchange8(
		(volatile char*)obj,
		(char)value,
		(char)*expected);
	if (initial == *expected) {
		return 1;
	}
	*expected = initial;
	return 0;
}

static inline int alifAtomic_compareExchange_uint8(uint8_t* obj, uint8_t* expected, uint8_t value)
{
	return alifAtomic_compareExchange_int8((int8_t*)obj,
		(int8_t*)expected,
		(int8_t)value);
}

static inline uint8_t alifAtomic_loadUint8_relaxed(const uint8_t* obj)
{
	return *(volatile uint8_t*)obj;
}

static inline uint8_t alifAtomic_loadUint8(const uint8_t* obj)
{
#if defined(_M_X64) || defined(_M_IX86)
	return *(volatile uint8_t*)obj;
#elif defined(_M_ARM64)
	return (uint8_t)__ldar8((unsigned __int8 volatile*)obj);
#else
#  error "no implementation of alif_atomic_load_uint8"
#endif
}


// in file AlifCore_Lock.h
enum AlifLockFlags {
	Alif_Lock_Dont_Detch = 0,

	Alif_Lock_Detach = 1,

	Alif_Lock_Handle_signals = 2,
} ;


class AlifMutex {
public:
	uint8_t bits;  // (private)
} ;

void alifMutex_lock(AlifMutex* );

void alifMutex_unlock(AlifMutex* );

static inline void alifSubMutex_lock(AlifMutex* m)
{
	uint8_t expected = 0;
	if (!alifAtomic_compareExchange_uint8(&m->bits, &expected, 1)) {
		alifMutex_lock(m);
	}
}
#define ALIFMUTEX_LOCK alifMutex_lock

static inline void alifSubMutex_unlock(AlifMutex* m)
{
	uint8_t expected = 0;
	if (!alifAtomic_compareExchange_uint8(&m->bits, &expected, 0)) {
		alifMutex_unlock(m);
	}
}
#define ALIFMUTEX_UNLOCK alifMutex_unlock

