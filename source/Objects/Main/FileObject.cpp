#include "alif.h"
#include "AlifCore_Call.h"
//#include "AlifCore_runtime.h"



// هذا الملف لا يحتوي AlifCore_FileObject.h

char* alifUniversal_newLineFGetsWithSize(char* _buf, AlifIntT _n, FILE* _stream, AlifSizeT* _size) {
	char* p = _buf;
	AlifIntT c{};
	while (--_n > 0 and (c = getc(_stream)) != EOF) {
		if (c == '\r') {
			c = getc(_stream);
			if (c != '\n') {
				ungetc(c, _stream);
				c = '\n';
			}
		}
		*p++ = c;
		if (c == '\n') {
			break;
		}
	}

	*p = '\0';
	if (p == _buf) {
		return nullptr;
	}
	*_size = p - _buf;
	return _buf;

}
