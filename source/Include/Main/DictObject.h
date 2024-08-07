#pragma once

extern AlifInitObject _alifDictType_;


#define ALIFDICT_CHECK(op) ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(op), ALIFTPFLAGS_DICT_SUBCLASS)
#define ALIFDICT_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifDictType_)

AlifObject* alifNew_dict();
AlifObject* alifDict_getItem(AlifObject* , AlifObject* );
int alifDict_delItem(AlifObject*, AlifObject*);
int alifDict_getItemRef(AlifObject* , AlifObject* , AlifObject** );
int alifDict_setItem(AlifObject*, AlifObject*, AlifObject* );
int alifDict_next(AlifObject* , int64_t* , AlifObject** , AlifObject** );
AlifObject* alifDict_keys(AlifObject* );
int alifDict_contains(AlifObject* , AlifObject* );

int alifDict_update(AlifObject* , AlifObject* );


int alifDict_setItemString(AlifObject*, const wchar_t* , AlifObject* );

extern AlifTypeObject _alifDictIterKeyType_;
extern AlifTypeObject _alifDictIterItemType_;


typedef class DictKeysObject AlifDictKeysObject;
typedef class DictValues AlifDictValues;

class AlifDictObject {
public:
	ALIFOBJECT_HEAD

	int64_t used;

	uint64_t versionTag;

	AlifDictKeysObject* keys;

	AlifDictValues* values;
} ;

AlifObject* alifDictGetItem_knownHash(AlifObject* , AlifObject* , size_t );

#define ALIFDICT_GET_SIZE(_op) (((AlifDictObject*)_op)->used)

int alifDict_containsString(AlifObject*, const wchar_t*);
int alifDict_pop(AlifObject* , AlifObject* , AlifObject** );

#define ALIF_FOREACH_DICT_EVENT(V) \
    V(ADDED)                     \
    V(MODIFIED)                  \
    V(DELETED)                   \
    V(CLONED)                    \
    V(CLEARED)                   \
    V(DEALLOCATED)

 enum AlifDictWatchEvent {
#define ALIF_DEF_EVENT(EVENT) AlifDict_Even_##EVENT,
	ALIF_FOREACH_DICT_EVENT(ALIF_DEF_EVENT)
#undef ALIF_DEF_EVENT
} ;


