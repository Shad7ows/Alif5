#include "alif.h"
#include "AlifCore_Hashtable.h"
#include "AlifCore_AlifHash.h"
#include "AlifCore_Memory.h"


#define HASHTABLE_MIN_SIZE 16
#define HASHTABLE_HIGH 0.50
#define HASHTABLE_LOW 0.10
#define HASHTABLE_REHASH_FACTOR 2.0 / (HASHTABLE_LOW + HASHTABLE_HIGH)

#define BUCKETS_HEAD(SLIST) \
        ((AlifHashTableEntryT *)ALIFSLIST_HEAD(&(SLIST)))
#define TABLE_HEAD(HT, BUCKET) \
        ((AlifHashTableEntryT *)ALIFSLIST_HEAD(&(HT)->buckets[BUCKET]))
#define ENTRY_NEXT(ENTRY) \
        ((AlifHashTableEntryT *)ALIFSLIST_HEAD(ENTRY))

static int hashtable_rehash(AlifHashTableT* );

static void alifSlist_prepend(AlifSlistT* list, AlifSlistItemS* item)
{
	item->next = list->head;
	list->head = item;
}

size_t alifHashtable_hash_ptr(const void* key)
{
	return (size_t)alif_hashPointerRaw(key);
}

int alifHashtable_compare_direct(const void* key1, const void* key2)
{
	return (key1 == key2);
}

static size_t round_size(size_t s)
{
	size_t i;
	if (s < HASHTABLE_MIN_SIZE)
		return HASHTABLE_MIN_SIZE;
	i = 1;
	while (i < s)
		i <<= 1;
	return i;
}


AlifHashTableEntryT* alifHashtable_get_entryGeneric(AlifHashTableT* ht, const void* key)
{
	size_t key_hash = ht->hashFunc(key);
	size_t index = key_hash & (ht->nbuckets - 1);
	AlifHashTableEntryT* entry = TABLE_HEAD(ht, index);
	while (1) {
		if (entry == NULL) {
			return NULL;
		}
		if (entry->keyHash == key_hash && ht->compareFunc(key, entry->key)) {
			break;
		}
		entry = ENTRY_NEXT(entry);
	}
	return entry;
}

static AlifHashTableEntryT* alifHashtable_get_entryPtr(AlifHashTableT* ht, const void* key)
{
	size_t key_hash = alifHashtable_hash_ptr(key);
	size_t index = key_hash & (ht->nbuckets - 1);
	AlifHashTableEntryT* entry = TABLE_HEAD(ht, index);
	while (1) {
		if (entry == NULL) {
			return NULL;
		}
		// Compare directly keys (ignore entry->key_hash)
		if (entry->key == key) {
			break;
		}
		entry = ENTRY_NEXT(entry);
	}
	return entry;
}

int alif_hashtable_set(AlifHashTableT* ht, const void* key, void* value)
{
	AlifHashTableEntryT* entry;

#ifndef NDEBUG
	entry = ht->getEntryFunc(ht, key);
#endif

	entry = (AlifHashTableEntryT*)ht->alloc.malloc(sizeof(AlifHashTableEntryT));
	if (entry == NULL) {
		return -1;
	}

	entry->keyHash = ht->hashFunc(key);
	entry->key = (void*)key;
	entry->value = value;

	ht->nentries++;
	if ((float)ht->nentries / (float)ht->nbuckets > HASHTABLE_HIGH) {
		if (hashtable_rehash(ht) < 0) {
			ht->nentries--;
			ht->alloc.free(entry);
			return -1;
		}
	}

	size_t index = entry->keyHash & (ht->nbuckets - 1);
	alifSlist_prepend(&ht->buckets[index], (AlifSlistItemS*)entry);
	return 0;
}

static int hashtable_rehash(AlifHashTableT* ht)
{
	size_t new_size = round_size((size_t)(ht->nentries * HASHTABLE_REHASH_FACTOR));
	if (new_size == ht->nbuckets) {
		return 0;
	}

	size_t buckets_size = new_size * sizeof(ht->buckets[0]);
	AlifSlistT* new_buckets = (AlifSlistT*)ht->alloc.malloc(buckets_size);
	if (new_buckets == NULL) {
		return -1;
	}
	memset(new_buckets, 0, buckets_size);

	for (size_t bucket = 0; bucket < ht->nbuckets; bucket++) {
		AlifHashTableEntryT* entry = BUCKETS_HEAD(ht->buckets[bucket]);
		while (entry != NULL) {
			AlifHashTableEntryT* next = ENTRY_NEXT(entry);
			size_t entry_index = entry->keyHash & (new_size - 1);
			alifSlist_prepend(&new_buckets[entry_index],(AlifSlistItemS*)entry);
			entry = next;
		}
	}

	ht->alloc.free(ht->buckets);
	ht->nbuckets = new_size;
	ht->buckets = new_buckets;
	return 0;
}

AlifHashTableT* alifHashtable_new_full(AlifHashTableHashFunc hash_func,
	AlifHashTableCompareFunc compare_func,
	AlifHashTableDestroyFunc key_destroy_func,
	AlifHashTableDestroyFunc value_destroy_func,
	AlifHashTableAllocatorT* allocator)
{
	AlifHashTableAllocatorT alloc;
	if (allocator == NULL) {
		alloc.malloc = alifMem_objAlloc;
		alloc.free = alifMem_objFree;
	}
	else {
		alloc = *allocator;
	}

	AlifHashTableT* ht = (AlifHashTableT*)alloc.malloc(sizeof(AlifHashTableT));
	if (ht == NULL) {
		return ht;
	}

	ht->nbuckets = HASHTABLE_MIN_SIZE;
	ht->nentries = 0;

	size_t buckets_size = ht->nbuckets * sizeof(ht->buckets[0]);
	ht->buckets = (AlifSlistT*)alloc.malloc(buckets_size);
	if (ht->buckets == NULL) {
		alloc.free(ht);
		return NULL;
	}
	memset(ht->buckets, 0, buckets_size);

	ht->getEntryFunc = alifHashtable_get_entryGeneric;
	ht->hashFunc = hash_func;
	ht->compareFunc = compare_func;
	ht->keyDestroyFunc = key_destroy_func;
	ht->valueDestroyFunc = value_destroy_func;
	ht->alloc = alloc;
	if (ht->hashFunc == alifHashtable_hash_ptr
		&& ht->compareFunc == alifHashtable_compare_direct)
	{
		ht->getEntryFunc = alifHashtable_get_entryPtr;
	}
	return ht;
}
