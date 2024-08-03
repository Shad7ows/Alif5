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

	if (MODULES_BY_INDEX(interp) == nullptr) {
		MODULES_BY_INDEX(interp) = alifNew_list(0);
		if (MODULES_BY_INDEX(interp) == nullptr) {
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

static bool resolve_module_alias(const wchar_t* name, const class ModuleAlias* aliases,
	const wchar_t** alias)
{
	const class ModuleAlias* entry; 
	for (entry = aliases; ; entry++) {
		if (entry->name == NULL) {
			/* It isn't an alias. */
			return false;
		}
		if (wcscmp(name, entry->name) == 0) {
			if (alias != NULL) {
				*alias = entry->orig;
			}
			return true;
		}
	}
}


static bool use_frozen(void)
{
	AlifInterpreter* interp = alifInterpreter_get();
	int override = (interp->imports.overrideFrozenModules);
	if (override > 0) {
		return true;
	}
	else if (override < 0) {
		return false;
	}
	else {
		//return interp->config.useFrozenModules; // سيتم اضافته لاحقا
	}
}

enum FrozenStatus {
	Frozen_Okay,
	Frozen_Bad_Name,   
	Frozen_Not_Found,  
	Frozen_Disabled,    
	Frozen_Excluded,
	Frozen_Invalid,     
} ;

static const class Frozen* look_up_frozen(const wchar_t* name)
{
	const class Frozen* p;
	// We always use the bootstrap modules.
	for (p = _alifImportFrozenBootstrap_; ; p++) {
		if (p->name == nullptr) {
			// We hit the end-of-list sentinel value.
			break;
		}
		if (wcscmp(name, p->name) == 0) {
			return p;
		}
	}
	// Prefer custom modules, if any.  Frozen stdlib modules can be
	// disabled here by setting "code" to nullptr in the array entry.
	if (_alifImportFrozenModules_ != nullptr) {
		for (p = _alifImportFrozenModules_; ; p++) {
			if (p->name == nullptr) {
				break;
			}
			if (wcscmp(name, p->name) == 0) {
				return p;
			}
		}
	}
	if (use_frozen()) {
		for (p = _alifImportFrozenStdlib_; ; p++) {
			if (p->name == nullptr) {
				break;
			}
			if (wcscmp(name, p->name) == 0) {
				return p;
			}
		}
		for (p = _alifImportFrozenTest_; ; p++) {
			if (p->name == nullptr) {
				break;
			}
			if (wcscmp(name, p->name) == 0) {
				return p;
			}
		}
	}
	return nullptr;
}

class FrozenInfo {
public:
	AlifObject* nameobj;
	const wchar_t* data;
	int64_t size;
	bool isPackage;
	bool isAlias;
	const wchar_t* origname;
};

static FrozenStatus find_frozen(AlifObject* nameobj, struct FrozenInfo* info)
{
	if (info != nullptr) {
		memset(info, 0, sizeof(*info));
	}

	if (nameobj == nullptr || nameobj == ALIF_NONE) {
		return Frozen_Bad_Name;
	}
	const wchar_t* name = alifUStr_asUTF8(nameobj);
	if (name == nullptr) {
		return Frozen_Bad_Name;
	}

	const class Frozen* p = look_up_frozen(name);
	if (p == nullptr) {
		return Frozen_Not_Found;
	}
	if (info != nullptr) {
		info->nameobj = nameobj;  // borrowed
		info->data = (const wchar_t*)p->code;
		info->size = p->size;
		info->isPackage = p->isPackage;
		if (p->size < 0) {
			// backward compatibility with negative size values
			info->size = -(p->size);
			info->isPackage = true;
		}
		info->origname = name;
		info->isAlias = resolve_module_alias(name, _alifImportFrozenAliases_,
			&info->origname);
	}
	if (p->code == nullptr) {
		/* It is frozen but marked as un-importable. */
		return Frozen_Excluded;
	}
	if (p->code[0] == '\0' || p->size == 0) {
		/* Does not contain executable code. */
		return Frozen_Invalid;
	}
	return Frozen_Okay;
}

static AlifObject* unmarshal_frozen_code(AlifInterpreter* interp, class FrozenInfo* info)
{
	AlifObject* co = PyMarshal_ReadObjectFromString(info->data, info->size);
	if (co == nullptr) {

		//set_frozen_error(FROZEN_INVALID, info->nameobj);
		return nullptr;
	}
	if (!ALIF_IS_TYPE(co, &_alifCodeType_)) {
		ALIF_DECREF(co);
		return nullptr;
	}
	return co;
}

int alifImport_importFrozenModuleObject(AlifObject* name)
{
	AlifThread* tstate = alifThread_get();
	AlifObject* co, * m, * d = nullptr;
	int err;

	class FrozenInfo info;
	FrozenStatus status = find_frozen(name, &info);
	if (status == Frozen_Not_Found || status == Frozen_Disabled) {
		return 0;
	}
	else if (status == Frozen_Bad_Name) {
		return 0;
	}
	else if (status != Frozen_Okay) {
		set_frozen_error(status, name);
		return -1;
	}
	co = unmarshal_frozen_code(tstate->interpreter, &info);
	if (co == nullptr) {
		return -1;
	}
	if (info.isPackage) {
		AlifObject* l;
		m = import_add_module(tstate, name);
		if (m == nullptr)
			goto errReturn;
		d = PyModule_GetDict(m);
		l = PyList_New(0);
		if (l == nullptr) {
			ALIF_DECREF(m);
			goto errReturn;
		}
		err = PyDict_SetItemString(d, "__path__", l);
		ALIF_DECREF(l);
		ALIF_DECREF(m);
		if (err != 0)
			goto errReturn;
	}
	d = module_dict_for_exec(tstate, name);
	if (d == nullptr) {
		goto errReturn;
	}
	m = exec_code_in_module(tstate, name, d, co);
	if (m == nullptr) {
		goto errReturn;
	}
	ALIF_DECREF(m);
	/* Set __origname__ (consumed in FrozenImporter._setup_module()). */
	AlifObject* origname;
	if (info.origname) {
		origname = alifUStr_fromString(info.origname);
		if (origname == nullptr) {
			goto errReturn;
		}
	}
	else {
		origname = ALIF_NEWREF(ALIF_NONE);
	}
	err = PyDict_SetItemString(d, L"__origname__", origname);
	ALIF_DECREF(origname);
	if (err != 0) {
		goto errReturn;
	}
	ALIF_DECREF(d);
	ALIF_DECREF(co);
	return 1;

errReturn:
	ALIF_XDECREF(d);
	ALIF_DECREF(co);
	return -1;
}

int alifImport_importFrozenModule(const wchar_t* name)
{
	AlifObject* nameobj;
	int ret;
	nameobj = alifUStr_fromString(name);
	if (nameobj == nullptr)
		return -1;
	ret = alifImport_importFrozenModuleObject(nameobj);
	ALIF_DECREF(nameobj);
	return ret;
}


/*************/
/* importlib */
/*************/

/* Import the _imp extension by calling manually _imp.create_builtin() and
   _imp.exec_builtin() since importlib is not initialized yet. Initializing
   importlib requires the _imp module: this function fix the bootstrap issue.
 */
static AlifObject*
bootstrap_imp(AlifThread* tstate)
{
	AlifObject* name = alifUStr_fromString(L"_imp");
	if (name == nullptr) {
		return nullptr;
	}

	// Mock a ModuleSpec object just good enough for alifModule_FromDefAndSpec():
	// an object with just a name attribute.
	//
	// _imp.__spec__ is overridden by importlib._bootstrap._instal() anyway.
	AlifObject* attrs = Py_BuildValue("{sO}", "name", name);
	if (attrs == nullptr) {
		goto error;
	}
	AlifObject* spec = _PyNamespace_New(attrs);
	ALIF_DECREF(attrs);
	if (spec == nullptr) {
		goto error;
	}

	// Create the _imp module from its definition.
	AlifObject* mod = create_builtin(tstate, name, spec);
	ALIF_CLEAR(name);
	ALIF_DECREF(spec);
	if (mod == nullptr) {
		goto error;
	}

	// Execute the _imp module: call imp_module_exec().
	if (exec_builtin_or_dynamic(mod) < 0) {
		ALIF_DECREF(mod);
		goto error;
	}
	return mod;

error:
	ALIF_XDECREF(name);
	return nullptr;
}


static int init_importlib(AlifThread* tstate, AlifObject* sysmod)
{
	AlifInterpreter* interp = tstate->interpreter;

	if (alifImport_importFrozenModule(L"_frozen_importlib") <= 0) {
		return -1;
	}

	AlifObject* importlib = alifImport_addModuleRef(L"_frozen_importlib");
	if (importlib == nullptr) {
		return -1;
	}
	interp->imports.importLib = importlib;


	AlifObject* imp_mod = bootstrap_imp(tstate);
	if (imp_mod == nullptr) {
		return -1;
	}
	if (alifImport_setModuleString(L"_imp", imp_mod) < 0) {
		ALIF_DECREF(imp_mod);
		return -1;
	}

	AlifObject* value = alifObject_callMethod(importlib, "_install",
		"OO", sysmod, imp_mod);
	ALIF_DECREF(imp_mod);
	if (value == nullptr) {
		return -1;
	}
	ALIF_DECREF(value);

	return 0;
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
	if (str1_data == nullptr || str2_data == nullptr) {
		return nullptr;
	}
	size_t size = str1_len + 1 + str2_len + 1;

	wchar_t* key = (wchar_t*)alifMem_dataAlloc(size);
	if (key == nullptr) {
		return nullptr;
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
//	struct extensions_cache_value* value = nullptr;
//	void* key = nullptr;
//	struct extensions_cache_value* newvalue = nullptr;
//	AlifModuleDefBase olddefbase = def->base;
//
//	extensions_lock_acquire();
//
//	if (EXTENSIONS.hashtable == nullptr) {
//		if (_extensions_cache_init() < 0) {
//			goto finally;
//		}
//	}
//
//	/* Create a cached value to populate for the module. */
//	AlifHashTableEntryT* entry =
//		_extensions_cache_find_unlocked(path, name, &key);
//	value = entry == nullptr
//		? nullptr
//		: (class ExtensionsCacheValue*)entry->value;
//	/* We should never be updating an existing cache value. */
//	if (value != nullptr) {
//		goto finally;
//	}
//	newvalue = alloc_extensions_cache_value();
//	if (newvalue == nullptr) {
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
//	if (entry == nullptr) {
//		/* It was never added. */
//		if (alifHashtable_set(EXTENSIONS.hashtable, key, newvalue) < 0) {
//			goto finally;
//		}
//		/* The hashtable owns the key now. */
//		key = nullptr;
//	}
//	else if (value == nullptr) {
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
//	if (value == nullptr) {
//		restore_old_cached_def(def, &olddefbase);
//		if (newvalue != nullptr) {
//			del_extensions_cache_value(newvalue);
//		}
//	}
//	else {
//		cleanup_old_cached_def(&olddefbase);
//	}
//
//	extensions_lock_release();
//	if (key != nullptr) {
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
	class ExtensionsCacheValue* cached = nullptr;
	AlifModInitFunction m_init = nullptr;
	AlifObject* m_dict = nullptr;

	/* Set up for _extensions_cache_set(). */
	if (singlephase == nullptr) {
	}
	else {
		if (singlephase->init != nullptr) {
			m_init = singlephase->init;
		}
		else if (singlephase->dict == nullptr) {
		}
		else {
			m_dict = singlephase->dict;
		}
	}

	//if (alif_isMainInterpreter(tstate->interpreter) || def->size == -1) {
//#ifndef NDEBUG
		//cached = extensions_cacheGet(path, name);
//#endif
		//cached = extensions_cacheSet(
			//path, name, def, m_init, singlephase->index, m_dict,
			//singlephase->origin, nullptr);
		//if (cached == nullptr) {
			//return nullptr;
		//}
	//}

	return cached;
}


static AlifHashTableEntryT* extensions_cacheFind_unlocked(AlifObject* path, AlifObject* name,
	void** p_key)
{
	if (EXTENSIONS.hashtable == nullptr) {
		return nullptr;
	}
	void* key = hashTable_keyFrom_2Strings(path, name, ' : ');
	if (key == nullptr) {
		return nullptr;
	}
	AlifHashTableEntryT* entry = alifHashTable_getEntry(EXTENSIONS.hashtable, key);
	if (p_key != nullptr) {
		*p_key = key;
	}
	else {
		hashTable_destroyStr(key);
	}
	return entry;
}

static class ExtensionsCacheValue* extensions_cacheGet(AlifObject* _path, AlifObject* _name)
{
	class ExtensionsCacheValue* value = nullptr;
	extensions_lock_acquire();

	AlifHashTableEntryT* entry =
		extensions_cacheFind_unlocked(_path, _name, nullptr);
	if (entry == nullptr) {
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

	if (modules != nullptr) {
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
	if (nameObj == nullptr) {
		return -1;
	}

	AlifModuleDef* def_ = alifModule_getDef(_mod);
	if (def_ == nullptr) {
		goto finally;
	}

	cached_ = extensions_cacheGet(nameObj, nameObj);
	if (cached_ == nullptr) {
		class SinglePhaseGlobalUpdate singlephase = {
			0,
			def_->base.index,
			nullptr,
			AlifExt_Module_Origin_Core,
		};
		//cached_ = update_global_state_for_extension(
			//_tState, nameObj, nameObj, def_, &singlephase);
		if (cached_ == nullptr) {
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

/*************************/
/* interpreter cycle */
/*************************/

int alifImport_InitCore(AlifThread* tstate, AlifObject* sysmod, int importlib)
{

	if (importlib) {
		if (init_importlib(tstate, sysmod) < 0) {
			return -1; // error
		}
	}

	return 0;
}
