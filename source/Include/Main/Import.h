#pragma once


// in file AlifImport_Importdll.h

enum AlifExtModuleKind {
	AlifExt_Module_Kind_Unknown = 0,
	AlifExt_Module_Kind_Singlephase = 1,
	AlifExt_Module_Kind_Multiphase = 2,
	AlifExt_Module_Kind_Invalid = 3,
} ;

enum AlifExtModuleOrigin {
	AlifExt_Module_Origin_Core = 1,
	AlifExt_Module_Origin_Builtin = 2,
	AlifExt_Module_Origin_Dynamic = 3,
};

class AlifExtModuleLoaderInfo {
public:
	AlifObject* filename;
#ifndef MS_WINDOWS
	AlifObject* filenameEncoded;
#endif
	AlifObject* name;
	AlifObject* nameEncoded;

	AlifObject* path;
	AlifExtModuleOrigin origin;
	const char* hookPrefix;
	const char* newContext;
};

class AlifExtModuleLoaderResult {
public:
	AlifModuleDef* def;
	AlifObject* module_;
	AlifExtModuleKind kind;
	class AlifExtModuleLoaderResultError* err;
	class AlifExtModuleLoaderResultError {
	public:
		enum AlifExtModuleLoaderResultErrorKind {
			AlifExt_Module_Loader_Result_Exception = 0,
			AlifExt_Module_Loader_Result_Err_Missing = 1,
			AlifExt_Module_Loader_Result_Err_Unreported_Exc = 2,
			AlifExt_Module_Loader_Result_Err_Uninitialized = 3,
			AlifExt_Module_Loader_Result_Err_Nonascii_Not_Multiphase = 4,
			AlifExt_Module_Loader_Result_Err_Not_Module = 5,
			AlifExt_Module_Loader_Result_Err_Missing_Def = 6,
		} kind;
		AlifObject* exc;
	} _err;
};

AlifObject* alifImport_addModuleRef(const wchar_t* );

class InitTable {
public:
	const wchar_t* name{};
	AlifObject* (*initFunc)(void);
};

class Frozen {
public:
	const wchar_t* name;                 /* ASCII encoded string */
	const wchar_t* code;
	int size;
	int isPackage;
};

extern const class Frozen* _alifImportFrozenModules_;
