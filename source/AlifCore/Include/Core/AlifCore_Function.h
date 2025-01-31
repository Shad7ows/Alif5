#pragma once


extern AlifObject* alifFunction_vectorCall(AlifObject*, AlifObject* const* ,
	AlifUSizeT , AlifObject* ); // 13

// 19
#define FUNC_MAX_WATCHERS 8


// 21
#define FUNC_VERSION_CACHE_SIZE (1<<12)  /* Must be a power of 2 */


class FuncVersionCacheItem { // 23
public:
	AlifFunctionObject* func{};
	AlifObject* code{};
};

class AlifFuncState { // 28
	public:
	AlifMutex mutex{};


	uint32_t nextVersion{};
	FuncVersionCacheItem funcVersionCache[FUNC_VERSION_CACHE_SIZE]{};
};





extern AlifFunctionObject* _alifFunction_fromConstructor(AlifFrameConstructor*); // 42

void _alifFunction_setVersion(AlifFunctionObject*, uint32_t); // 45



