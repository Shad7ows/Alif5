#pragma once






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
