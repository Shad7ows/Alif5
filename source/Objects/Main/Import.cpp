#include "alif.h"

//#include "AlifCore_Import.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_Memory.h"



#pragma warning(suppress : 4996)// for disable error wcsncpy function 
#define _CRT_SECURE_NO_WARNINGS


extern InitTable alifImportInitTab[];

InitTable* alifImportInitTable = alifImportInitTab;

#define INITTABLE _alifDureRun_.imports.initTable
#define LAST_MODULE_INDEX _alifDureRun_.imports.lastModuleIndex
#define EXTENSIONS _alifDureRun_.imports.extensions

#define PKGCONTEXT (_alifDureRun_.imports.pkgcontext)


#define MODULES(interp) \
    (interp)->imports.modules_
#define MODULES_BY_INDEX(interp) \
    (interp)->imports.modulesByIndex

/***************/
/* sys.modules */
/***************/


AlifObject* alifImport_initModules(AlifInterpreter* _interp)
{
	MODULES(_interp) = alifNew_dict();
	if (MODULES(_interp) == nullptr) {
		return nullptr;
	}
	return MODULES(_interp);
}

AlifObject* alifImport_getModules(AlifInterpreter* _interp)
{
	return MODULES(_interp);
}

static AlifIntT initBuildin_modulesTable() {

	AlifUSizeT size_{};

	for (size_ = 0; alifImportInitTable[size_].name != nullptr; size_++)
		;
	size_++;

	InitTable* tableCopy = (InitTable*)alifMem_dataAlloc(size_ * sizeof(InitTable));
	if (tableCopy == nullptr) return -1;

	memcpy(tableCopy, alifImportInitTable, size_ * sizeof(InitTable));
	INITTABLE = tableCopy;
	return 0;
}

static int modules_byIndex_set(AlifInterpreter* interp,
	int64_t index, AlifObject* module)
{

	if (MODULES_BY_INDEX(interp) == NULL) {
		MODULES_BY_INDEX(interp) = alifNew_list(0);
		if (MODULES_BY_INDEX(interp) == NULL) {
			return -1;
		}
	}

	while (ALIFLIST_GET_SIZE(MODULES_BY_INDEX(interp)) <= index) {
		if (alifList_append(MODULES_BY_INDEX(interp), ALIF_NONE) < 0) {
			return -1;
		}
	}

	return alifList_setItem(MODULES_BY_INDEX(interp), index, ALIF_NEWREF(module));
}

static AlifObject* import_addModule(AlifThread* _tstate, AlifObject* _name)
{
	AlifObject* modules_ = _tstate->interpreter->imports.modules_;
	if (modules_ == nullptr) {
		return nullptr;
	}

	AlifObject* m_;
	if (alifMapping_getOptionalItem(modules_, _name, &m_) < 0) {
		return nullptr;
	}
	if (m_ != nullptr and ALIFMODULE_CHECK(m_)) {
		return m_;
	}
	ALIF_XDECREF(m_);
	m_ = alifModule_newObject(_name);
	if (m_ == nullptr)
		return nullptr;
	if (alifObject_setItem(modules_, _name, m_) != 0) {
		ALIF_DECREF(m_);
		return nullptr;
	}

	return m_;
}

AlifObject* alifImport_addModuleRef(const wchar_t* _name)
{
	AlifObject* nameObj = alifUStr_fromString(_name);
	if (nameObj == nullptr) {
		return nullptr;
	}
	AlifThread* tstate = alifThread_get();
	AlifObject* module_ = import_addModule(tstate, nameObj);
	ALIF_DECREF(nameObj);
	return module_;
}

// in file AlifImport_Importdll.h
enum AlifExtModuleOrigin {
	AlifExt_Module_Origin_Core = 1,
	AlifExt_Module_Origin_Builtin = 2,
	AlifExt_Module_Origin_Dynamic = 3,
} ;

typedef AlifObject* (*AlifModInitFunction)(void);


static inline void extensions_lock_acquire()
{
	ALIFMUTEX_LOCK(&_alifDureRun_.imports.extensions.mutex);
}

static inline void extensions_lock_release()
{
	ALIFMUTEX_UNLOCK(&_alifDureRun_.imports.extensions.mutex);
}

static void* hashTable_keyFrom_2Strings(AlifObject* str1, AlifObject* str2, const wchar_t sep)
{
	int64_t str1_len, str2_len;
	const wchar_t* str1_data = alifUStr_asUTF8AndSize(str1, &str1_len);
	const wchar_t* str2_data = alifUStr_asUTF8AndSize(str2, &str2_len);
	if (str1_data == NULL || str2_data == NULL) {
		return NULL;
	}
	size_t size = str1_len + 1 + str2_len + 1;

	wchar_t* key = (wchar_t*)alifMem_dataAlloc(size);
	if (key == NULL) {
		return NULL;
	}

	//wcsncpy(key, str1_data, str1_len);
	key[str1_len] = sep;
	//wcsncpy(key + str1_len + 1, str2_data, str2_len + 1);
	return key;
}

static void hashTable_destroyStr(void* ptr)
{
	alifMem_dataFree(ptr);
}

class SinglePhaseGlobalUpdate {
public:
	AlifModInitFunction init;
	int64_t index;
	AlifObject* dict;
	AlifExtModuleOrigin origin;
};

typedef class CachedMDict {
public:
	AlifObject* copied;
	int64_t interpid;
} ;

class ExtensionsCacheValue {
public:
	AlifModuleDef* def;
	AlifModInitFunction init;
	int64_t index;
	CachedMDict* dict;
	CachedMDict Mdict;

	AlifExtModuleOrigin origin;
};

//static class ExtensionsCacheValue* extensions_cacheSet(AlifObject* path, AlifObject* name,
//	AlifModuleDef* def, AlifModInitFunction m_init,
//	int64_t m_index, AlifObject* m_dict,
//	AlifExtModuleOrigin origin, void* md_gil)
//{
//	struct extensions_cache_value* value = NULL;
//	void* key = NULL;
//	struct extensions_cache_value* newvalue = NULL;
//	AlifModuleDefBase olddefbase = def->base;
//
//	extensions_lock_acquire();
//
//	if (EXTENSIONS.hashtable == NULL) {
//		if (_extensions_cache_init() < 0) {
//			goto finally;
//		}
//	}
//
//	/* Create a cached value to populate for the module. */
//	AlifHashTableEntryT* entry =
//		_extensions_cache_find_unlocked(path, name, &key);
//	value = entry == NULL
//		? NULL
//		: (class ExtensionsCacheValue*)entry->value;
//	/* We should never be updating an existing cache value. */
//	if (value != NULL) {
//		goto finally;
//	}
//	newvalue = alloc_extensions_cache_value();
//	if (newvalue == NULL) {
//		goto finally;
//	}
//
//	*newvalue = (class ExtensionsCacheValue){
//		def,
//		m_init,
//		m_index,
//		origin,
//		nullptr,
//	};
//	if (init_cached_m_dict(newvalue, m_dict) < 0) {
//		goto finally;
//	}
//	fixup_cached_def(newvalue);
//
//	if (entry == NULL) {
//		/* It was never added. */
//		if (_Py_hashtable_set(EXTENSIONS.hashtable, key, newvalue) < 0) {
//			PyErr_NoMemory();
//			goto finally;
//		}
//		/* The hashtable owns the key now. */
//		key = NULL;
//	}
//	else if (value == NULL) {
//		/* It was previously deleted. */
//		entry->value = newvalue;
//	}
//	else {
//		//ALIF_UNREACHABLE();
//	}
//
//	value = newvalue;
//
//	finally:
//	if (value == NULL) {
//		restore_old_cached_def(def, &olddefbase);
//		if (newvalue != NULL) {
//			del_extensions_cache_value(newvalue);
//		}
//	}
//	else {
//		cleanup_old_cached_def(&olddefbase);
//	}
//
//	extensions_lock_release();
//	if (key != NULL) {
//		hashtable_destroy_str(key);
//	}
//
//	return value;
//}


static class ExtensionsCacheValue* updateGlobal_state_forExtension(AlifThread* tstate,
	AlifObject* path, AlifObject* name,
	AlifModuleDef* def,
	 SinglePhaseGlobalUpdate* singlephase)
{
	class ExtensionsCacheValue* cached = NULL;
	AlifModInitFunction m_init = NULL;
	AlifObject* m_dict = NULL;

	/* Set up for _extensions_cache_set(). */
	if (singlephase == NULL) {
	}
	else {
		if (singlephase->init != NULL) {
			m_init = singlephase->init;
		}
		else if (singlephase->dict == NULL) {
		}
		else {
			m_dict = singlephase->dict;
		}
	}

	//if (_Py_IsMainInterpreter(tstate->interpreter) || def->size == -1) {
//#ifndef NDEBUG
		//cached = extensions_cacheGet(path, name);
//#endif
		//cached = extensions_cacheSet(
			//path, name, def, m_init, singlephase->index, m_dict,
			//singlephase->origin, nullptr);
		//if (cached == NULL) {
			//return NULL;
		//}
	//}

	return cached;
}


static AlifHashTableEntryT* extensions_cacheFind_unlocked(AlifObject* path, AlifObject* name,
	void** p_key)
{
	if (EXTENSIONS.hashtable == NULL) {
		return NULL;
	}
	void* key = hashTable_keyFrom_2Strings(path, name, ' : ');
	if (key == NULL) {
		return NULL;
	}
	AlifHashTableEntryT* entry = alifHashTable_getEntry(EXTENSIONS.hashtable, key);
	if (p_key != NULL) {
		*p_key = key;
	}
	else {
		hashTable_destroyStr(key);
	}
	return entry;
}

static class ExtensionsCacheValue* extensions_cacheGet(AlifObject* _path, AlifObject* _name)
{
	class ExtensionsCacheValue* value = NULL;
	extensions_lock_acquire();

	AlifHashTableEntryT* entry =
		extensions_cacheFind_unlocked(_path, _name, NULL);
	if (entry == NULL) {
		/* It was never added. */
		goto finally;
	}
	value = (class ExtensionsCacheValue*)entry->value;

	finally:
	extensions_lock_release();
	return value;
}

static int finish_singlePhase_extension(AlifThread* tstate, AlifObject* mod,
	class ExtensionsCacheValue* cached,
	AlifObject* name, AlifObject* modules)
{

	int64_t index = ((ExtensionsCacheValue*)cached)->index;
	if (modules_byIndex_set(tstate->interpreter, index, mod) < 0) {
		return -1;
	}

	if (modules != NULL) {
		if (alifObject_setItem(modules, name, mod) < 0) {
			return -1;
		}
	}

	return 0;
}

/*******************/
/* builtin modules */
/*******************/

int alifImport_fixupBuiltin(AlifThread* _tState, AlifObject* _mod, const wchar_t* _name, // 2180
	AlifObject* _modules)
{
	int res_ = -1;
	class ExtensionsCacheValue* cached_{};
	AlifObject* nameObj;
	nameObj = alifUStr_fromString(_name);
	if (nameObj == NULL) {
		return -1;
	}

	AlifModuleDef* def_ = alifModule_getDef(_mod);
	if (def_ == NULL) {
		goto finally;
	}

	cached_ = extensions_cacheGet(nameObj, nameObj);
	if (cached_ == NULL) {
		class SinglePhaseGlobalUpdate singlephase = {
			0,
			def_->base.index,
			NULL,
			AlifExt_Module_Origin_Core,
		};
		//cached_ = update_global_state_for_extension(
			//_tState, nameObj, nameObj, def_, &singlephase);
		if (cached_ == NULL) {
			goto finally;
		}
	}

	if (finish_singlePhase_extension(_tState, _mod, cached_, nameObj, _modules) < 0) {
		goto finally;
	}

	res_ = 0;

	finally:
	ALIF_DECREF(nameObj);
	return res_;
}

AlifIntT alifImport_init() {

	if (INITTABLE != nullptr) {
		// error
	}

	AlifIntT status = 1;

	if (initBuildin_modulesTable() != 0) {
		status = -1;
		return status;
	}

	return status;
}

