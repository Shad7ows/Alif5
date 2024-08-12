#pragma once

class AlifSlistItemS {
public:
	AlifSlistItemS* next;
};

class AlifSlistT {
public:
	AlifSlistItemS* head;
};

#define ALIFSLIST_ITEM_NEXT(ITEM) (((AlifSlistItemS *)(ITEM))->next)
#define ALIFSLIST_HEAD(SLIST) (((AlifSlistT *)(SLIST))->head)

class AlifHashTableEntryT {
public:
	AlifSlistItemS alifSlistItem;
	size_t keyHash;
	void* key;
	void* value;
};

class AlifHashTableT;

// Define function pointer types
typedef size_t(*AlifHashTableHashFunc)(const void*);
typedef int (*AlifHashTableCompareFunc)(const void*, const void*);
typedef void (*AlifHashTableDestroyFunc)(void*);
typedef AlifHashTableEntryT* (*AlifHashTableGetEntryFunc)(AlifHashTableT*, const void*);

// Define allocator class with function pointers
class AlifHashTableAllocatorT {
public:
	// Allocate a memory block
	void* (*malloc)(size_t size);

	// Release a memory block
	void (*free)(void* ptr);
};

class AlifHashTableT {
public:
	size_t nentries;
	size_t nbuckets;
	AlifSlistT* buckets;

	AlifHashTableGetEntryFunc getEntryFunc;
	AlifHashTableHashFunc hashFunc;
	AlifHashTableCompareFunc compareFunc;
	AlifHashTableDestroyFunc keyDestroyFunc;
	AlifHashTableDestroyFunc valueDestroyFunc;
	AlifHashTableAllocatorT alloc;

	// Constructor
	AlifHashTableT(size_t nbuckets, AlifHashTableGetEntryFunc getEntryFunc,
		AlifHashTableHashFunc hashFunc, AlifHashTableCompareFunc compareFunc,
		AlifHashTableDestroyFunc keyDestroyFunc, AlifHashTableDestroyFunc valueDestroyFunc,
		AlifHashTableAllocatorT alloc)
		: nentries(0), nbuckets(nbuckets), getEntryFunc(getEntryFunc), hashFunc(hashFunc),
		compareFunc(compareFunc), keyDestroyFunc(keyDestroyFunc), valueDestroyFunc(valueDestroyFunc), alloc(alloc) {
		buckets = new AlifSlistT[nbuckets](); // Initialize buckets
	}

	// Destructor
	~AlifHashTableT() {
		// Cleanup and deallocate resources
		delete[] buckets;
	}
};

AlifHashTableT* alifHashtable_new_full(AlifHashTableHashFunc ,
	AlifHashTableCompareFunc ,
	AlifHashTableDestroyFunc ,
	AlifHashTableDestroyFunc ,
	AlifHashTableAllocatorT* );

int alif_hashtable_set(AlifHashTableT* , const void* , void* );

static inline AlifHashTableEntryT* alifHashTable_getEntry(AlifHashTableT* ht, const void* key)
{
	return ht->getEntryFunc(ht, key);
}
