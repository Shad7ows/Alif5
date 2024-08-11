#include "alif.h"

#include "AlifCore_Import.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_Memory.h"
#include "AlifCore_Call.h"
#include "AlifCore_Namespace.h"
#include "AlifCore_ModuleObject.h"
#include "Marshal.h"


// in file Importdl.c

static const char* const _asciiOnlyPrefix_ = "AlifInit";
static const char* const _nonasciiPrefix_ = "AlifInitU";

//static void alifExt_moduleLoaderResult_setError(
//	class AlifExtModuleLoaderResult* res,
//	enum AlifExtModuleLoaderResultErrorKind kind)
//{
//#ifndef NDEBUG
//	switch (kind) {
//	case AlifExt_Module_Loader_Result_Exception: ALIFFALLTHROUGH;
//	case AlifExt_Module_Loader_Result_Err_Unreported_Exc:
//		break;
//	case AlifExt_Module_Loader_Result_Err_Missing: ALIFFALLTHROUGH;
//	case AlifExt_Module_Loader_Result_Err_Uninitialized: ALIFFALLTHROUGH;
//	case AlifExt_Module_Loader_Result_Err_Nonascii_Not_Multiphase: ALIFFALLTHROUGH;
//	case AlifExt_Module_Loader_Result_Err_Not_Module: ALIFFALLTHROUGH;
//	case AlifExt_Module_Loader_Result_Err_Missing_Def:
//		break;
//	default:
//	}
//#endif
//
//	res->err = &res->_err;
//	*res->err = (class AlifExtModuleLoaderResultError){
//		.kind = kind,
//		.exc = alifErr_GetRaisedException(),
//	};
//	switch (kind) {
//	case AlifExt_Module_Loader_Result_Err_Uninitialized:
//		res->kind = AlifExt_module_kind_INVALID;
//		break;
//	case AlifExt_Module_Loader_Result_Exception: ALIFFALLTHROUGH;
//	case AlifExt_Module_Loader_Result_Err_Missing: ALIFFALLTHROUGH;
//	case AlifExt_module_loader_result_ERR_UNREPORTED_EXC: ALIFFALLTHROUGH;
//	case AlifExt_Module_Loader_Result_Err_Nonascii_Not_Multiphase: ALIFFALLTHROUGH;
//	case AlifExt_Module_Loader_Result_Err_Not_Module: ALIFFALLTHROUGH;
//	case AlifExt_Module_Loader_Result_Err_Missing_Def:
//		break;
//	default:
//	}
//}

int alifImport_runModInitFunc(AlifModInitFunction p0,
	class AlifExtModuleLoaderInfo* info,
	class AlifExtModuleLoaderResult* p_res)
{
	class AlifExtModuleLoaderResult res = {
		nullptr,
		nullptr,
		AlifExt_Module_Kind_Unknown,
	};

	const char* oldcontext = alifImport_swapPackageContext(info->newContext);
	AlifObject* m = p0();
	alifImport_swapPackageContext(oldcontext);

	if (m == NULL) {
		res.kind = AlifExt_Module_Kind_Singlephase;
		//alifExt_moduleLoaderResult_setError(
			//&res, AlifExt_Module_Loader_Result_Err_Missing);

		goto error;
	}

	if (ALIF_IS_TYPE(m, NULL)) {
		//alifExt_module_loader_result_set_error(
			//&res, alifExt_module_loader_result_ERR_UNINITIALIZED);
		m = NULL;
			goto error;
	}

	if (ALIFOBJECT_TYPECHECK(m, &_alifModuleType_)) {
		res.kind = AlifExt_Module_Kind_Multiphase;
		res.def = (AlifModuleDef*)m;
	}
	else if (info->hookPrefix == _nonasciiPrefix_) {
		res.kind = AlifExt_Module_Kind_Multiphase;
		//alifExt_module_loader_result_set_error(
			//&res, alifExt_module_loader_result_ERR_NONASCII_NOT_MULTIPHASE);
		goto error;
	}
	else {
		res.kind = AlifExt_Module_Kind_Singlephase;
		res.module_ = m;

		if (!ALIFMODULE_CHECK(m)) {
			//alifExt_module_loader_result_set_error(
				//&res, alifExt_module_loader_result_ERR_NOT_MODULE);
			goto error;
		}

		res.def = alifModule_getDef(m);
		if (res.def == NULL) {
			//AlifExt_module_loader_result_set_error(
				//&res, alifExt_module_loader_result_ERR_MISSING_DEF);
			goto error;
		}
	}
	*p_res = res;
	return 0;

error:
	ALIF_CLEAR(res.module_);
	res.def = NULL;
	*p_res = res;
	//p_res->err = &p_res->_err;
	return -1;
}




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
/* sys.modules */ // 126
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

static inline AlifObject* get_modules_dict(AlifThread* tstate, bool fatal)
{

	AlifObject* modules = MODULES(tstate->interpreter);
	if (modules == NULL) {
		return NULL;
	}
	return modules;
}

int alifImport_setModuleString(const wchar_t* name, AlifObject* m)
{
	AlifThread* tstate = alifThread_get();
	AlifObject* modules = get_modules_dict(tstate, true);
	return alifMapping_setItemString(modules, name, m);
}

static AlifObject* import_getModule(AlifThread* tstate, AlifObject* name)
{
	AlifObject* modules = get_modules_dict(tstate, false);
	if (modules == NULL) {
		return NULL;
	}

	AlifObject* m;
	ALIF_INCREF(modules);
	(void)alifMapping_getOptionalItem(modules, name, &m);
	ALIF_DECREF(modules);
	return m;
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

static void remove_module(AlifThread* tstate, AlifObject* name)
{

	AlifObject* modules = get_modules_dict(tstate, true);
	if (ALIFDICT_CHECKEXACT(modules)) {
		(void)alifDict_pop(modules, name, NULL);
	}
	else if (alifObject_delItem(modules, name) < 0) {

	}

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

const char* alifImport_swapPackageContext(const char* newcontext)
{
//#ifndef HAVE_THREAD_LOCAL
	//alifThread_acquire_lock(EXTENSIONS.mutex, WAIT_LOCK);
//#endif
	const char* oldcontext = PKGCONTEXT;
	PKGCONTEXT = newcontext;
//#ifndef HAVE_THREAD_LOCAL
	//alifThread_release_lock(EXTENSIONS.mutex);
//#endif
	return oldcontext;
}

static int execBuiltin_orDynamic(AlifObject* mod) {
	AlifModuleDef* def;
	void* state;

	if (!ALIFMODULE_CHECK(mod)) {
		return 0;
	}

	def = alifModule_getDef(mod);
	if (def == NULL) {
		return 0;
	}

	state = ((AlifModuleObject*)mod)->state;
	if (state) {
		/* Already initialized; skip reload */
		return 0;
	}

	return alifModule_execDef(mod, def);
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

static AlifObject* moduleDict_forExec(AlifThread* tstate, AlifObject* name)
{
	AlifObject* m, * d;

	m = import_addModule(tstate, name);
	if (m == NULL)
		return NULL;

	d = alifModule_getDict(m);
	AlifObject* strBuiltins = alifUStr_fromString(L"__builtins__");
	int r = alifDict_contains(d, strBuiltins);
	if (r == 0) {
		AlifThread* thread = alifThread_get();
		r = alifDict_setItem(d, strBuiltins, alifEval_getBuiltins(thread));
	}
	if (r < 0) {
		remove_module(tstate, name);
		ALIF_DECREF(m);
		return NULL;
	}

	ALIF_INCREF(d);
	ALIF_DECREF(m);
	return d;
}

static AlifObject* execCode_inModule(AlifThread* tstate, AlifObject* name,
	AlifObject* module_dict, AlifObject* code_object)
{
	AlifObject* v, * m;

	v = alifEval_evalCode(code_object, module_dict, module_dict);
	if (v == NULL) {
		remove_module(tstate, name);
		return NULL;
	}
	ALIF_DECREF(v);

	m = import_getModule(tstate, name);


	return m;
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
	AlifObject* co = alifMarshal_readObjectFromString(info->data, info->size);
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
		return -1;
	}
	co = unmarshal_frozen_code(tstate->interpreter, &info);
	if (co == nullptr) {
		return -1;
	}
	if (info.isPackage) {
		AlifObject* l;
		m = import_addModule(tstate, name);
		if (m == nullptr)
			goto errReturn;
		d = alifModule_getDict(m);
		l = alifNew_list(0);
		if (l == nullptr) {
			ALIF_DECREF(m);
			goto errReturn;
		}
		err = alifDict_setItemString(d, L"__path__", l);
		ALIF_DECREF(l);
		ALIF_DECREF(m);
		if (err != 0)
			goto errReturn;
	}
	d = moduleDict_forExec(tstate, name);
	if (d == nullptr) {
		goto errReturn;
	}
	m = execCode_inModule(tstate, name, d, co);
	if (m == nullptr) {
		goto errReturn;
	}
	ALIF_DECREF(m);
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
	err = alifDict_setItemString(d, L"__origname__", origname);
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
	AlifObject* mod{};
	AlifObject* spec{};
	// Mock a ModuleSpec object just good enough for alifModule_FromDefAndSpec():
	// an object with just a name attribute.
	//
	// _imp.__spec__ is overridden by importlib._bootstrap._instal() anyway.
	AlifObject* attrs = alif_buildValue(L"{sO}", "name", name);
	if (attrs == nullptr) {
		goto error;
	}
	spec = alifNew_namespace(attrs);
	ALIF_DECREF(attrs);
	if (spec == nullptr) {
		goto error;
	}

	//// Create the _imp module from its definition.
	mod = create_builtin(tstate, name, spec);
	ALIF_CLEAR(name);
	ALIF_DECREF(spec);
	if (mod == nullptr) {
		goto error;
	}

	//// Execute the _imp module: call imp_module_exec().
	if (execBuiltin_orDynamic(mod) < 0) {
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

	AlifObject* value = alifObject_callMethod(importlib, L"_install",
		L"OO", sysmod, imp_mod);
	ALIF_DECREF(imp_mod);
	if (value == nullptr) {
		return -1;
	}
	ALIF_DECREF(value);

	return 0;
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

typedef AlifObject* (*AlifModInitFunction)(void);

static inline void extensions_lock_acquire()
{
	ALIFMUTEX_LOCK(&_alifDureRun_.imports.extensions.mutex);
}

static inline void extensions_lock_release()
{
	ALIFMUTEX_UNLOCK(&_alifDureRun_.imports.extensions.mutex);
}

static AlifThread* switchTo_mainInterpreter(AlifThread* tstate)
{
	if ((tstate->interpreter == _alifDureRun_.interpreters.main)) {
		return tstate;
	}
	//AlifThread* main_tstate = alifThreadState_NewBound(
		//tstate->interpreter == _alifDureRun_.interpreters.main, alifThreadState_WHENCE_EXEC);
	//if (main_tstate == NULL) {
		//return NULL;
	//}
#ifndef NDEBUG
	//AlifThread* old_tstate = alifThreadState_Swap(main_tstate);
#else
	//(void)ThreadState_Swap(main_tstate);
#endif
	//return main_tstate;
}

class SinglephaseGlobalUpdate {
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

static AlifObject* getCore_moduleDict( AlifInterpreter* , AlifObject* , AlifObject* );

static AlifObject*
getCached_mDict(class ExtensionsCacheValue* value,
	AlifObject* name, AlifObject* path) // 1120
{
	AlifInterpreter* interp = alifInterpreter_get();
	if (value->origin == AlifExt_Module_Origin_Core) {
		return getCore_moduleDict(interp, name, path);
	}
	AlifObject* dict = value->def->base.copy;
	ALIF_XINCREF(dict);
	return dict;
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

static AlifHashTableEntryT* extensionsCache_findUnlocked(AlifObject* path, AlifObject* name,
	void** p_key)
{
	if (EXTENSIONS.hashtable == NULL) {
		return NULL;
	}
	void* key = hashTable_keyFrom_2Strings(path, name, ':');
	if (key == NULL) {
		return NULL;
	}
	AlifHashTableEntryT* entry = EXTENSIONS.hashtable->getEntryFunc(EXTENSIONS.hashtable, key);
	if (p_key != NULL) {
		*p_key = key;
	}
	else {
		hashTable_destroyStr(key);
	}
	return entry;
}

static class ExtensionsCacheValue* extensions_cache_get(AlifObject* path, AlifObject* name)
{
	class ExtensionsCacheValue* value = NULL;
	extensions_lock_acquire();

	AlifHashTableEntryT* entry =
		extensionsCache_findUnlocked(path, name, NULL);
	if (entry == NULL) {
		goto finally;
	}
	value = (class ExtensionsCacheValue*)entry->value;

	finally:
	extensions_lock_release();
	return value;
}

static bool check_multi_interp_extensions(AlifInterpreter* interp) // 1444
{
	int override = (interp->imports.overrideMultiInterpExtensionsCheck);
	if (override < 0) {
		return false;
	}
	else if (override > 0) {
		return true;
	}
	else if (alifInterpreterState_hasFeature(
		interp, 1UL << 8)) {
		return true;
	}
	return false;
}

int alifImport_checkSubinterpIncompatibleExtensionAllowed(const wchar_t* name)
{
	AlifInterpreter* interp = alifInterpreter_get();
	if (check_multi_interp_extensions(interp)) {
		return -1;
	}
	return 0;
}

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

static AlifObject* getCore_moduleDict(AlifInterpreter* interp, AlifObject* name, AlifObject* path)
{
	if (path == name) {
		//if (alifUnicode_CompareWithASCIIString(name, "sys") == 0) {
			//return ALIF_NEWREF(interp->sysdictCopy);
		//}
		//if (alifUnicode_CompareWithASCIIString(name, "builtins") == 0) {
			//return ALIF_NEWREF(interp->builtinsCopy);
		//}
	}
	return NULL;
}

static class ExtensionsCacheValue* updateGlobal_state_forExtension(AlifThread* tstate,
	AlifObject* path, AlifObject* name,
	AlifModuleDef* def,
	 SinglephaseGlobalUpdate* singlephase)
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

static AlifObject* reload_singlephase_extension(AlifThread* tstate, // 1760
	class ExtensionsCacheValue* cached,
	class AlifExtModuleLoaderInfo* info)
{
	AlifModuleDef* def = cached->def;
	AlifObject* mod = NULL;

	const wchar_t* name_buf = alifUStr_asUTF8(info->name);
	if (alifImport_checkSubinterpIncompatibleExtensionAllowed(name_buf) < 0) {
		return NULL;
	}

	AlifObject* modules = get_modules_dict(tstate, true);
	if (def->size == -1) {

		AlifObject* copy = getCached_mDict(cached, info->name, info->path);
		if (copy == NULL) {
			return NULL;
		}
		mod = import_addModule(tstate, info->name);
		if (mod == NULL) {
			ALIF_DECREF(copy);
			return NULL;
		}
		AlifObject* mdict = alifModule_getDict(mod);
		if (mdict == NULL) {
			ALIF_DECREF(copy);
			ALIF_DECREF(mod);
			return NULL;
		}
		int rc = alifDict_update(mdict, copy);
		ALIF_DECREF(copy);
		if (rc < 0) {
			ALIF_DECREF(mod);
			return NULL;
		}
	}
//#ifdef ALIF_GIL_DISABLED
		//if (def->base.copy != NULL) {
			//((AlifModuleObject*)mod)->gil = cached->gil;
		//}
//#endif
	//}
	//else {
		AlifModInitFunction p0 = def->base.init;
		if (p0 == NULL) {
			return NULL;
		}
		class AlifExtModuleLoaderResult res {0};
		//if (alifImport_RunModInitFunc(p0, info, &res) < 0) {
			//alif_ext_module_loader_result_apply_error(&res, name_buf);
			//return NULL;
		//}
		mod = res.module_;

		//res = 0;

		if (info->filename != NULL) {
			if (alifModule_addObjectRef(mod, L"__file__", info->filename) < 0) {
				// return error
			}
		}

		if (alifObject_setItem(modules, info->name, mod) == -1) {
			ALIF_DECREF(mod);
			return NULL;
		}
	//}

	int64_t index = (cached->index);
	if (modules_byIndex_set(tstate->interpreter, index, mod) < 0) {
		alifObject_delItem(modules, info->name);
		ALIF_DECREF(mod);
		return NULL;
	}

	return mod;
}

static AlifObject* import_find_extension(AlifThread* tstate,
	class AlifExtModuleLoaderInfo* info,
	class ExtensionsCacheValue** p_cached)
{
	class ExtensionsCacheValue* cached
		= extensions_cache_get(info->path, info->name);
	if (cached == NULL) {
		return NULL;
	}

	*p_cached = cached;


	const wchar_t* name_buf = alifUStr_asUTF8(info->name);
	if (alifImport_checkSubinterpIncompatibleExtensionAllowed(name_buf) < 0) {
		return NULL;
	}

	AlifObject* mod = reload_singlephase_extension(tstate, cached, info);
	if (mod == NULL) {
		return NULL;
	}

	//int verbose = alifInterpreterState_getConfig(tstate->interp)->verbose;

	return mod;
}


static AlifObject* import_run_extension(AlifThread* tstate, AlifModInitFunction p0, // 1908
	class AlifExtModuleLoaderInfo* info,
	AlifObject* spec, AlifObject* modules)
{

	AlifObject* mod = NULL;
	AlifModuleDef* def = NULL;
	class ExtensionsCacheValue* cached = NULL;
	const wchar_t* name_buf = ALIFWBYTES_AS_STRING(info->nameEncoded);
	bool switched = false;
	AlifThread* main_tstate = switchTo_mainInterpreter(tstate);
	if (main_tstate == NULL) {
		return NULL;
	}
	else if (main_tstate != tstate) {
		switched = true;
	}

	class AlifExtModuleLoaderResult res;
	int rc = alifImport_runModInitFunc(p0, info, &res);
	if (rc < 0) {

	}
	else {

		mod = res.module_;
		res.module_ = NULL;
		def = res.def;

		if (res.kind == AlifExt_Module_Kind_Singlephase) {
			if (info->filename != NULL) {
				AlifObject* filename = NULL;
				if (switched) {

					//filename = AlifUStr_copy(info->filename);
					if (filename == NULL) {
						return NULL;
					}
				}
				else {
					filename = ALIF_NEWREF(info->filename);
				}

				AlifInterpreter* interp = alifInterpreter_get();
				//alifUStr_internImmortal(interp, &filename);

				if (alifModule_addObjectRef(mod, L"__file__", filename) < 0) {
					return nullptr;
				}
			}

			class SinglephaseGlobalUpdate singlephase = {
				nullptr,
				def->base.index,
				nullptr,
				info->origin,

			};

			if (def->size == -1) {

				singlephase.dict = alifModule_getDict(mod);
			}
			else {
				singlephase.init = p0;
			}
			cached = updateGlobal_state_forExtension(
				tstate, info->path, info->name, def, &singlephase);
			if (cached == NULL) {
				goto main_finally;
			}
		}
	}

main_finally:
	if (switched) {
		//switch_back_from_main_interpreter(tstate, main_tstate, mod);
		mod = NULL;
	}

	if (rc < 0) {
		//alifExt_module_loader_result_apply_error(&res, name_buf);
		goto error;
	}

	if (res.kind == AlifExt_Module_Kind_Multiphase) {
		mod = ALIFMODULE_FROMDEFANDSPEC(def, spec);
		if (mod == NULL) {
			goto error;
		}
	}
	else {
		if (alifImport_checkSubinterpIncompatibleExtensionAllowed(name_buf) < 0) {
			goto error;
		}

		if (switched) {
			mod = reload_singlephase_extension(tstate, cached, info);
			if (mod == NULL) {
				goto error;
			}
		}
		else {
			AlifObject* modules = get_modules_dict(tstate, true);
			if (finish_singlePhase_extension(
				tstate, mod, cached, info->name, modules) < 0)
			{
				goto error;
			}
		}
	}

	//res = (class AlifExtModuleLoaderResult)(0);
	return mod;

error:
	ALIF_XDECREF(mod);
	//res = (class AlifExtModuleLoaderResult)(0);
	return NULL;
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
		class SinglephaseGlobalUpdate singlephase = {
			0,
			def_->base.index,
			nullptr,
			AlifExt_Module_Origin_Core,
		};
		cached_ = updateGlobal_state_forExtension(
			_tState, nameObj, nameObj, def_, &singlephase);
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

static AlifObject* create_builtin(AlifThread* tstate, AlifObject* name, AlifObject* spec)
{
	class AlifExtModuleLoaderInfo info;
	//if (alifExt_module_loader_info_init_for_builtin(&info, name) < 0) {
		//return NULL;
	//}
	class InitTable* found{};
	AlifModInitFunction p0{};
	class ExtensionsCacheValue* cached = NULL;
	AlifObject* mod = import_find_extension(tstate, &info, &cached);
	if (mod != NULL) {
		goto finally;
	}

	//if (cached != NULL) {
		//_extensions_cache_delete(info.path, info.name);
	//}

	found = NULL;
	//for (class InitTable* p = INITTABLE; p->name != NULL; p++) {
		//if (alifUnicode_EqualToASCIIString(info.name, p->name)) {
			//found = p;
		//}
	//}
	if (found == NULL) {
		// not found
		mod = ALIF_NEWREF(ALIF_NONE);
		goto finally;
	}

	 p0 = (AlifModInitFunction)found->initFunc;
	if (p0 == NULL) {
		mod = import_addModule(tstate, info.name);
		goto finally;
	}

//#ifdef ALIF_GIL_DISABLED
	//alifEval_EnableGILTransient(tstate);
//#endif
	/* Now load it. */
	mod = import_run_extension(
		tstate, p0, &info, spec, get_modules_dict(tstate, true));
//#ifdef ALIF_GIL_DISABLED
	//if (alifImport_CheckGILForModule(mod, info.name) < 0) {
		//ALIF_CLEAR(mod);
		//goto finally;
	//}
//#endif

	finally:
	//&info;
	return mod;
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

int alifImport_initCore(AlifThread* tstate, AlifObject* sysmod, int importlib)
{

	if (importlib) {
		if (init_importlib(tstate, sysmod) < 0) {
			return -1; // error
		}
	}

	return 0;
}
