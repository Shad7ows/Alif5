#pragma once


static inline size_t alif_hashPointerRaw(const void* ptr)
{
	uintptr_t x = (uintptr_t)ptr;

	x = (x >> 4) | (x << (8 * sizeof(uintptr_t) - 4));

	return (size_t)x;
}

size_t alif_hashBytes(const void* , int64_t );

typedef union  {
public:
	unsigned char uc[24];
	class {
	public:
		size_t prefix;
		size_t suffix;
	} fnv;
	class {
	public:
		uint64_t k0;
		uint64_t k1;
	} siphash;
	class {
	public:
		unsigned char padding[16];
		size_t suffix;
	} djbx33a;
	class {
	public:
		unsigned char padding[16];
		size_t hashsalt;
	} expat;
}AlifHashSecretT;

class AlifHashFuncDef {
public:
	size_t(* const hash)(const void*, int64_t);
	const char* name;
	const int hash_bits;
	const int seed_bits;
};

// Export for '_elementtree' shared extension
extern AlifHashSecretT _alifHashSecret_;
