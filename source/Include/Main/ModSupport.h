#pragma once


#define ALIF_CLEANUP_SUPPORTED 0x20000

AlifObject* alif_buildValue(const wchar_t* , ...);

AlifIntT alifModule_addFunctions(AlifObject*, AlifMethodDef*);
AlifObject* alif_vaBuildValue(const wchar_t*, va_list);

int alifModule_addObjectRef(AlifObject* , const wchar_t* , AlifObject* );

int alifModule_execDef(AlifObject* , AlifModuleDef*);

AlifObject* alifModule_fromDefAndSpec2(AlifModuleDef* , AlifObject* , int );

#define ALIFMODULE_FROMDEFANDSPEC(module, spec) \
    alifModule_fromDefAndSpec2((module), (spec), 0)

int alifArg_parseTuple(AlifObject* , const wchar_t* , ...);

int alifModule_addObjectRef(AlifObject* , const wchar_t* , AlifObject* );
