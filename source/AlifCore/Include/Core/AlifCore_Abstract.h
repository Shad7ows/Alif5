#pragma once











static inline AlifIntT alifIndex_check(AlifObject* _obj) { 
	AlifNumberMethods* asNumber = ALIF_TYPE(_obj)->asNumber;
	return (asNumber != nullptr and asNumber->index != nullptr);
}


AlifObject* _alifNumber_powerNoMod(AlifObject*, AlifObject*); 
AlifObject* _alifNumber_inPlacePowerNoMod(AlifObject*, AlifObject*); 

extern AlifIntT alifObject_hasLen(AlifObject*); 



#define ALIF_ITERSEARCH_COUNT    1
#define ALIF_ITERSEARCH_INDEX    2
#define ALIF_ITERSEARCH_CONTAINS 3



























AlifObject* _alifNumber_index(AlifObject*); 
