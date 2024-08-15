#include "alif.h"
#include "AlifCore_Call.h"
#include "AlifCore_Memory.h"
#include "AlifCore_Code.h"
#include <AlifCore_AlifState.h>


#define TYPE_NULL               '0'
#define TYPE_NONE               'N'
#define TYPE_FALSE              'F'
#define TYPE_TRUE               'T'
#define TYPE_STOPITER           'S'
#define TYPE_ELLIPSIS           '.'
#define TYPE_INT                'i'
/* TYPE_INT64 is not generated anymore.
   Supported for backward compatibility only. */
#define TYPE_INT64              'I'
#define TYPE_FLOAT              'f'
#define TYPE_BINARY_FLOAT       'g'
#define TYPE_COMPLEX            'x'
#define TYPE_BINARY_COMPLEX     'y'
#define TYPE_LONG               'l'
#define TYPE_STRING             's'
#define TYPE_INTERNED           't'
#define TYPE_REF                'r'
#define TYPE_TUPLE              '('
#define TYPE_LIST               '['
#define TYPE_DICT               '{'
#define TYPE_CODE               'c'
#define TYPE_UNICODE            'u'
#define TYPE_UNKNOWN            '?'
#define TYPE_SET                '<'
#define TYPE_FROZENSET          '>'
#define FLAG_REF                '\x80' /* with a type, add obj to index */

#define TYPE_ASCII              'a'
#define TYPE_ASCII_INTERNED     'A'
#define TYPE_SMALL_TUPLE        ')'
#define TYPE_SHORT_ASCII        'z'
#define TYPE_SHORT_ASCII_INTERNED 'Z'

#define WFERR_OK 0
#define WFERR_UNMARSHALLABLE 1
#define WFERR_NESTEDTOODEEP 2
#define WFERR_NOMEMORY 3
#define WFERR_CODE_NOT_ALLOWED 4

typedef class Rfile{ // 696
public:
	FILE* fp;
	int depth;
	AlifObject* readable;  /* Stream-like object being read from */
	const char* ptr;
	const char* end;
	char* buf;
	int64_t bufSize;
	AlifObject* refs;  /* a list */
	int allowCode;
} ;

#define SIZE32_MAX  0x7FFFFFFF


static const char* r_string(int64_t n, Rfile* p) // 707
{
	int64_t read = -1;

	if (p->ptr != NULL) {
		/* Fast path for loads() */
		const char* res = p->ptr;
		int64_t left = p->end - p->ptr;
		if (left < n) {

			return NULL;
		}
		p->ptr += n;
		return res;
	}
	if (p->buf == NULL) {
		p->buf = (char*)alifMem_dataAlloc(n);
		if (p->buf == NULL) {
			return NULL;
		}
		p->bufSize = n;
	}
	else if (p->bufSize < n) {
		char* tmp = (char*)alifMem_objRealloc(p->buf, n);
		if (tmp == NULL) {
			return NULL;
		}
		p->buf = tmp;
		p->bufSize = n;
	}

	if (!p->readable) {
		read = fread(p->buf, 1, n, p->fp);
	}
	else {
		AlifObject* res, * mview;
		AlifBuffer buf;

		if (alifBuffer_fillInfo(&buf, NULL, p->buf, n, 0, ALIFBUF_CONTIG) == -1)
			return NULL;
		mview = alifMemoryView_fromBuffer(&buf);
		if (mview == NULL)
			return NULL;

		AlifObject* strReadInto = alifUStr_fromString(L"readinto");
		res = alifSubObject_callMethod(p->readable, strReadInto, L"N", mview);
		if (res != NULL) {
			read = alifInteger_asLongLong(res);
			ALIF_DECREF(res);
		}
	}
	if (read != n) {
		return NULL;
	}
	return p->buf;
}

static int r_byte(Rfile* p)
{
	if (p->ptr != NULL) {
		if (p->ptr < p->end) {
			return (unsigned char)*p->ptr++;
		}
	}
	else if (!p->readable) {
		int c = getc(p->fp);
		if (c != EOF) {
			return c;
		}
	}
	else {
		const char* ptr = r_string(1, p);
		if (ptr != NULL) {
			return *(const unsigned char*)ptr;
		}
		return EOF;
	}

	return EOF;
}

static long r_long(Rfile* p)
{
	long x = -1;
	const unsigned char* buffer;

	buffer = (const unsigned char*)r_string(4, p);
	if (buffer != NULL) {
		x = buffer[0];
		x |= (long)buffer[1] << 8;
		x |= (long)buffer[2] << 16;
		x |= (long)buffer[3] << 24;
#if SIZEOF_LONG > 4
		/* Sign extension for 64-bit machines */
		x |= -(x & 0x80000000L);
#endif
	}
	return x;
}

static double r_float_bin(Rfile* p)
{
	const char* buf = r_string(8, p);
	if (buf == NULL)
		return -1;
	return 1;
	//return alifFloat_unpack8(buf, 1);
}


static double r_float_str(Rfile* p)
{
	int n;
	char buf[256];
	const char* ptr;
	n = r_byte(p);
	if (n == EOF) {
		return -1;
	}
	ptr = r_string(n, p);
	if (ptr == NULL) {
		return -1;
	}
	memcpy(buf, ptr, n);
	buf[n] = '\0';
	return 1;
	//return alifOS_string_to_double(buf, NULL, NULL);
}

static int64_t r_ref_reserve(int flag, Rfile* p)
{
	if (flag) { /* currently only FLAG_REF is defined */
		int64_t idx = ALIFLIST_GET_SIZE(p->refs);
		if (idx >= 0x7ffffffe) {
			return -1;
		}
		if (alifList_append(p->refs, ALIF_NONE) < 0)
			return -1;
		return idx;
	}
	else
		return 0;
}

static AlifObject* r_ref_insert(AlifObject* o, int64_t idx, int flag, Rfile* p)
{
	if (o != NULL && flag) { /* currently only FLAG_REF is defined */
		AlifObject* tmp = ALIFLIST_GET_ITEM(p->refs, idx);
		ALIFLIST_SETITEM(p->refs, idx, ALIF_NEWREF(o));
		ALIF_DECREF(tmp);
	}
	return o;
}

static AlifObject* r_ref(AlifObject* o, int flag, Rfile* p)
{
	if (o == NULL)
		return NULL;
	if (alifList_append(p->refs, o) < 0) {
		ALIF_DECREF(o); 
		return NULL;
	}
	return o;
}

static AlifObject* r_object(Rfile* p) // 1005
{

	AlifObject* v{}, * v2;
	int64_t idx = 0;
	long i, n;
	int type, code = r_byte(p);
	if (code == 0) {

	}
	int flag, is_interned = 0;
	AlifObject* retval = NULL;

	if (code == EOF) {
		return NULL;
	}

	p->depth++;

	if (p->depth > 1000) {
		p->depth--;
		return NULL;
	}

	flag = code & FLAG_REF;
	type = code & ~FLAG_REF;

#define R_REF(O) do{\
    if (flag) \
        O = r_ref(O, flag, p);\
} while (0)

	switch (type) {

	case TYPE_NULL:
		break;

	case TYPE_NONE:
		retval = ALIF_NONE;
		break;

	case TYPE_STOPITER:
		//retval = ALIF_NEWREF(_alifExcStopIteration_);
		exit(-1);

		break;

	case TYPE_ELLIPSIS:
		//retval = ALIF_ELLIPSIS;
		exit(-1);

		break;

	case TYPE_FALSE:
		retval = ALIF_FALSE;
		break;

	case TYPE_TRUE:
		retval = ALIF_TRUE;
		break;

	case TYPE_INT:
		n = r_long(p);
		retval = alifInteger_fromLongLong(n);
		R_REF(retval);
		break;

	case TYPE_INT64:
		exit(-1);
		//	retval = r_long64(p);
	//	R_REF(retval);
		break;

	case TYPE_LONG:
		exit(-1);

	//	retval = r_alifLong(p);
	//	R_REF(retval);
		break;

	case TYPE_FLOAT:
	{
		double x = r_float_str(p);
		retval = alifFloat_fromDouble(x);
		R_REF(retval);
		break;
	}

	case TYPE_BINARY_FLOAT:
	{
		double x = r_float_bin(p);
		retval = alifFloat_fromDouble(x);
		R_REF(retval);
		break;
	}

	case TYPE_COMPLEX:
	{
		exit(-1);
	//	alif_complex c;
	//	c.real = r_float_str(p);
	//	c.imag = r_float_str(p);
	//	retval = alifComplex_FromCComplex(c);
	//	R_REF(retval);
	//	break;
	}

	case TYPE_BINARY_COMPLEX:
	{
		exit(-1);
	//	alif_complex c;
	//	c.real = r_float_bin(p);
	//	c.imag = r_float_bin(p);
	//	retval = alifComplex_FromCComplex(c);
	//	R_REF(retval);
	//	break;
	}

	case TYPE_STRING:
	{
		const char* ptr;
		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			
			break;
		}
		v = alifBytes_fromStringAndSize((wchar_t*)NULL, n);
		if (v == NULL)
			break;
		ptr = r_string(n, p);
		if (ptr == NULL) {
			ALIF_DECREF(v);
			break;
		}
		memcpy(ALIFWBYTES_AS_STRING(v), ptr, n);
		retval = v;
		R_REF(retval);
		break;
	}

	case TYPE_ASCII_INTERNED:
		is_interned = 1;
		ALIFFALLTHROUGH;
	case TYPE_ASCII:
		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			break;
		}
		goto _read_ascii;

	case TYPE_SHORT_ASCII_INTERNED:
		is_interned = 1;
		ALIFFALLTHROUGH;
	case TYPE_SHORT_ASCII:
		n = r_byte(p);
		if (n == EOF) {
			break;
		}
	_read_ascii:
		{
			const char* ptr;
			ptr = r_string(n, p);
			if (ptr == NULL)
				break;
			v = alifUStr_fromKindAndData(USTR_2BYTE, ptr, n);
			if (v == NULL)
				break;
			if (is_interned) {
				// marshal is meant to serialize .alifc files with code
				// objects, and code-related strings are currently immortal.
				AlifInterpreter* interp = alifInterpreter_get();
				//alifUStr_internImmortal(interp, &v);
			}
			retval = v;
			R_REF(retval);
			break;
		}

	case TYPE_INTERNED:
		is_interned = 1;
		ALIFFALLTHROUGH;
	case TYPE_UNICODE:
	{
		const char* buffer;

		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			break;
		}
		if (n != 0) {
			buffer = r_string(n, p);
			if (buffer == NULL)
				break;
			v = alifUStr_decodeUTF8((wchar_t*)buffer, n, L"surrogatepass");
		}
		else {
			v = alifNew_uStr(0, 0);
		}
		if (v == NULL)
			break;
		if (is_interned) {
			// marshal is meant to serialize .alifc files with code
			// objects, and code-related strings are currently immortal.
			//AlifInterpreterState* interp = alifInterpreterState_GET();
			//alifUnicode_internImmortal(interp, &v);
		}
		retval = v;
		R_REF(retval);
		break;
	}

	case TYPE_SMALL_TUPLE:
		n = r_byte(p);
		if (n == EOF) {
			break;
		}
		goto _read_tuple;
	case TYPE_TUPLE:
		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			break;
		}
	_read_tuple:
		v = alifNew_tuple(n);
		R_REF(v);
		if (v == NULL)
			break;

		for (i = 0; i < n; i++) {
			if (n == 10 && i == 4) {
				int a = 1 + 3;
			}
			v2 = r_object(p);
			if (v2 == NULL) {
				ALIF_SETREF(v, NULL);
				break;
			}
			ALIFTUPLE_SET_ITEM(v, i, v2);
		}
		retval = v;
		break;

	case TYPE_LIST:
		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			break;
		}
		v = alifNew_list(n);
		R_REF(v);
		if (v == NULL)
			break;
		for (i = 0; i < n; i++) {
			v2 = r_object(p);
			if (v2 == NULL) {
				ALIF_SETREF(v, NULL);
				break;
			}
			ALIFLIST_SETITEM(v, i, v2);
		}
		retval = v;
		break;

	case TYPE_DICT:
		v = alifNew_dict();
		R_REF(v);
		if (v == NULL)
			break;
		for (;;) {
			AlifObject* key, * val;
			key = r_object(p);
			if (key == NULL)
				break;
			val = r_object(p);
			if (val == NULL) {
				ALIF_DECREF(key);
				break;
			}
			if (alifDict_setItem(v, key, val) < 0) {
				ALIF_DECREF(key);
				ALIF_DECREF(val);
				break;
			}
			ALIF_DECREF(key);
			ALIF_DECREF(val);
		}
		retval = v;
		break;

	case TYPE_SET:
	case TYPE_FROZENSET:
		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			break;
		}

		if (n == 0 && type == TYPE_FROZENSET) {
			/* call frozenset() to get the empty frozenset singleton */
			//v = alifObject_CallNoArgs((AlifObject*)&_alifFrozenSetType_);
			if (v == NULL)
				break;
			R_REF(v);
			retval = v;
		}
		else {
			v = (type == TYPE_SET) ? alifNew_set(NULL) : nullptr;/*alifFrozenSet_New(NULL);*/
			if (type == TYPE_SET) {
				R_REF(v);
			}
			else {

				idx = r_ref_reserve(flag, p);
				if (idx < 0)
					ALIF_CLEAR(v); /* signal error */
			}
			if (v == NULL)
				break;

			for (i = 0; i < n; i++) {
				v2 = r_object(p);
				if (v2 == NULL) {
					ALIF_SETREF(v, NULL);
					break;
				}
				if (alifSet_add(v, v2) == -1) {
					ALIF_DECREF(v);
					ALIF_DECREF(v2);
					v = NULL;
					break;
				}
				ALIF_DECREF(v2);
			}
			if (type != TYPE_SET)
				v = r_ref_insert(v, idx, flag, p);
			retval = v;
		}
		break;

	case TYPE_CODE:
	{
		int argcount;
		int posonlyargcount;
		int kwonlyargcount;
		int stacksize;
		int flags;
		AlifObject* code = NULL;
		AlifObject* consts = NULL;
		AlifObject* names = NULL;
		AlifObject* localsplusnames = NULL;
		AlifObject* localspluskinds = NULL;
		AlifObject* filename = NULL;
		AlifObject* name = NULL;
		AlifObject* qualname = NULL;
		int firstlineno;
		AlifObject* linetable = NULL;
		AlifObject* exceptiontable = NULL;

		if (!p->allowCode) {
			break;
		}
		idx = r_ref_reserve(flag, p);
		if (idx < 0)
			break;

		v = NULL;

		/* XXX ignore long->int overflows for now */
		argcount = (int)r_long(p);
		posonlyargcount = (int)r_long(p);
		kwonlyargcount = (int)r_long(p);
		stacksize = (int)r_long(p);
		flags = (int)r_long(p);
		code = r_object(p);
		class AlifCodeConstructor con {};
		if (code == NULL)
			goto code_error;
		consts = r_object(p);
		if (consts == NULL)
			goto code_error;
		names = r_object(p);
		if (names == NULL)
			goto code_error;
		localsplusnames = r_object(p);
		if (localsplusnames == NULL)
			goto code_error;
		localspluskinds = r_object(p);
		if (localspluskinds == NULL)
			goto code_error;
		filename = r_object(p);
		if (filename == NULL)
			goto code_error;
		name = r_object(p);
		if (name == NULL)
			goto code_error;
		qualname = r_object(p);
		if (qualname == NULL)
			goto code_error;
		firstlineno = (int)r_long(p);
		linetable = r_object(p);
		if (linetable == NULL)
			goto code_error;
		exceptiontable = r_object(p);
		if (exceptiontable == NULL)
			goto code_error;

		con = {
			 filename,
		     name,
			 qualname,
			 flags,
			code,
			firstlineno,
			 linetable,
			 consts,
			names,
			 localsplusnames,
			 localspluskinds,
			 argcount,
			posonlyargcount,
		 kwonlyargcount,
			stacksize,
			exceptiontable
		};

		if (alifCode_validate(&con) < 0) {
			goto code_error;
		}

		v = (AlifObject*)alifCode_new(&con);
		if (v == NULL) {
			goto code_error;
		}

		v = r_ref_insert(v, idx, flag, p);

	code_error:
		ALIF_XDECREF(code);
		ALIF_XDECREF(consts);
		ALIF_XDECREF(names);
		ALIF_XDECREF(localsplusnames);
		ALIF_XDECREF(localspluskinds);
		ALIF_XDECREF(filename);
		ALIF_XDECREF(name);
		ALIF_XDECREF(qualname);
		ALIF_XDECREF(linetable);
		ALIF_XDECREF(exceptiontable);
	}
	retval = v;
	break;

	case TYPE_REF:
		n = r_long(p);
		if (n < 0 || n >= ALIFLIST_GET_SIZE(p->refs)) {
			break;
		}
		v = ALIFLIST_GET_ITEM(p->refs, n);
		if (v == ALIF_NONE) {
			break;
		}
		retval = ALIF_NEWREF(v);
		break;

	default:

		break;

	}
	p->depth--;
	return retval;
}

static AlifObject* read_object(Rfile* p) // 1548
{
	AlifObject* v;
	v = r_object(p);

	return v;
}

AlifObject* alifMarshal_readObjectFromString(const char* str, int64_t len) // 1670
{
	Rfile rf;
	AlifObject* result{};
	rf.allowCode = 1;
	rf.fp = NULL;
	rf.readable = NULL;
	rf.ptr = str;
	rf.end = str + len;
	rf.buf = NULL;
	rf.depth = 0;
	rf.refs = alifNew_list(0);
	if (rf.refs == NULL)
		return NULL;
	result = read_object(&rf);
	ALIF_DECREF(rf.refs);
	if (rf.buf != NULL)
		alifMem_dataFree(rf.buf);
	return result;
}
