#pragma once

#include "AlifCore_Hashtable.h"

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

const char* alifImport_swapPackageContext(const char* );

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
