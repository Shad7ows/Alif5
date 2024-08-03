#include "alif.h"
#include "AlifCore_Memory.h"
#include"AlifCore_Code.h"

typedef class Rfile{ // 696
public:
	FILE* fp;
	int depth;
	AlifObject* readable;  /* Stream-like object being read from */
	const wchar_t* ptr;
	const wchar_t* end;
	wchar_t* buf;
	int64_t bufSize;
	AlifObject* refs;  /* a list */
	int allowCode;
} ;

static AlifObject* r_object(Rfile* p)
{

	AlifObject* v, * v2;
	int64_t idx = 0;
	long i, n;
	int type, code = r_byte(p);
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
		retval = Py_None;
		break;

	case TYPE_STOPITER:
		retval = Py_NewRef(PyExc_StopIteration);
		break;

	case TYPE_ELLIPSIS:
		retval = Py_Ellipsis;
		break;

	case TYPE_FALSE:
		retval = Py_False;
		break;

	case TYPE_TRUE:
		retval = Py_True;
		break;

	case TYPE_INT:
		n = r_long(p);
		if (n == -1 && PyErr_Occurred()) {
			break;
		}
		retval = PyLong_FromLong(n);
		R_REF(retval);
		break;

	case TYPE_INT64:
		retval = r_long64(p);
		R_REF(retval);
		break;

	case TYPE_LONG:
		retval = r_PyLong(p);
		R_REF(retval);
		break;

	case TYPE_FLOAT:
	{
		double x = r_float_str(p);
		if (x == -1.0 && PyErr_Occurred())
			break;
		retval = PyFloat_FromDouble(x);
		R_REF(retval);
		break;
	}

	case TYPE_BINARY_FLOAT:
	{
		double x = r_float_bin(p);
		if (x == -1.0 && PyErr_Occurred())
			break;
		retval = PyFloat_FromDouble(x);
		R_REF(retval);
		break;
	}

	case TYPE_COMPLEX:
	{
		Py_complex c;
		c.real = r_float_str(p);
		if (c.real == -1.0 && PyErr_Occurred())
			break;
		c.imag = r_float_str(p);
		if (c.imag == -1.0 && PyErr_Occurred())
			break;
		retval = PyComplex_FromCComplex(c);
		R_REF(retval);
		break;
	}

	case TYPE_BINARY_COMPLEX:
	{
		Py_complex c;
		c.real = r_float_bin(p);
		if (c.real == -1.0 && PyErr_Occurred())
			break;
		c.imag = r_float_bin(p);
		if (c.imag == -1.0 && PyErr_Occurred())
			break;
		retval = PyComplex_FromCComplex(c);
		R_REF(retval);
		break;
	}

	case TYPE_STRING:
	{
		const char* ptr;
		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			if (!PyErr_Occurred()) {
				PyErr_SetString(PyExc_ValueError,
					"bad marshal data (bytes object size out of range)");
			}
			break;
		}
		v = PyBytes_FromStringAndSize((char*)NULL, n);
		if (v == NULL)
			break;
		ptr = r_string(n, p);
		if (ptr == NULL) {
			Py_DECREF(v);
			break;
		}
		memcpy(PyBytes_AS_STRING(v), ptr, n);
		retval = v;
		R_REF(retval);
		break;
	}

	case TYPE_ASCII_INTERNED:
		is_interned = 1;
		_Py_FALLTHROUGH;
	case TYPE_ASCII:
		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			if (!PyErr_Occurred()) {
				PyErr_SetString(PyExc_ValueError,
					"bad marshal data (string size out of range)");
			}
			break;
		}
		goto _read_ascii;

	case TYPE_SHORT_ASCII_INTERNED:
		is_interned = 1;
		_Py_FALLTHROUGH;
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
			v = PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, ptr, n);
			if (v == NULL)
				break;
			if (is_interned) {
				// marshal is meant to serialize .pyc files with code
				// objects, and code-related strings are currently immortal.
				PyInterpreterState* interp = _PyInterpreterState_GET();
				_PyUnicode_InternImmortal(interp, &v);
			}
			retval = v;
			R_REF(retval);
			break;
		}

	case TYPE_INTERNED:
		is_interned = 1;
		_Py_FALLTHROUGH;
	case TYPE_UNICODE:
	{
		const char* buffer;

		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			if (!PyErr_Occurred()) {
				PyErr_SetString(PyExc_ValueError,
					"bad marshal data (string size out of range)");
			}
			break;
		}
		if (n != 0) {
			buffer = r_string(n, p);
			if (buffer == NULL)
				break;
			v = PyUnicode_DecodeUTF8(buffer, n, "surrogatepass");
		}
		else {
			v = PyUnicode_New(0, 0);
		}
		if (v == NULL)
			break;
		if (is_interned) {
			// marshal is meant to serialize .pyc files with code
			// objects, and code-related strings are currently immortal.
			PyInterpreterState* interp = _PyInterpreterState_GET();
			_PyUnicode_InternImmortal(interp, &v);
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
			if (!PyErr_Occurred()) {
				PyErr_SetString(PyExc_ValueError,
					"bad marshal data (tuple size out of range)");
			}
			break;
		}
	_read_tuple:
		v = PyTuple_New(n);
		R_REF(v);
		if (v == NULL)
			break;

		for (i = 0; i < n; i++) {
			v2 = r_object(p);
			if (v2 == NULL) {
				if (!PyErr_Occurred())
					PyErr_SetString(PyExc_TypeError,
						"NULL object in marshal data for tuple");
				Py_SETREF(v, NULL);
				break;
			}
			PyTuple_SET_ITEM(v, i, v2);
		}
		retval = v;
		break;

	case TYPE_LIST:
		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			if (!PyErr_Occurred()) {
				PyErr_SetString(PyExc_ValueError,
					"bad marshal data (list size out of range)");
			}
			break;
		}
		v = PyList_New(n);
		R_REF(v);
		if (v == NULL)
			break;
		for (i = 0; i < n; i++) {
			v2 = r_object(p);
			if (v2 == NULL) {
				if (!PyErr_Occurred())
					PyErr_SetString(PyExc_TypeError,
						"NULL object in marshal data for list");
				Py_SETREF(v, NULL);
				break;
			}
			PyList_SET_ITEM(v, i, v2);
		}
		retval = v;
		break;

	case TYPE_DICT:
		v = PyDict_New();
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
				Py_DECREF(key);
				break;
			}
			if (PyDict_SetItem(v, key, val) < 0) {
				Py_DECREF(key);
				Py_DECREF(val);
				break;
			}
			Py_DECREF(key);
			Py_DECREF(val);
		}
		if (PyErr_Occurred()) {
			Py_SETREF(v, NULL);
		}
		retval = v;
		break;

	case TYPE_SET:
	case TYPE_FROZENSET:
		n = r_long(p);
		if (n < 0 || n > SIZE32_MAX) {
			if (!PyErr_Occurred()) {
				PyErr_SetString(PyExc_ValueError,
					"bad marshal data (set size out of range)");
			}
			break;
		}

		if (n == 0 && type == TYPE_FROZENSET) {
			/* call frozenset() to get the empty frozenset singleton */
			v = _PyObject_CallNoArgs((AlifObject*)&PyFrozenSet_Type);
			if (v == NULL)
				break;
			R_REF(v);
			retval = v;
		}
		else {
			v = (type == TYPE_SET) ? PySet_New(NULL) : PyFrozenSet_New(NULL);
			if (type == TYPE_SET) {
				R_REF(v);
			}
			else {
				/* must use delayed registration of frozensets because they must
				 * be init with a refcount of 1
				 */
				idx = r_ref_reserve(flag, p);
				if (idx < 0)
					Py_CLEAR(v); /* signal error */
			}
			if (v == NULL)
				break;

			for (i = 0; i < n; i++) {
				v2 = r_object(p);
				if (v2 == NULL) {
					if (!PyErr_Occurred())
						PyErr_SetString(PyExc_TypeError,
							"NULL object in marshal data for set");
					Py_SETREF(v, NULL);
					break;
				}
				if (PySet_Add(v, v2) == -1) {
					Py_DECREF(v);
					Py_DECREF(v2);
					v = NULL;
					break;
				}
				Py_DECREF(v2);
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

		if (!p->allow_code) {
			PyErr_SetString(PyExc_ValueError,
				"unmarshalling code objects is disallowed");
			break;
		}
		idx = r_ref_reserve(flag, p);
		if (idx < 0)
			break;

		v = NULL;

		/* XXX ignore long->int overflows for now */
		argcount = (int)r_long(p);
		if (argcount == -1 && PyErr_Occurred())
			goto code_error;
		posonlyargcount = (int)r_long(p);
		if (posonlyargcount == -1 && PyErr_Occurred()) {
			goto code_error;
		}
		kwonlyargcount = (int)r_long(p);
		if (kwonlyargcount == -1 && PyErr_Occurred())
			goto code_error;
		stacksize = (int)r_long(p);
		if (stacksize == -1 && PyErr_Occurred())
			goto code_error;
		flags = (int)r_long(p);
		if (flags == -1 && PyErr_Occurred())
			goto code_error;
		code = r_object(p);
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
		if (firstlineno == -1 && PyErr_Occurred())
			break;
		linetable = r_object(p);
		if (linetable == NULL)
			goto code_error;
		exceptiontable = r_object(p);
		if (exceptiontable == NULL)
			goto code_error;

		class AlifCodeConstructor con = {
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

			 exceptiontable,
		};

		if (_PyCode_Validate(&con) < 0) {
			goto code_error;
		}

		v = (AlifObject*)_PyCode_New(&con);
		if (v == NULL) {
			goto code_error;
		}

		v = r_ref_insert(v, idx, flag, p);

	code_error:
		if (v == NULL && !PyErr_Occurred()) {
			PyErr_SetString(PyExc_TypeError,
				"NULL object in marshal data for code object");
		}
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
		if (n < 0 || n >= PyList_GET_SIZE(p->refs)) {
			if (!PyErr_Occurred()) {
				PyErr_SetString(PyExc_ValueError,
					"bad marshal data (invalid reference)");
			}
			break;
		}
		v = PyList_GET_ITEM(p->refs, n);
		if (v == Py_None) {
			break;
		}
		retval = Py_NewRef(v);
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
	if (p->ptr && p->end) {
		//if (alifSys_audit("marshal.loads", "y#", p->ptr, (int64_t)(p->end - p->ptr)) < 0) {
			//return NULL;
		//}
	}
	else if (p->fp || p->readable) {
		//if (alifSys_audit("marshal.load", NULL) < 0) {
			//return NULL;
		//}
	}
	v = r_object(p);
	//if (v == NULL && !PyErr_Occurred())
		//PyErr_SetString(PyExc_TypeError, "NULL object in marshal data for object");
	return v;
}

AlifObject* alifMarshal_readObjectFromString(const wchar_t* str, int64_t len) // 1670
{
	Rfile rf;
	AlifObject* result;
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
