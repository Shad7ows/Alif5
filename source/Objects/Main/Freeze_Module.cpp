#include "alif.h"
#include "Marshal.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_Import.h"

#include "AlifCore_Import.h"
#include "Importlib._bootstrap.h"
#include "Importlib._bootstrap_external.h"
#include "ZipImport.h"

static const class Frozen _alifBootstrapModules_[] = {
	{L"_frozen_importlib", _alifMImportlibBootstrap_, (int)sizeof(_alifMImportlibBootstrap_), false},
	{L"_frozen_importlib_external", _alifMImportlibBootstrapExternal_, (int)sizeof(_alifMImportlibBootstrapExternal_), false},
	{L"zipimport", _alifMZipImport_, (int)sizeof(_alifMZipImport_), false},
	{0, 0, 0} /* bootstrap sentinel */
};

const class Frozen* _alifImportFrozenBootstrap_ = _alifBootstrapModules_;

static const class ModuleAlias aliases[] = {
	{L"_frozen_importlib", L"importlib._bootstrap"},
	{L"_frozen_importlib_external", L"importlib._bootstrap_external"},
	{L"os.path", L"posixpath"},
	{L"__hello_alias__", L"__hello__"},
	{L"__phello_alias__", L"__hello__"},
	{L"__phello_alias__.spam", L"__hello__"},
	{L"__phello__.__init__", L"<__phello__"},
	{L"__phello__.ham.__init__", L"<__phello__.ham"},
	{L"__hello_only__", NULL},
	{0, 0} /* aliases sentinel */
};
const class ModuleAlias* _alifImportFrozenAliases_ = aliases;


//const class Frozen* _alifImportFrozenBootstrap_;
const class Frozen* _alifImportFrozenStdlib_;
const class Frozen* _alifImportFrozenTest_;
const class Frozen* _alifImportFrozenModules_;
//const class ModuleAlias* _alifImportFrozenAliases_;

