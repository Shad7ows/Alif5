#pragma once

#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_FreeListState.h"
#include "AlifCore_Mimalloc.h"
#include "AlifCore_QSBR.h"

class AlifThreadImpl { // 20
public:
	AlifThread base{};

	QSBRThreadState* qsbr{};

	LListNode memFreeQueue{};

#ifdef ALIF_GIL_DISABLED
	GCThreadState gc{};
	MimallocThreadState mimalloc{};

	AlifFreeLists freeLists{};
	BRCThreadState brc{};

	class {
	public:
		AlifSizeT* refCounts{};
		AlifSizeT size{};
		AlifIntT isFinalized{};
	}types{};
#endif // ALIF_GIL_DISABLED

};
