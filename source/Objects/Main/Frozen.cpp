#include "alif.h"
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
