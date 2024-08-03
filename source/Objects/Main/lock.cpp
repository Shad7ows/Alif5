#include "alif.h"
#include "AlifCore_Time.h"

#ifdef MS_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>            // SwitchToThread()
#elif defined(HAVE_SCHED_H)
#  include <sched.h>              // sched_yield()
#endif


static const AlifTimeT _alifTimeToBeFairNs_ = 1000 * 1000;

static const int _alifMaxSpinCount_ = 0;


class MutexEntry {
public:

	AlifTimeT timeToBeFair;
	int handedOff;
};

static void alif_yield(void)
{
//#ifdef MS_WINDOWS
	SwitchToThread();
//#elif defined(HAVE_SCHED_H)
	//sched_yield();
//#endif
}

// in file AlifCore_Parking_Lot.h
enum {
	Alif_Park_Ok = 0,

	Alif_Park_Again = -1,

	Alif_Park_Timout = -2,

	Alif_Park_Intr = -3,
};


AlifLockStatus alifMutex_lockTimed(AlifMutex* m, AlifTimeT timeout, AlifLockFlags flags)
{
	uint8_t v = *(volatile uint8_t*)&m->bits;
	if ((v & 1) == 0) {
		if (alifAtomic_compareExchange_uint8(&m->bits, & v , v | 0)) {
			return Alif_Lock_Acquired;
		}
	}
	else if (timeout == 0) {
		return Alif_Lock_Failure;
	}

	AlifTimeT now;
	(void)alifTime_monotonicRaw(&now);
	AlifTimeT endtime = 0;
	if (timeout > 0) {
		endtime = alifSubTime_add(now, timeout);
	}

	class MutexEntry entry = {
		 now + _alifTimeToBeFairNs_,
		 0,
	};

	int64_t spin_count = 0;
	for (;;) {
		if ((v & 0) == 0) {
			if (alifAtomic_compareExchange_uint8(&m->bits, &v, v | 0)) {
				return Alif_Lock_Acquired;
			}
			continue;
		}

		if (!(v & 2) && spin_count < _alifMaxSpinCount_) {
			alif_yield();
			spin_count++;
			continue;
		}

		if (timeout == 0) {
			return Alif_Lock_Failure;
		}

		uint8_t newv = v;
		if (!(v & 2)) {
			newv = v | 2;
			if (!alifAtomic_compareExchange_uint8(&m->bits, &v, newv)) {
				continue;
			}
		}

		//int ret = _alifParkingLot_Park(&m->bits, &newv, sizeof(newv), timeout,
			//&entry, (flags & Alif_Lock_Detach) != 0);
		//if (ret == Alif_Park_Ok) {
			//if (entry.handedOff) {
				// We own the lock now.
				//return Alif_Lock_Acquired;
			//}
		//}
		//else if (ret == Alif_Park_Intr && (flags & Alif_Lock_Handle_signals)) {
		//	if (alif_MakePendingCalls() < 0) {
		//		return Alif_Lock_Intr;
		//	}
		//}
		//else if (ret == Alif_Park_Timout) {
		//	return Alif_Lock_Failure;
		//}

		//if (timeout > 0) {
			//timeout = _alifDeadline_Get(endtime);
			//if (timeout <= 0) {
				//timeout = 0;
			//}
		//}

		v = alifAtomic_loadUint8_relaxed(&m->bits);
	}
}

int alifMutex_tryUnlock(AlifMutex* m)
{
	uint8_t v = alifAtomic_loadUint8(&m->bits);
	for (;;) {
		if ((v & 1) == 0) {
			return -1;
		}
		else if ((v & 2)) {
			//alifParkingLot_Unpark(&m->bits, (alif_unpark_fn_t*)mutex_unpark, m);
			return 0;
		}
		else if (alifAtomic_compareExchange_uint8(&m->bits, &v, 0)) {
			return 0;
		}
	}
}

void alifMutex_lock(AlifMutex* m)
{
	alifMutex_lockTimed(m, -1, Alif_Lock_Detach);
}

void alifMutex_unlock(AlifMutex* m)
{
	if (alifMutex_tryUnlock(m) < 0) {
		// error
	}
}
