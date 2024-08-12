#include "alif.h"
#include "AlifCore_AlifHash.h"

extern AlifHashFuncDef _alifHashFunc_;


#  define ROTATE(x, b)  _rotl64(x, b)

#define HALF_ROUND(a,b,c,d,s,t)     \
    a += b; c += d;                 \
    b = ROTATE(b, s) ^ a;           \
    d = ROTATE(d, t) ^ c;           \
    a = ROTATE(a, 32);

#define SINGLE_ROUND(v0,v1,v2,v3)   \
    HALF_ROUND(v0,v1,v2,v3,13,16);  \
    HALF_ROUND(v2,v1,v0,v3,17,21);

#define DOUBLE_ROUND(v0,v1,v2,v3)   \
    SINGLE_ROUND(v0,v1,v2,v3);      \
    SINGLE_ROUND(v0,v1,v2,v3);


static uint64_t
siphash13(uint64_t k0, uint64_t k1, const void* src, int64_t src_sz) {
	uint64_t b = (uint64_t)src_sz << 56;
	const uint8_t* in = (const uint8_t*)src;

	uint64_t v0 = k0 ^ 0x736f6d6570736575ULL;
	uint64_t v1 = k1 ^ 0x646f72616e646f6dULL;
	uint64_t v2 = k0 ^ 0x6c7967656e657261ULL;
	uint64_t v3 = k1 ^ 0x7465646279746573ULL;

	uint64_t t;
	uint8_t* pt;

	while (src_sz >= 8) {
		uint64_t mi;
		memcpy(&mi, in, sizeof(mi));
		mi = (uint64_t)(mi);
		in += sizeof(mi);
		src_sz -= sizeof(mi);
		v3 ^= mi;
		SINGLE_ROUND(v0, v1, v2, v3);
		v0 ^= mi;
	}

	t = 0;
	pt = (uint8_t*)&t;
	switch (src_sz) {
	case 7: pt[6] = in[6]; ALIFFALLTHROUGH;
	case 6: pt[5] = in[5]; ALIFFALLTHROUGH;
	case 5: pt[4] = in[4]; ALIFFALLTHROUGH;
	case 4: memcpy(pt, in, sizeof(uint32_t)); break;
	case 3: pt[2] = in[2]; ALIFFALLTHROUGH;
	case 2: pt[1] = in[1]; ALIFFALLTHROUGH;
	case 1: pt[0] = in[0]; break;
	}
	b |= (uint64_t)(t);

	v3 ^= b;
	SINGLE_ROUND(v0, v1, v2, v3);
	v0 ^= b;
	v2 ^= 0xff;
	SINGLE_ROUND(v0, v1, v2, v3);
	SINGLE_ROUND(v0, v1, v2, v3);
	SINGLE_ROUND(v0, v1, v2, v3);

	/* modified */
	t = (v0 ^ v1) ^ (v2 ^ v3);
	return t;
}


size_t alif_hashBytes(const void* src, int64_t len)
{
	size_t x;

	if (len == 0) {
		return 0;
	}

	//#ifdef ALIF_HASH_STATS
		//hashstats[(len <= ALIFHASH_STATS_MAX) ? len : 0]++;
	//#endif

#if ALIF_HASH_CUTOFF > 0
	if (len < ALIF_HASH_CUTOFF) {
		size_t hash;
		const unsigned char* p = src;
		hash = 5381;

		switch (len) {
		case 7: hash = ((hash << 5) + hash) + *p++; ALIFFALLTHROUGH;
		case 6: hash = ((hash << 5) + hash) + *p++; ALIFFALLTHROUGH;
		case 5: hash = ((hash << 5) + hash) + *p++; ALIFFALLTHROUGH;
		case 4: hash = ((hash << 5) + hash) + *p++; ALIFFALLTHROUGH;
		case 3: hash = ((hash << 5) + hash) + *p++; ALIFFALLTHROUGH;
		case 2: hash = ((hash << 5) + hash) + *p++; ALIFFALLTHROUGH;
		case 1: hash = ((hash << 5) + hash) + *p++; break;
		default:
			ALIF_UNREACHABLE();
		}
		hash ^= len;
		hash ^= (Py_uhash_t)_Py_HashSecret.djbx33a.suffix;
		x = (Py_hash_t)hash;
	}
	else
#endif /
		x = _alifHashFunc_.hash(src, len);

	if (x == -1)
		return -2;
	return x;
}



static size_t alifSiphash(const void* src, int64_t src_sz) {
	//return (size_t)siphash13(
		//(uint64_t)(_alifHashSecret_.siphash.k0), (uint64_t)(_alifHashSecret_.siphash.k1),
		//src, src_sz);
	return 1;
}

static AlifHashFuncDef _alifHashFunc_ = { alifSiphash, "siphash13", 64, 128 };
