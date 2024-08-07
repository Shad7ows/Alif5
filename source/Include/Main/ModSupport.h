#pragma once


#define ALIF_CLEANUP_SUPPORTED 0x20000

AlifObject* alif_buildValue(const wchar_t* , ...);

AlifIntT alifModule_addFunctions(AlifObject*, AlifMethodDef*);
AlifObject* alif_vaBuildValue(const wchar_t*, va_list);

int alifArg_parseTuple(AlifObject* , const wchar_t* , ...);

