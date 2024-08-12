#pragma once

#define ALIFSINGLE_INPUT 256
#define ALIFFILE_INPUT 257
#define ALIFEVAL_INPUT 258
#define ALIFFUNCTYPE_INPUT 345


#define ALIFCF_MASK (CO_FUTURE_DIVISION | CO_FUTURE_ABSOLUTE_IMPORT | \
                   CO_FUTURE_WITH_STATEMENT | CO_FUTURE_PRINT_FUNCTION | \
                   CO_FUTURE_UNICODE_LITERALS | CO_FUTURE_BARRY_AS_BDFL | \
                   CO_FUTURE_GENERATOR_STOP | CO_FUTURE_ANNOTATIONS)

class AlifCompilerFlags{
public:
	int cfFlags;  /* bitmask of CO_xxx flags relevant to future */
	int cfFeatureVersion;  /* minor Python version (PyCF_ONLY_AST) */
} ;
