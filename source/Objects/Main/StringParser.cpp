#include "alif.h"

#include "AlifCore_BytesObject.h"
//#include "AlifCore_UStrObject.h"

#include "AlifTokenState.h"
#include "AlifParserEngine.h"
#include "StringParser.h"


AlifObject* alifParserEngine_decodeString(AlifParser* _p, AlifIntT _raw, const char* _s, AlifSizeT _len, AlifPToken* _t) {
	if (_raw) {
		return alifUStr_decodeUTF8Stateful(_s, _len, nullptr, nullptr);
	}
	//return decode_UStrWithEscape(_p, _s, _len, _t);
	return nullptr; // temp
}

AlifObject* alifParserEngine_parseString(AlifParser* _p, AlifPToken* _t) {
	const char* s = _alifWBytes_asString(_t->bytes);
	if (s == nullptr) return nullptr;

	AlifUSizeT len{};
	AlifIntT quote = ALIF_CHARMASK(*s);
	AlifIntT bytesMode{};
	AlifIntT rawMode{};

	//if (ALIF_ISALPHA(quote)) {
	//	while (!bytesMode or !rawMode) {
	//		// if 'b' or 'B'
	//	}
	//}

	if (quote != '\'' and quote != '\"') {
		// error
		return nullptr;
	}
	// skip the leading quote wchar_t
	s++;
	len = strlen(s);
	// error
	// error

	if (len >= 4 and s[0] == quote and s[1] == quote) {
		s += 2;
		len -= 2;
		// error
	}

	rawMode ? rawMode : rawMode = (strchr(s, '\\') == nullptr); // يجب مراجعتها لانه تم التعديل عليها
	/*
	.
	.
	*/

	return alifParserEngine_decodeString(_p, rawMode, s, len, _t);
}
