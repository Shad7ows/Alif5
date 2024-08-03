#include "alif.h"
#include "AlifCore_Time.h"          // AlifTimeT

#include <time.h>                 // gmtime_r()
#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>           // gettimeofday()
#endif
#ifdef MS_WINDOWS
#  include <winsock2.h>           // struct timeval
#endif

#if defined(__APPLE__)
#  include <mach/mach_time.h>     // mach_absolute_time(), mach_timebase_info()

#if defined(__APPLE__) && defined(__has_builtin)
#  if __has_builtin(__builtin_available)
#    define HAVE_CLOCK_GETTIME_RUNTIME __builtin_available(macOS 10.12, iOS 10.0, tvOS 10.0, watchOS 3.0, *)
#  endif
#endif
#endif

/* To millisecond (10^-3) */
#define SEC_TO_MS 1000

/* To microseconds (10^-6) */
#define MS_TO_US 1000
#define SEC_TO_US (SEC_TO_MS * MS_TO_US)

/* To nanoseconds (10^-9) */
#define US_TO_NS 1000
#define MS_TO_NS (MS_TO_US * US_TO_NS)
#define SEC_TO_NS (SEC_TO_MS * MS_TO_NS)

/* Conversion from nanoseconds */
#define NS_TO_MS (1000 * 1000)
#define NS_TO_US (1000)
#define NS_TO_100NS (100)

static AlifTimeFraction _alifQpcBase_ = { 0, 0 };

static int alifWin_perfCounter_frequency(AlifTimeFraction* , int );

static AlifTimeT alifTime_GCD(AlifTimeT x, AlifTimeT y)
{
	while (y != 0) {
		AlifTimeT tmp = y;
		y = x % y;
		x = tmp;
	}
	return x;
}


int alifTimeFraction_set(AlifTimeFraction* frac, AlifTimeT numer, AlifTimeT denom)
{
	if (numer < 1 || denom < 1) {
		return -1;
	}

	AlifTimeT gcd = alifTime_GCD(numer, denom);
	frac->numer = numer / gcd;
	frac->denom = denom / gcd;
	return 0;
}

double alifTimeFraction_resolution(const AlifTimeFraction* frac)
{
	return (double)frac->numer / (double)frac->denom / 1e9;
}

static inline int alifTime_add(AlifTimeT* t1, AlifTimeT t2)
{
	if (t2 > 0 && *t1 > ALIFTime_MAX - t2) {
		*t1 = ALIFTime_MAX;
		return -1;
	}
	else if (t2 < 0 && *t1 < ALIFTime_MIN - t2) {
		*t1 = ALIFTime_MIN;
		return -1;
	}
	else {
		*t1 += t2;
		return 0;
	}
}

AlifTimeT alifSubTime_add(AlifTimeT t1, AlifTimeT t2)
{
	(void)alifTime_add(&t1, t2);
	return t1;
}

static inline int alifTime_mulCheck_overflow(AlifTimeT _a, AlifTimeT _b)
{
    if (_b != 0) {
        return ((_a < ALIFTime_MIN / _b) || (ALIFTime_MAX / _b < _a));
    }
    else {
        return 0;
    }
}

static inline int alifTime_mul(AlifTimeT* _t, AlifTimeT _k)
{
    if (alifTime_mulCheck_overflow(*_t, _k)) {
        *_t = (*_t >= 0) ? ALIFTime_MAX : ALIFTime_MIN;
        return -1;
    }
    else {
        *_t *= _k;
        return 0;
    }
}

static inline AlifTimeT alifSubTime_mul(AlifTimeT t, AlifTimeT k)
{
	(void)alifTime_mul(&t, k);
	return t;
}

AlifTimeT alifTimeFraction_Mul(AlifTimeT ticks, const AlifTimeFraction* frac)
{
	const AlifTimeT mul = frac->numer;
	const AlifTimeT div = frac->denom;

	if (div == 1) {
		return alifSubTime_mul(ticks, mul);
	}

	AlifTimeT intpart, remaining;
	intpart = ticks / div;
	ticks %= div;
	remaining = alifSubTime_mul(ticks, mul) / div;
	return alifSubTime_add(alifSubTime_mul(intpart, mul), remaining);
}


// in file AlifMath.h
#define ALIF_IS_NAN(X) isnan(X)

static double aliftime_round_half_even(double _x)
{
    double rounded_ = round(_x);
    if (fabs(_x - rounded_) == 0.5) {
        /* halfway case: round to even */
        rounded_ = 2.0 * round(_x / 2.0);
    }
    return rounded_;
}

static double aliftime_round(double _x, AlifSubTimeRoundT _round)
{
    volatile double d_;

    d_ = _x;
    if (_round == AlifSubTime_Round_Half_Even) {
        d_ = aliftime_round_half_even(d_);
    }
    else if (_round == AlifSubTime_Round_Ceiling) {
        d_ = ceil(d_);
    }
    else if (_round == AlifSubTime_Round_Floor) {
        d_ = floor(d_);
    }
    else {
        d_ = (d_ >= 0.0) ? ceil(d_) : floor(d_);
    }
    return d_;
}

static int alifTime_from_double(AlifTimeT* _tp, double _value, AlifSubTimeRoundT _round,
    long _unitToNs)
{
    volatile double d_;

    d_ = _value;
    d_ *= (double)_unitToNs;
    d_ = aliftime_round(d_, _round);

    if (!((double)ALIFTime_MIN <= d_ && d_ < -(double)ALIFTime_MIN)) {
        *_tp = 0;
        return -1;
    }
    AlifTimeT ns_ = (AlifTimeT)d_;

    *_tp = ns_;
    return 0;
}

static int alifTime_from_object(AlifTimeT* _tp, AlifObject* _obj, AlifSubTimeRoundT _round,
    long _unitToNs)
{
    if ((_obj->type_ == &_alifFloatType)) {
        double d_;
        d_ = alifFloat_asLongDouble(_obj);
        if (ALIF_IS_NAN(d_)) {
            return -1;
        }
        return alifTime_from_double(_tp, d_, _round, _unitToNs);
    }
    else {
        long long sec = alifInteger_asLongLong(_obj);

        AlifTimeT ns = (AlifTimeT)sec;
        if (alifTime_mul(&ns, _unitToNs) < 0) {
            return -1;
        }

        *_tp = ns;
        return 0;
    }
}

int alifSubTime_fromSecondsObject(AlifTimeT* _tp, AlifObject* _obj, AlifSubTimeRoundT _round)
{
    return alifTime_from_object(_tp, _obj, _round, SEC_TO_NS);
}

static AlifTimeT alifTime_divide_round_up(const AlifTimeT _t, const AlifTimeT _k)
{
    if (_t >= 0) {
        AlifTimeT q_ = _t / _k;
        if (_t % _k) {
            q_ += 1;
        }
        return q_;
    }
    else {

        AlifTimeT q_ = _t / _k;
        if (_t % _k) {
            q_ -= 1;
        }
        return q_;
    }
}


static AlifTimeT alifTime_divide(const AlifTimeT _t, const AlifTimeT _k,
    const AlifSubTimeRoundT _round)
{
    if (_round == AlifSubTime_Round_Half_Even) {
        AlifTimeT x_ = _t / _k;
        AlifTimeT r_ = _t % _k;
        AlifTimeT absR = ALIF_ABS(r_);
        if (absR > _k / 2 || (absR == _k / 2 && (ALIF_ABS(x_) & 1))) {
            if (_t >= 0) {
                x_++;
            }
            else {
                x_--;
            }
        }
        return x_;
    }
    else if (_round == AlifSubTime_Round_Ceiling) {
        if (_t >= 0) {
            return alifTime_divide_round_up(_t, _k);
        }
        else {
            return _t / _k;
        }
    }
    else if (_round == AlifSubTime_Round_Floor) {
        if (_t >= 0) {
            return _t / _k;
        }
        else {
            return alifTime_divide_round_up(_t, _k);
        }
    }
    else {
        return alifTime_divide_round_up(_t, _k);
    }
}

AlifTimeT alifSubTime_asMicroseconds(AlifTimeT _ns, AlifSubTimeRoundT _round)
{
    return alifTime_divide(_ns, NS_TO_US, _round);
}


static int alifWin_perfCounter_frequency(AlifTimeFraction* base, int raise_exc)
{
	LARGE_INTEGER freq;
	(void)QueryPerformanceFrequency(&freq);
	LONGLONG frequency = freq.QuadPart;


	AlifTimeT denom = (AlifTimeT)frequency;

	if (alifTimeFraction_set(base, SEC_TO_NS, denom) < 0) {
		if (raise_exc) {
			// error
		}
		return -1;
	}
	return 0;
}

static int alifGet_winPerf_counter(AlifTimeT* tp, AlifClockInfoT* info, int raise_exc)
{

	if (_alifQpcBase_.denom == 0) {
		if (alifWin_perfCounter_frequency(&_alifQpcBase_, raise_exc) < 0) {
			return -1;
}
	}

	if (info) {
		info->implementation = L"QueryPerformanceCounter()";
		info->resolution = alifTimeFraction_resolution(&_alifQpcBase_);
		info->monotonic = 1;
		info->adjustable = 0;
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	LONGLONG ticksll = now.QuadPart;

	AlifTimeT ticks;
	static_assert(sizeof(ticksll) <= sizeof(ticks),
		"LONGLONG is larger than AlifTimeT");
	ticks = (AlifTimeT)ticksll;

	*tp = alifTimeFraction_Mul(ticks, &_alifQpcBase_);
	return 0;
}

static int alifGet_monotonic_clock(AlifTimeT* tp, AlifClockInfoT* info, int raise_exc)
{

//#if defined(MS_WINDOWS)
	if (alifGet_winPerf_counter(tp, info, raise_exc) < 0) {
		return -1;
	}
//#elif defined(__APPLE__)
//	static _alifTimeFraction base = { 0, 0 };
//	if (base.denom == 0) {
//		if (alif_mach_timebase_info(&base, raise_exc) < 0) {
//			return -1;
//		}
//	}
//
//	if (info) {
//		info->implementation = "mach_absolute_time()";
//		info->resolution = _alifTimeFraction_Resolution(&base);
//		info->monotonic = 1;
//		info->adjustable = 0;
//	}
//
//	uint64_t uticks = mach_absolute_time();
//	// unsigned => signed
//	assert(uticks <= (uint64_t)alifTime_MAX);
//	alifTime_t ticks = (alifTime_t)uticks;
//
//	alifTime_t ns = _alifTimeFraction_Mul(ticks, &base);
//	*tp = ns;
//
//#elif defined(__hpux)
//	hrtime_t time = gethrtime();
//	if (time == -1) {
//		if (raise_exc) {
//			alifErr_SetFromErrno(alifExc_OSError);
//		}
//		return -1;
//	}
//
//	*tp = time;
//
//	if (info) {
//		info->implementation = "gethrtime()";
//		info->resolution = 1e-9;
//		info->monotonic = 1;
//		info->adjustable = 0;
//	}
//
//#else
//
//#ifdef CLOCK_HIGHRES
//	const clockid_t clk_id = CLOCK_HIGHRES;
//	const char* implementation = "clock_gettime(CLOCK_HIGHRES)";
//#else
//	const clockid_t clk_id = CLOCK_MONOTONIC;
//	const char* implementation = "clock_gettime(CLOCK_MONOTONIC)";
//#endif
//
//	struct timespec ts;
//	if (clock_gettime(clk_id, &ts) != 0) {
//		if (raise_exc) {
//			alifErr_SetFromErrno(alifExc_OSError);
//			return -1;
//		}
//		return -1;
//	}
//
//	if (aliftime_fromtimespec(tp, &ts, raise_exc) < 0) {
//		return -1;
//	}
//
//	if (info) {
//		info->monotonic = 1;
//		info->implementation = implementation;
//		info->adjustable = 0;
//		struct timespec res;
//		if (clock_getres(clk_id, &res) != 0) {
//			alifErr_SetFromErrno(alifExc_OSError);
//			return -1;
//		}
//		info->resolution = res.tv_sec + res.tv_nsec * 1e-9;
//	}
//#endif
	return 0;
}



int alifTime_monotonicRaw(AlifTimeT* _result)
{
	if (alifGet_monotonic_clock(_result, NULL, 0) < 0) {
		*_result = 0;
		return -1;
	}
	return 0;
}
