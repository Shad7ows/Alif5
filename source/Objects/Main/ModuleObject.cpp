#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_AlifState.h"


AlifTypeObject _alifModuleDefType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
	L"moduledef",                       
	sizeof(AlifModuleDef),
	0,
};



AlifObject* alifModuleDef_init(AlifModuleDef* def) { 
	if (def->base.index == 0) {
		ALIFSET_REFCNT(def, 1);
		ALIFSET_TYPE(def, &_alifModuleDefType_);
		//def->base.index = alifImport_getNextModuleIndex();
		def->base.index = 1; // temp
	}
	return (AlifObject*)def;
}



AlifObject* alifModule_createInitialized(AlifModuleDef* _module) { 

	const wchar_t* name{};
	AlifModuleObject* m{};

	if (!alifModuleDef_init(_module)) return nullptr;

	name = _module->name;
	if (_module->slots) {
		// error
		return nullptr;
	}
	//name = alifImport_resolveNameWithPackageContext(name);
	if ((m = (AlifModuleObject*)alifNew_module(name)) == nullptr) return nullptr;

	if (_module->size > 0) {
		m->state = alifMem_dataAlloc(_module->size);
		if (!m->state) {
			// memory error
			ALIF_DECREF(m);
			return nullptr;
		}
		//memset(m->state, 0, _module->size);
	}

	if (_module->methods != nullptr) {
		if (alifModule_addFunctions((AlifObject*)m, _module->methods) != 0) {
			ALIF_DECREF(m);
			return nullptr;
		}
	}
	if (_module->doc != nullptr) {
		//if (alifModule_setDocString((AlifObject*)m, _module->doc) != 0) {
		//	ALIF_DECREF(m);
		//	return nullptr;
		//}
	}
	m->def = _module;

	return (AlifObject*)m;
}


AlifIntT alifModule_addFunctions(AlifObject* m, AlifMethodDef* functions) { 
	AlifIntT res{};
	AlifObject* name = alifModule_getNameObject(m);
	if (name == nullptr) return -1;

	res = addMethods_toObject(m, name, functions);
	ALIF_DECREF(name);
	return res;
}


AlifObject* alifModule_getNameObject(AlifObject* mod) { 

	AlifObject* name{};
	AlifObject* name1{};

	if (!ALIFMODULE_CHECK(mod)) {
		//alifErr_badArgument();
		return nullptr;
	}
	AlifObject* dict = ((AlifModuleObject*)mod)->dict;
	if (dict == nullptr or !ALIFDICT_CHECK(dict)) {
		goto error;
	}
	name1 = alifUStr_decodeStringToUTF8(L"__name__");
	if (alifDict_getItemRef(dict, name1, &name) <= 0) {
		goto error;
	}
	if (!ALIFUSTR_CHECK(name)) {
		ALIF_DECREF(name);
		goto error;
	}
	return name;

error:
	//if (!alifErr_occurred()) {
	//	alifErr_setString(alifExcSystemError, "nameless module");
	//}
	return nullptr;
}










AlifObject* alifModule_getAttroImpl(AlifModuleObject* m, AlifObject* name, AlifIntT suppress) { 
	AlifObject* attr, * modName, * getattr;

	AlifIntT isPossiblyShadowing{};
	AlifIntT isPossiblyShadowingStdlib = 0;
	AlifObject* origin{};
	AlifObject* spec{};


	attr = alifSubObject_genericGetAttrWithDict((AlifObject*)m, name, nullptr, suppress);
	if (attr) {
		return attr;
	}
	if (suppress == 1) {
		//if (alifErr_occurred()) {
		//	return nullptr;
		//}
	}
	else {
		//if (!alifErr_exceptionMatches(alifExcAttributeError)) {
		//	return nullptr;
		//}
		//alifErr_clear();
	}

	AlifObject* name1 = alifUStr_decodeStringToUTF8(L"__getattr__");
	if (alifDict_getItemRef(m->dict, name1, &getattr) < 0) {
		return nullptr;
	}
	if (getattr) {
		AlifObject* result = alifObject_callOneArg(getattr, name);
		//if (result == nullptr and suppress == 1 and alifErr_exceptionMatches(alifExcAttributeError)) {
		//	alifErr_clear();
		//}
		ALIF_DECREF(getattr);
		return result;
	}

	if (suppress == 1) {
		return nullptr;
	}
	AlifObject* name2 = alifUStr_decodeStringToUTF8(L"__name__");
	if (alifDict_getItemRef(m->dict, name2, &modName) < 0) {
		return nullptr;
	}
	if (!modName or !ALIFUSTR_CHECK(modName)) {
		ALIF_XDECREF(modName);
		//alifErr_format(alifExcAttributeError,
		//	"module has no attribute '%U'", name);
		return nullptr;
	}

	AlifObject* name3 = alifUStr_decodeStringToUTF8(L"__spec__");
	if (alifDict_getItemRef(m->dict, name3, &spec) < 0) {
		ALIF_DECREF(modName);
		return nullptr;
	}
	if (spec == nullptr) {
		//alifErr_format(alifExcAttributeError,
		//	"module '%U' has no attribute '%U'",
		//	modName, name);
		ALIF_DECREF(modName);
		return nullptr;
	}

	origin = nullptr;
	//if (getFileOrigin_fromSpec(spec, &origin) < 0) {
	//	goto done;
	//}

	//isPossiblyShadowing = isModule_possiblyShadowing(origin);
	//if (isPossiblyShadowing < 0) {
	//	goto done;
	//}

	if (isPossiblyShadowing) {
		//AlifObject* stdlibModules = alifSys_getObject(L"stdlib_module_names");
		//if (stdlibModules and ALIFANYSET_CHECK(stdlibModules)) {
		//	isPossiblyShadowingStdlib = alifSet_contains(stdlibModules, modName);
		//	if (isPossiblyShadowingStdlib < 0) {
		//		goto done;
		//	}
		//}
	}

	if (isPossiblyShadowingStdlib) {
		//alifErr_format(alifExcAttributeError,
		//	"module '%U' has no attribute '%U' "
		//	"(consider renaming '%U' since it has the same "
		//	"name as the standard library module named '%U' "
		//	"and the import system gives it precedence)",
		//	modName, name, origin, modName);
	}
	else {
		//int rc = alifModuleSpec_isInitializing(spec);
		//if (rc > 0) {
		//	if (is_possibly_shadowing) {
		//		alifErr_format(alifExcAttributeError,
		//			"module '%U' has no attribute '%U' "
		//			"(consider renaming '%U' if it has the same name "
		//			"as a third-party module you intended to import)",
		//			modName, name, origin);
		//	}
		//	else if (origin) {
		//		alifErr_format(alifExcAttributeError,
		//			"partially initialized "
		//			"module '%U' from '%U' has no attribute '%U' "
		//			"(most likely due to a circular import)",
		//			modName, origin, name);
		//	}
		//	else {
		//		alifErr_format(alifExcAttributeError,
		//			"partially initialized "
		//			"module '%U' has no attribute '%U' "
		//			"(most likely due to a circular import)",
		//			modName, name);
		//	}
		//}
		//else if (rc == 0) {
		//	rc = alifModuleSpec_isUninitializedSubmodule(spec, name);
		//	if (rc > 0) {
		//		alifErr_format(alifExcAttributeError,
		//			"cannot access submodule '%U' of module '%U' "
		//			"(most likely due to a circular import)",
		//			name, modName);
		//	}
		//	else if (rc == 0) {
		//		alifErr_format(alifExcAttributeError,
		//			"module '%U' has no attribute '%U'",
		//			modName, name);
		//	}
		//}
	}

done:
	ALIF_XDECREF(origin);
	ALIF_DECREF(spec);
	ALIF_DECREF(modName);
	return nullptr;
}


AlifObject* alifModule_getAttro(AlifModuleObject* m, AlifObject* name) { 
	return alifModule_getAttroImpl(m, name, 0);
}









static int moduleInit_dict(AlifModuleObject* _mod, AlifObject* _dict, AlifObject* _name, AlifObject* _doc)
{
	if (_doc == nullptr)
		_doc = ALIF_NONE;
	AlifObject* nameName = alifUStr_fromString(L"__name__");
	if (alifDict_setItem(_dict, nameName, _name) != 0)
		return -1;
	AlifObject* nameDoc = alifUStr_fromString(L"__doc__");
	if (alifDict_setItem(_dict, nameDoc, _doc) != 0)
		return -1;
	AlifObject* namePackage = alifUStr_fromString(L"__package__");
	if (alifDict_setItem(_dict, namePackage, ALIF_NONE) != 0)
		return -1;
	AlifObject* nameLoader = alifUStr_fromString(L"__loader__");
	if (alifDict_setItem(_dict, nameLoader, ALIF_NONE) != 0)
		return -1;
	AlifObject* nameSpac = alifUStr_fromString(L"__spac__");
	if (alifDict_setItem(_dict, nameSpac, ALIF_NONE) != 0)
		return -1;
	if (ALIFUSTR_CHECK(_name)) {
		ALIF_XSETREF(_mod->name, ALIF_NEWREF(_name));
	}

	return 0;
}

static AlifModuleObject* newModule_noTrack(AlifTypeObject* _moduleType)
{
	AlifModuleObject* m_;
	m_ = (AlifModuleObject*)alifType_allocNoTrack(_moduleType, 0);
	if (m_ == nullptr)
		return nullptr;
	m_->def = nullptr;
	m_->state = nullptr;
	m_->weakList = nullptr;
	m_->name = nullptr;
	m_->dict = alifNew_dict();
	if (m_->dict != nullptr) {
		return m_;
	}
	ALIF_DECREF(m_);
	return nullptr;
}

static AlifObject* new_module(AlifTypeObject* mt, AlifObject* args, AlifObject* kws)
{
	AlifObject* m = (AlifObject*)newModule_noTrack(mt);
	if (m != nullptr) {
		alifObject_gc_track(m);
	}
	return m;
}

AlifObject* alifModule_newObject(AlifObject* _name)
{
	AlifModuleObject* m_ = newModule_noTrack(&_alifModuleType_);
	if (m_ == nullptr)
		return nullptr;
	if (moduleInit_dict(m_, m_->dict, _name, nullptr) != 0)
		goto fail;
	alifObject_gc_track(m_);
	return (AlifObject*)m_;

fail:
	ALIF_DECREF(m_);
	return nullptr;
}

AlifObject* alifNew_module(const wchar_t* _name)
{
	AlifObject* nameObj, * module;
	nameObj = alifUStr_fromString(_name);
	if (nameObj == nullptr)
		return nullptr;
	module = alifModule_newObject(nameObj);
	ALIF_DECREF(nameObj);
	return module;
}

static AlifIntT addMethods_toObject(AlifObject* _module, AlifObject* _name, AlifMethodDef* _functions)
{
	AlifObject* func{};
	AlifMethodDef* fDef{};

	for (fDef = _functions; fDef->name != nullptr; fDef++) {
		if ((fDef->flags & METHOD_CLASS) or
			(fDef->flags & METHOD_STATIC)) {
			// error
			return -1;
		}
		func = ALIFCFUNCTION_NEWEX(fDef, (AlifObject*)_module, _name);
		if (func == nullptr) return -1;

		//alifObject_setDeferredRefCount(func); // need review
		if (alifObject_setAttrString(_module, fDef->name, func) != 0) {
			ALIF_DECREF(func);
			return -1;
		}
		ALIF_DECREF(func);
	}

	return 0;
}

AlifObject* alifModule_fromDefAndSpec2(AlifModuleDef* def, AlifObject* spec, int module_api_version)
{
	AlifModuleDefSlot* cur_slot;
	AlifObject* (*create)(AlifObject*, AlifModuleDef*) = NULL;
	AlifObject* nameobj;
	AlifObject* m = NULL;
	int has_multiple_interpreters_slot = 0;
	void* multiple_interpreters = (void*)0;
	int has_gil_slot = 0;
	void* gil_slot = (void*)0;
	int has_execution_slots = 0;
	const wchar_t* name;
	int ret;
	AlifInterpreter* interp = alifInterpreter_get();

	alifModuleDef_init(def);

	nameobj = alifObject_getAttrString(spec, L"name");
	if (nameobj == NULL) {
		return NULL;
	}
	name = alifUStr_asUTF8(nameobj);
	if (name == NULL) {
		goto error;
	}

	//if (!check_api_version(name, module_api_version)) {
		//goto error;
	//}

	if (def->size < 0) {
		goto error;
	}

	for (cur_slot = def->slots; cur_slot && cur_slot->slot; cur_slot++) {
		switch (cur_slot->slot) {
		case 1:
			if (create) {
				goto error;
			}
			//create = cur_slot->value;
			break;
		case 2:
			has_execution_slots = 1;
			break;
		case 3:
			if (has_multiple_interpreters_slot) {
				goto error;
			}
			multiple_interpreters = cur_slot->value;
			has_multiple_interpreters_slot = 1;
			break;
		case 4:
			if (has_gil_slot) {;
				goto error;
			}
			gil_slot = cur_slot->value;
			has_gil_slot = 1;
			break;
		default:
			goto error;
		}
	}

	if (!has_multiple_interpreters_slot) {
		multiple_interpreters = ((void*)1);
	}
	if (multiple_interpreters == (void*)0) {
		if (!(interp == _alifDureRun_.interpreters.main)
			&& alifImport_checkSubinterpIncompatibleExtensionAllowed(name) < 0)
		{
			goto error;
		}
	}
	else if (multiple_interpreters != ((void*)2)
		//&& interp->ceval.ownGil
		&& !(interp == _alifDureRun_.interpreters.main)
		&& alifImport_checkSubinterpIncompatibleExtensionAllowed(name) < 0)
	{
		goto error;
	}

	if (create) {
		m = create(spec, def);
		if (m == NULL) {
			goto error;
		}
	}
	else {
		m = alifModule_newObject(nameobj);
		if (m == NULL) {
			goto error;
		}
	}

	if (ALIFMODULE_CHECK(m)) {
		((AlifModuleObject*)m)->state = NULL;
		((AlifModuleObject*)m)->def = def;
//#ifdef ALIF_GIL_DISABLED
		//((AlifModuleObject*)m)->md_gil = gil_slot;
//#else
		(void)gil_slot;
//#endif
	}
	else {
		if (def->size > 0 || def->traverse || def->clear || def->free) {
			goto error;
		}
		if (has_execution_slots) {
			goto error;
		}
	}

	if (def->methods != NULL) {
		ret = addMethods_toObject(m, nameobj, def->methods);
		if (ret != 0) {
			goto error;
		}
	}

	if (def->doc != NULL) {
		//ret = alifModule_setDocString(m, def->doc);
		if (ret != 0) {
			goto error;
		}
	}

	ALIF_DECREF(nameobj);
	return m;

error:
	ALIF_DECREF(nameobj);
	ALIF_XDECREF(m);
	return NULL;
}

int alifModule_execDef(AlifObject* module, AlifModuleDef* def)
{
	AlifModuleDefSlot* cur_slot;
	const wchar_t* name;
	int ret;

	name = alifModule_getName(module);
	if (name == NULL) {
		return -1;
	}

	if (def->size >= 0) {
		AlifModuleObject* md = (AlifModuleObject*)module;
		if (md->state == NULL) {
			md->state = alifMem_objAlloc(def->size);
			if (!md->state) {
				return -1;
			}
			memset(md->state, 0, def->size);
		}
	}

	if (def->slots == NULL) {
		return 0;
	}

	for (cur_slot = def->slots; cur_slot && cur_slot->slot; cur_slot++) {
		switch (cur_slot->slot) {
		case 1:
			break;
		case 2:
			ret = ((int (*)(AlifObject*))cur_slot->value)(module);
			if (ret != 0) {

				return -1;
			}
			break;
		case 3:
		case 4:
			break;
		default:
			return -1;
		}
	}
	return 0;
}

const wchar_t* alifModule_getName(AlifObject* m)
{
	AlifObject* name = alifModule_getNameObject(m);
	if (name == NULL) {
		return NULL;
	}
	ALIF_DECREF(name);   /* module dict has still a reference */
	return alifUStr_asUTF8(name);
}

AlifObject* alifModule_getDict(AlifObject* _m)
{
	if (!(_m->type_ == &_alifModuleType_)) {
		return nullptr;
	}
	return alifSubModule_getDict(_m);  // borrowed reference
}

AlifModuleDef* alifModule_getDef(AlifObject* _m)
{
	if (!ALIFMODULE_CHECK(_m)) {
		return NULL;
	}
	return alifSubModule_getDef(_m);
}

AlifTypeObject _alifModuleType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
	L"module",                                 
	sizeof(AlifModuleObject),                  
	0,                                         
	0, //(destructor)module_dealloc,           
	0,                                         
	0,                                         
	0,                                         
	0, //(reprfunc)module_repr,                
	0,                                         
	0,                                         
	0,                                         
	0,                                         
	0,                                         
	0,                                         
	(GetAttroFunc)alifObject_genericGetAttr, // worng and need fix                 
	alifObject_genericSetAttr,
	0,                                         
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC |
		ALIFTPFLAGS_BASETYPE,                  
	0, //(traverseproc)module_traverse,        
	0, //(inquiry)module_clear,                
	0,                                         
	0,                                         
	offsetof(AlifModuleObject, weakList),      
	0,                                         
	0, //module_methods,                       
	0, //module_members,                       
	0,//module_getsets,                        
	0,                                         
	0,                                         
	0,                                         
	0,
	0,
	offsetof(AlifModuleObject, dict),                      
	0,                                         
	0,
	new_module,                                
	alifObject_gcDel,                          
};
