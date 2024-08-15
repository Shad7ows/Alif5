#pragma once
#include <cstdint>

class AlifBackoffCounter {
public:
	union {
		class {
		public:
			uint16_t backoff : 4;
			uint16_t value : 12;
		};
		uint16_t asCounter;
	};
};

#define UNREACHABLE_BACKOFF 0xFFFF

static inline bool isUnreachable_backoffCounter(AlifBackoffCounter counter)
{
	return counter.asCounter == UNREACHABLE_BACKOFF;
}

static inline AlifBackoffCounter make_backoffCounter(uint16_t _value, uint16_t _backoff)
{
	AlifBackoffCounter result;
	result.value = _value;
	result.backoff = _backoff;
	return result;
}

static inline AlifBackoffCounter forge_backoffCounter(uint16_t counter)
{
	AlifBackoffCounter result;
	result.asCounter = counter;
	return result;
}

static inline AlifBackoffCounter restart_backoff_counter(AlifBackoffCounter counter)
{
	if (counter.backoff < 12) {
		return make_backoffCounter((1 << (counter.backoff + 1)) - 1, counter.backoff + 1);
	}
	else {
		return make_backoffCounter((1 << 12) - 1, 12);
	}
}

static inline bool backoff_counterTriggers(AlifBackoffCounter _counter)
{
	return _counter.value == 0;
}

static inline AlifBackoffCounter advance_backoffCounter(AlifBackoffCounter counter)
{
	if (!isUnreachable_backoffCounter(counter)) {
		return make_backoffCounter((counter.value - 1) & 0xFFF, counter.backoff);
	}
	else {
		return counter;
	}
}
