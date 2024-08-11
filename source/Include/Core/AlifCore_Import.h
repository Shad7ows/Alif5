#pragma once




// in file AlifCore_HashTable.h


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


static inline AlifHashTableEntryT* alifHashTable_getEntry(AlifHashTableT* ht, const void* key)
{
	return ht->getEntryFunc(ht, key);
}

int alifImport_fixupBuiltin(AlifThread* , AlifObject* , const wchar_t* ,
	AlifObject* );

class ImportDureRun {
public:
	class InitTable* initTable;

	int64_t lastModuleIndex;
	class {
	public:
		AlifMutex mutex;
		AlifHashTableT* hashtable;
	}extensions;
	const char* pkgcontext;
};

class ImportState {
public:
	AlifObject* modules_;

	AlifObject* modulesByIndex;
	AlifObject* importLib;

	int overrideFrozenModules;
	int overrideMultiInterpExtensionsCheck;
//#ifdef HAVE_DLOPEN
	//int dlopenflags;
//#endif
	AlifObject* importFunc;
	class {
	public:
		void* mutex_;
		long thread_;
		int level_;
	} lock_;
	class {
	public:
		int importLevel;
		AlifTimeT accumulated_;
		int header_;
	} findAndLoad;
};

#  define ALIF_DLOPEN_FLAGS 0
#  define DLOPENFLAGS_INIT

#define IMPORTS_INIT \
    { \
        nullptr, \
        nullptr, \
        nullptr, \
        0, \
        0, \
        nullptr, \
        { \
            nullptr, \
            -1, \
            0, \
        }, \
        { \
            1, \
        }, \
    }

AlifObject* alifImport_initModules(AlifInterpreter* );
AlifObject* alifImport_getModules(AlifInterpreter* );


extern AlifIntT alifImport_init();

int alifImport_initCore(AlifThread* , AlifObject* , int );

class ModuleAlias {
public:
	const wchar_t* name;                 /* ASCII encoded string */
	const wchar_t* orig;                 /* ASCII encoded string */
};

extern const class Frozen* _alifImportFrozenBootstrap_;
extern const class Frozen* _alifImportFrozenStdlib_;
extern const class Frozen* _alifImportFrozenTest_;

extern const class ModuleAlias* _alifImportFrozenAliases_;

int alifImport_checkSubinterpIncompatibleExtensionAllowed(const wchar_t* );
