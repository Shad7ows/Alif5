
#include "alif.h"
#include "AlifCore_Memory.h"

static AlifObject* va_build_value(const wchar_t*, va_list);

static int64_t countFormat(const wchar_t* format, wchar_t endchar)
{
	int64_t count = 0;
	int level = 0;
	while (level > 0 || *format != endchar) {
		switch (*format) {
		case '\0':
			return -1;
		case '(':
		case '[':
		case '{':
			if (level == 0) {
				count++;
			}
			level++;
			break;
		case ')':
		case ']':
		case '}':
			level--;
			break;
		case '#':
		case '&':
		case ',':
		case ':':
		case ' ':
		case '\t':
			break;
		default:
			if (level == 0) {
				count++;
			}
		}
		format++;
	}
	return count;
}

static AlifObject* do_mktuple(const wchar_t**, va_list*, wchar_t, int64_t);
static int do_mkstack(AlifObject**, const wchar_t**, va_list*, wchar_t, int64_t);
static AlifObject* do_mklist(const wchar_t**, va_list*, wchar_t, int64_t);
static AlifObject* do_mkdict(const wchar_t**, va_list*, wchar_t, int64_t);
static AlifObject* do_mkvalue(const wchar_t**, va_list*);

static int check_end(const wchar_t** pFormat, wchar_t endchar)
{
	const wchar_t* f = *pFormat;
	while (*f != endchar) {
		if (*f != ' ' && *f != '\t' && *f != ',' && *f != ':') {

			return 0;
		}
		f++;
	}
	if (endchar) {
		f++;
	}
	*pFormat = f;
	return 1;
}

static void do_ignore(const wchar_t** pFormat, va_list* pVA, wchar_t endchar, int64_t n)
{
	AlifObject* v = alifNew_tuple(n);
	for (int64_t i = 0; i < n; i++) {
		AlifObject* w = do_mkvalue(pFormat, pVA);
		if (w != NULL) {
			if (v != NULL) {
				ALIFTUPLE_SET_ITEM(v, i, w);
			}
			else {
				ALIF_DECREF(w);
			}
		}
	}
	ALIF_XDECREF(v);
	if (!check_end(pFormat, endchar)) {
		return;
	}
}

static AlifObject* do_mkdict(const wchar_t** pFormat, va_list* pVA, wchar_t endchar, int64_t n)
{
	AlifObject* d;
	int64_t i;
	if (n < 0)
		return NULL;
	if (n % 2) {
		do_ignore(pFormat, pVA, endchar, n);
		return NULL;
	}

	if ((d = alifNew_dict()) == NULL) {
		do_ignore(pFormat, pVA, endchar, n);
		return NULL;
	}
	for (i = 0; i < n; i += 2) {
		AlifObject* k, * v;

		k = do_mkvalue(pFormat, pVA);
		if (k == NULL) {
			do_ignore(pFormat, pVA, endchar, n - i - 1);
			ALIF_DECREF(d);
			return NULL;
		}
		v = do_mkvalue(pFormat, pVA);
		if (v == NULL || alifDict_setItem(d, k, v) < 0) {
			do_ignore(pFormat, pVA, endchar, n - i - 2);
			ALIF_DECREF(k);
			ALIF_XDECREF(v);
			ALIF_DECREF(d);
			return NULL;
		}
		ALIF_DECREF(k);
		ALIF_DECREF(v);
	}
	if (!check_end(pFormat, endchar)) {
		ALIF_DECREF(d);
		return NULL;
	}
	return d;
}

static AlifObject* do_mklist(const wchar_t** pFormat, va_list* pVA, wchar_t endchar, int64_t n)
{
	AlifObject* v;
	int64_t i;
	if (n < 0)
		return NULL;

	v = alifNew_list(n);
	if (v == NULL) {
		do_ignore(pFormat, pVA, endchar, n);
		return NULL;
	}
	for (i = 0; i < n; i++) {
		AlifObject* w = do_mkvalue(pFormat, pVA);
		if (w == NULL) {
			do_ignore(pFormat, pVA, endchar, n - i - 1);
			ALIF_DECREF(v);
			return NULL;
		}
		ALIFLIST_SETITEM(v, i, w);
	}
	if (!check_end(pFormat, endchar)) {
		ALIF_DECREF(v);
		return NULL;
	}
	return v;
}


static int do_mkstack(AlifObject** stack, const wchar_t** pFormat, va_list* pVA,
	wchar_t endchar, int64_t n)
{
	int64_t i;

	if (n < 0) {
		return -1;
	}

	for (i = 0; i < n; i++) {
		AlifObject* w = do_mkvalue(pFormat, pVA);
		if (w == NULL) {
			do_ignore(pFormat, pVA, endchar, n - i - 1);
			goto error;
		}
		stack[i] = w;
	}
	if (!check_end(pFormat, endchar)) {
		goto error;
	}
	return 0;

error:
	n = i;
	for (i = 0; i < n; i++) {
		ALIF_DECREF(stack[i]);
	}
	return -1;
}

static AlifObject* do_mktuple(const wchar_t** pFormat, va_list* pVA, wchar_t endchar, int64_t n)
{
	AlifObject* v;
	int64_t i;
	if (n < 0)
		return NULL;

	if ((v = alifNew_tuple(n)) == NULL) {
		do_ignore(pFormat, pVA, endchar, n);
		return NULL;
	}
	for (i = 0; i < n; i++) {
		AlifObject* w = do_mkvalue(pFormat, pVA);
		if (w == NULL) {
			do_ignore(pFormat, pVA, endchar, n - i - 1);
			ALIF_DECREF(v);
			return NULL;
		}
		ALIFTUPLE_SET_ITEM(v, i, w);
	}
	if (!check_end(pFormat, endchar)) {
		ALIF_DECREF(v);
		return NULL;
	}
	return v;
}


static AlifObject* do_mkvalue(const wchar_t** pFormat, va_list* pVA)
{
	for (;;) {
		switch (*(*pFormat)++) {
		case '(':
			return do_mktuple(pFormat, pVA, ')',
				countFormat(*pFormat, ')'));

		case '[':
			return do_mklist(pFormat, pVA, ']',
				countFormat(*pFormat, ']'));

		case '{':
			return do_mkdict(pFormat, pVA, '}',
				countFormat(*pFormat, '}'));

		case 'b':
		case 'B':
		case 'h':
		case 'i':
			return alifInteger_fromLongLong((long)va_arg(*pVA, int));

		case 'H':
			return alifInteger_fromLongLong((long)va_arg(*pVA, unsigned int));

		case 'I':
		{
			unsigned int n;
			n = va_arg(*pVA, unsigned int);
			return alifInteger_fromLongLong(n);
		}

		case 'n':
#if SIZEOF_SIZE_T!=SIZEOF_LONG
			return PyLong_FromSsize_t(va_arg(*pVA, int64_t));
#endif
			ALIFFALLTHROUGH;
		case 'l':
			return alifInteger_fromLongLong(va_arg(*pVA, long));

		case 'k':
		{
			unsigned long n;
			n = va_arg(*pVA, unsigned long);
			return alifInteger_fromLongLong(n);
		}

		case 'L':
			return alifInteger_fromLongLong((long long)va_arg(*pVA, long long));

		case 'K':
			return alifInteger_fromLongLong((long long)va_arg(*pVA, unsigned long long));

		case 'u':
		{
			AlifObject* v;
			wchar_t* u = va_arg(*pVA, wchar_t*);
			int64_t n;
			if (**pFormat == '#') {
				++*pFormat;
				n = va_arg(*pVA, int64_t);
			}
			else
				n = -1;
			if (u == NULL) {
				v = ALIF_NEWREF(ALIF_NONE);
			}
			else {
				if (n < 0)
					n = wcslen(u);
				v = alifUStr_objFromWChar(u);
			}
			return v;
		}
		case 'f':
		case 'd':
			return alifFloat_fromDouble(
				(double)va_arg(*pVA, double));

		//case 'D':
			//return alifComplex_fromCComplex(
				//*((ALIF_complex*)va_arg(*pVA, ALIF_complex*))); // سيتم عمل هذا النوع لاحقا

		case 'c':
		{
			wchar_t p[1];
			p[0] = (char)va_arg(*pVA, int);
			return alifBytes_fromStringAndSize(p, 1);
		}
		case 'C':
		{
			//int i = va_arg(*pVA, int);
			//return alifUStr_fromOrdinal(i); // سيتم عمله لاحقا
		}

		case 's':
		case 'z':
		case 'U':   /* XXX deprecated alias */
		{
			AlifObject* v;
			const wchar_t* str = va_arg(*pVA, const wchar_t*);
			int64_t n;
			if (**pFormat == '#') {
				++*pFormat;
				n = va_arg(*pVA, int64_t);
			}
			else
				n = -1;
			if (str == NULL) {
				v = ALIF_NEWREF(ALIF_NONE);
			}
			else {
				if (n < 0) {
					size_t m = wcslen(str);
					if (m > LLONG_MAX) {

						return NULL;
					}
					n = (int64_t)m;
				}
				v = alifUStr_fromString(str);
			}
			return v;
		}

		case 'y':
		{
			AlifObject* v;
			const wchar_t* str = va_arg(*pVA, const wchar_t*);
			int64_t n;
			if (**pFormat == '#') {
				++*pFormat;
				n = va_arg(*pVA, int64_t);
			}
			else
				n = -1;
			if (str == NULL) {
				v = ALIF_NEWREF(ALIF_NONE);
			}
			else {
				if (n < 0) {
					size_t m = wcslen(str);
					if (m > LLONG_MAX) {
						return NULL;
					}
					n = (int64_t)m;
				}
				v = alifBytes_fromStringAndSize(str, n);
			}
			return v;
		}

		case 'N':
		case 'S':
		case 'O':
			if (**pFormat == '&') {
				typedef AlifObject* (*converter)(void*);
				converter func = va_arg(*pVA, converter);
				void* arg = va_arg(*pVA, void*);
				++*pFormat;
				return (*func)(arg);
			}
			else {
				AlifObject* v;
				v = va_arg(*pVA, AlifObject*);
				if (v != NULL) {
					if (*(*pFormat - 1) != 'N')
						ALIF_INCREF(v);
				}
				return v;
			}

		case ':':
		case ',':
		case ' ':
		case '\t':
			break;

		default:
			return NULL;

		}
	}
}

AlifObject* alif_buildValue(const wchar_t* format, ...)
{
	va_list va;
	AlifObject* retval;
	va_start(va, format);
	retval = va_build_value(format, va);
	va_end(va);
	return retval;
}

static AlifObject* va_build_value(const wchar_t* format, va_list va)
{
	const wchar_t* f = format;
	int64_t n = countFormat(f, '\0');
	va_list lva;
	AlifObject* retval;

	if (n < 0)
		return NULL;
	if (n == 0) {
		return ALIF_NONE;
	}
	va_copy(lva, va);
	if (n == 1) {
		retval = do_mkvalue(&f, &lva);
	}
	else {
		retval = do_mktuple(&f, &lva, '\0', n);
	}
	va_end(lva);
	return retval;
}

AlifObject** alif_vaBuildStack(AlifObject** smallStack, int64_t smallStackLen,
	const wchar_t* format, va_list va, int64_t* pNArgs)
{
	const wchar_t* f;
	int64_t n;
	va_list lva;
	AlifObject** stack;
	int res;

	n = countFormat(format, '\0');
	if (n < 0) {
		*pNArgs = 0;
		return NULL;
	}

	if (n == 0) {
		*pNArgs = 0;
		return smallStack;
	}

	if (n <= smallStackLen) {
		stack = smallStack;
	}
	else {
		stack = (AlifObject**)alifMem_objAlloc(n * sizeof(stack[0]));
		if (stack == NULL) {
			return NULL;
		}
	}

	va_copy(lva, va);
	f = format;
	res = do_mkstack(stack, &f, &lva, '\0', n);
	va_end(lva);

	if (res < 0) {
		if (stack != smallStack) {
			alifMem_objFree(stack);
		}
		return NULL;
	}

	*pNArgs = n;
	return stack;
}

