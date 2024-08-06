#include "alif.h"

#include "AlifCore_AlifToken.h"
#include "ErrorCode.h"
#include "AlifTokenState.h"
#include "Helpers.h"
#include "AlifCore_Memory.h"

// Alternate tab spacing
#define ALTTABSIZE 1

#define IS_IDENTIFIER_START(_ch) ((_ch >= 'a' and _ch <= 'z') \
								or (_ch >= 'A' and _ch <= 'Z') /* to exclude nums and symbols */ \
								or (_ch == '_') \
								or (_ch < '٠' and _ch >= 128) \
								or (_ch > '٩' and _ch >= 128)) /* exclude arabic-indic nums */

#define IS_IDENTIFIER_CHAR(_ch) ((_ch >= 'a' and _ch <= 'z') \
								or (_ch >= 'A' and _ch <= 'Z') \
								or (_ch >= '0' and _ch <= '9') \
								or (_ch == '_') \
								or (_ch >= 128))

#define TOK_GETMODE(tok) (&(_tokInfo->tokModeStack[_tokInfo->tokModeStackIndex]))

#define MAKE_TOKEN(TT) alifLexer_setupToken(_tokInfo, _token, TT, pStart, pEnd)


static void tok_backup(TokenInfo* _tokInfo, AlifIntT _wc) {
	if (_wc != EOF) {
		if (--_tokInfo->cur < _tokInfo->buf) {
			// error
		}
		if ((AlifIntT)(char)*_tokInfo->cur != ALIF_CHARMASK(_wc)) {
			// error
		}
		_tokInfo->colOffset--;
	}
}


static AlifIntT tok_nextChar(TokenInfo* _tokInfo) {
	AlifIntT rc{};
	for (;;) {
		if (_tokInfo->cur != _tokInfo->inp) {
			if ((AlifUIntT)_tokInfo->colOffset >= (AlifUIntT)INT_MAX) {
				//_tokInfo->done = E_COLOMN_OVERFLOW;
				return EOF;
			}
			_tokInfo->colOffset++;
			return ALIF_CHARMASK(*_tokInfo->cur++);
		}
		if (_tokInfo->done != E_OK) {
			return EOF;
		}

		rc = _tokInfo->underflow(_tokInfo);

		if (!rc) {
			_tokInfo->cur = _tokInfo->inp;
			return EOF;
		}
		_tokInfo->lineStart = _tokInfo->cur;

	}
}


static AlifIntT set_fStringExpr(TokenInfo* _tokInfo, AlifToken* _token, char _ch) {

	TokenizerMode* tokMode = TOK_GETMODE(_tokInfo);

	if (_token->data) return 0;

	AlifObject* res = nullptr;

	// Check if there is a # character in the expression
	AlifIntT hashDetected = 0;
	for (AlifUSizeT i = 0; i < tokMode->lastExprSize - tokMode->lastExprEnd; i++) {
		if (tokMode->lastExprBuff[i] == '#') {
			hashDetected = 1;
			break;
		}
	}

	if (hashDetected) {
		AlifUSizeT inputLength = tokMode->lastExprSize - tokMode->lastExprEnd;
		char* result = (char*)alifMem_dataAlloc((inputLength + 1));
		if (!result) return -1;

		AlifUSizeT i = 0;
		AlifUSizeT j = 0;

		for (i = 0, j = 0; i < inputLength; i++) {
			if (tokMode->lastExprBuff[i] == '#') {
				// Skip characters until newline or end of string
				while (tokMode->lastExprBuff[i] != '\0' && i < inputLength) {
					if (tokMode->lastExprBuff[i] == '\n') {
						result[j++] = tokMode->lastExprBuff[i];
						break;
					}
					i++;
				}
			}
			else {
				result[j++] = tokMode->lastExprBuff[i];
			}
		}

		result[j] = '\0';  // Null-terminate the result string
		res = alifUStr_decodeUTF8(result, j, nullptr);
		alifMem_dataFree(result);
	}
	else {
		res = alifUStr_decodeUTF8(tokMode->lastExprBuff, tokMode->lastExprSize - tokMode->lastExprEnd, nullptr);
	}

	if (!res) return -1;
	_token->data = res;
	return 0;
}


AlifIntT alifLexer_updateFStringExpr(TokenInfo* _tokInfo, wchar_t _cur) {

	AlifUSizeT size = strlen(_tokInfo->cur);
	TokenizerMode* tokMode = TOK_GETMODE(_tokInfo);
	char* newBuffer{};

	switch (_cur) {

	case 0:
		if (!tokMode->lastExprBuff or tokMode->lastExprEnd >= 0) {
			return 1;
		}
		newBuffer = (char*)alifMem_dataRealloc(tokMode->lastExprBuff, tokMode->lastExprSize + size);

		if (newBuffer == nullptr) {
			alifMem_dataFree(tokMode->lastExprBuff);
			goto error;
		}
		tokMode->lastExprBuff = newBuffer;
		strncpy(tokMode->lastExprBuff + tokMode->lastExprSize, _tokInfo->cur, size);
		tokMode->lastExprSize += size;
		break;
	case '{':
		if (tokMode->lastExprBuff != nullptr) {
			alifMem_dataFree(tokMode->lastExprBuff);
		}
		tokMode->lastExprBuff = (char*)alifMem_dataAlloc(size);
		if (tokMode->lastExprBuff == nullptr) {
			goto error;
		}
		tokMode->lastExprSize = size;
		tokMode->lastExprEnd = -1;
		strncpy(tokMode->lastExprBuff, _tokInfo->cur, size);
		break;
	case '}':
	case '!':
	case ':':
		if (tokMode->lastExprEnd == -1) {
			tokMode->lastExprEnd = strlen(_tokInfo->start);
		}
		break;
	default:
		//ALIF_UNREACHABLE();
		break;
	}

	return 1;

error:
	_tokInfo->done = E_NOMEM;
	return 0;
}


static AlifIntT tok_decimalTail(TokenInfo* _tokInfo) {
	AlifIntT ch_{};

	while (1) {
		do {
			ch_ = tok_nextChar(_tokInfo);
		} while (ALIF_ISDIGIT(ch_));
		if (ch_ != '_') {
			break;
		}
		ch_ = tok_nextChar(_tokInfo);
		if (!ALIF_ISDIGIT(ch_)) {
			tok_backup(_tokInfo, ch_);
			// error
			return 0;
		}
	}
	return ch_;
}

static inline AlifIntT tok_continuationLine(TokenInfo* _tokInfo) {

	AlifIntT ch_ = tok_nextChar(_tokInfo);
	if (ch_ == '\r') ch_ = tok_nextChar(_tokInfo);
	if (ch_ != '\n') { _tokInfo->done = E_LINECONT; return -1; }

	ch_ = tok_nextChar(_tokInfo);
	if (ch_ == EOF) {
		_tokInfo->done = E_EOF;
		_tokInfo->cur = _tokInfo->inp;
		return -1;
	}
	else {
		tok_backup(_tokInfo, ch_);
	}

	return ch_;
}

static AlifIntT tokGet_normalMode(TokenInfo* _tokInfo, TokenizerMode* _currentTok, AlifToken* _token) {
	AlifIntT ch_{};
	AlifIntT blankLine{};

	const char* pStart{};
	const char* pEnd{};

nextline:
	_tokInfo->start = nullptr;
	_tokInfo->startingColOffset = -1;
	blankLine = 0;

	// indentation level
	if (_tokInfo->atBeginOfLine) {
		AlifIntT col = 0;
		AlifIntT altCol = 0;
		_tokInfo->atBeginOfLine = 0;
		AlifIntT contLineCol = 0;

		for (;;) {
			ch_ = tok_nextChar(_tokInfo);
			if (ch_ == ' ') { col++; altCol++; }
			else if (ch_ == '\t')
			{
				col = (col / _tokInfo->tabSize + 1) * _tokInfo->tabSize;
				altCol = (altCol / ALTTABSIZE + 1) * ALTTABSIZE;
			}
			else if (ch_ == '\\') {
				contLineCol = contLineCol ? contLineCol : col;
				if ((ch_ = tok_continuationLine(_tokInfo)) == -1) return MAKE_TOKEN(ERRORTOKEN);
			}
			else break;
		}
		tok_backup(_tokInfo, ch_);
		if (ch_ == '#' or ch_ == '\n' or ch_ == '\r') {
			if (col == 0 and ch_ == '\n' and _tokInfo->prompt != nullptr) blankLine = 0;
			else if (_tokInfo->prompt != nullptr and _tokInfo->lineNo == 1) {
				blankLine = 0;
				col = altCol = 0;
			}
			else { blankLine = 1; }
		}
		if (!blankLine and _tokInfo->level == 0) {
			col = contLineCol ? contLineCol : col;
			altCol = contLineCol ? contLineCol : altCol;
			if (col == _tokInfo->indStack[_tokInfo->indent])
			{
				if (altCol != _tokInfo->alterIndStack[_tokInfo->indent]) {
					//return MAKE_TOKEN(alifTokenizer_indentError(_tokInfo)); // indent error
				}
			}
			else if (col > _tokInfo->indStack[_tokInfo->indent]) {
				// indent - always one
				if (_tokInfo->indent + 1 >= MAXINDENT) {
					_tokInfo->done = E_TOODEEP;
					_tokInfo->cur = _tokInfo->inp;
					return MAKE_TOKEN(ERRORTOKEN);
				}
				if (altCol <= _tokInfo->alterIndStack[_tokInfo->indent]) {
					//return MAKE_TOKEN(alifTokenizer_indentError(_tokInfo)); // indent error
				}
				_tokInfo->pendInd++;
				_tokInfo->indStack[++_tokInfo->indent] = col;
				_tokInfo->alterIndStack[++_tokInfo->indent] = altCol;
			}
			else {
				/* col < tok->indstack[tok->indent] */
				/* Dedent -- any number, must be consistent */

				while (_tokInfo->indent > 0 and col < _tokInfo->indStack[_tokInfo->indent]) {
					_tokInfo->pendInd--;
					_tokInfo->indent--;
				}
				if (col != _tokInfo->indStack[_tokInfo->indent]) {
					_tokInfo->done = E_DEDENT;
					_tokInfo->cur = _tokInfo->inp;
					return MAKE_TOKEN(ERRORTOKEN);
				}
				if (altCol != _tokInfo->alterIndStack[_tokInfo->indent]) {
					//return MAKE_TOKEN(alifTokenizer_indentError(_tokInfo)); // indent error
				}
			}
		}
	}

	_tokInfo->start = _tokInfo->cur;
	_tokInfo->startingColOffset = _tokInfo->colOffset;

	// return pending indents/dedents
	if (_tokInfo->pendInd != 0) {
		if (_tokInfo->pendInd < 0) {
			if (_tokInfo->tokExtraTokens) {
				pStart = _tokInfo->cur;
				pEnd = _tokInfo->cur;
			}
			_tokInfo->pendInd++;
			return MAKE_TOKEN(DEDENT);
		}
		else {
			if (_tokInfo->tokExtraTokens) {
				pStart = _tokInfo->buf;
				pEnd = _tokInfo->cur;
			}
			_tokInfo->pendInd--;
			return MAKE_TOKEN(INDENT);
		}
	}


	ch_ = tok_nextChar(_tokInfo);
	tok_backup(_tokInfo, ch_);

again:
	_tokInfo->start = nullptr;

	// skip spaces
	do {
		ch_ = tok_nextChar(_tokInfo);
	} while (ch_ == ' ' or ch_ == '\t' or ch_ == '\014');

	// Set start of current token
	_tokInfo->start = _tokInfo->cur == nullptr ? nullptr : _tokInfo->cur - 1;
	_tokInfo->startingColOffset = _tokInfo->colOffset - 1;

	// Skip comment
	if (ch_ == '#') {
		const char* p{};
		AlifIntT currentStartingColOffset{};

		while (ch_ != EOF and ch_ != '\n' and ch_ != '\r') {
			ch_ = tok_nextChar(_tokInfo);
		}

		if (_tokInfo->tokExtraTokens) { p = _tokInfo->start; }

		if (_tokInfo->tokExtraTokens) {
			tok_backup(_tokInfo, ch_);
			pStart = p;
			pEnd = _tokInfo->cur;
			_tokInfo->commentNewline = blankLine;
			return MAKE_TOKEN(COMMENT);
		}
	}

	if (_tokInfo->done == E_INTERACT_STOP) {
		return MAKE_TOKEN(ENDMARKER);
	}

	// check for EOF and errors now
	if (ch_ == EOF) {
		if (_tokInfo->level) {
			return MAKE_TOKEN(ERRORTOKEN);
		}
		return MAKE_TOKEN(_tokInfo->done == E_EOF ? ENDMARKER : ERRORTOKEN);
	}


	/* Identifire */
	if (IS_IDENTIFIER_START(ch_)) {
		AlifIntT b_ = 0, r_ = 0, u_ = 0, f_ = 0;
		while (true) {
			if (!(b_ or u_ or f_) and ch_ == 'ب') b_ = 1; // ب = بايت
			else if (!(b_ or u_ or r_) and ch_ == 'ت') u_ = 1; // ت = ترميز
			else if (!(r_ or u_) and ch_ == 'خ') r_ = 1; // خ = خام
			else if (!(f_ or b_ or u_) and ch_ == 'م') f_ = 1; // م = منسق
			else break;

			ch_ = tok_nextChar(_tokInfo);
			if (ch_ == '"' or ch_ == '\'') {
				if (f_) goto fStringQuote;
				goto letterQuote;
			}
		}
		while (IS_IDENTIFIER_CHAR(ch_)) { // يجب مراجعة وظيفة هذا التحقق
			ch_ = tok_nextChar(_tokInfo);
		}
		tok_backup(_tokInfo, ch_);

		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;

		return MAKE_TOKEN(NAME);
	}

	if (ch_ == '\r') { ch_ = tok_nextChar(_tokInfo); }

	/* Newline */
	if (ch_ == '\n') {
		_tokInfo->atBeginOfLine = 1;
		if (blankLine or _tokInfo->level > 0) {
			if (_tokInfo->tokExtraTokens) {
				if (_tokInfo->commentNewline) {
					_tokInfo->commentNewline = 0;
				}
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
				return MAKE_TOKEN(NL);
			}
			goto nextline;
		}
		if (_tokInfo->commentNewline and _tokInfo->tokExtraTokens) {
			_tokInfo->commentNewline = 0;
			pStart = _tokInfo->start;
			pEnd = _tokInfo->cur;
			return MAKE_TOKEN(NL);
		}
		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur - 1;
		_tokInfo->countLine = 0; // Leave '\n' out of the string

		return MAKE_TOKEN(NEWLINE);
	}

	/* Period or number starting with period? */
	if (ch_ == '.') {

		ch_ = tok_nextChar(_tokInfo);
		if (ALIF_ISDIGIT(ch_)) {
			goto fraction;
		}

		tok_backup(_tokInfo, ch_);
		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;

		return MAKE_TOKEN(DOT);
	}


	/* Number */
	if (ALIF_ISDIGIT(ch_)) {
		if (ch_ == '0') {
			/* Hex or Octal or Binary */
			ch_ = tok_nextChar(_tokInfo);
			if (ch_ == 'ه') {
				ch_ = tok_nextChar(_tokInfo);
				do {
					if (ch_ == '_') {
						ch_ = tok_nextChar(_tokInfo);
					}
					if (!ALIF_ISXDIGIT(ch_)) {
						tok_backup(_tokInfo, ch_);
						//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ستعشري غير صحيح"));
					}
					do {
						ch_ = tok_nextChar(_tokInfo);
					} while (ALIF_ISXDIGIT(ch_));
				} while (ch_ == '_');
				//if (!verify_endOfNumber(_tokInfo, wcs, L"ستعشري")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
			else if (ch_ == 'ث') {
				// Octal
				ch_ = tok_nextChar(_tokInfo);
				do {
					if (ch_ == '_') { ch_ = tok_nextChar(_tokInfo); }
					if (ch_ < '0' or ch_ >= '8') {
						if (ALIF_ISDIGIT(ch_)) {
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثماني غير صحيح '%wcs'", wcs));
						}
						else {
							tok_backup(_tokInfo, ch_);
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثماني غير صحيح"));
						}
					}
					do {
						ch_ = tok_nextChar(_tokInfo);
					} while ('0' <= ch_ and ch_ < '8');
				} while (ch_ == '-');
				if (ALIF_ISDIGIT(ch_)) {
					//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثماني غير صحيح '%wcs'", wcs));
				}
				//if (!verify_endOfNumber(_tokInfo, wcs, L"ثماني")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
			else if (ch_ == 'ن') {
				// Binary
				ch_ = tok_nextChar(_tokInfo);
				do {
					if (ch_ == '_') { ch_ = tok_nextChar(_tokInfo); }

					if (ch_ != '0' and ch_ != '1') {
						if (ALIF_ISDIGIT(ch_)) {
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثنائي غير صحيح '%wcs'", wcs));
						}
						else {
							tok_nextChar(_tokInfo);
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثنائي غير صحيح"));
						}
					}
					do {
						ch_ = tok_nextChar(_tokInfo);
					} while (ch_ == '0' or ch_ == '1');
				} while (ch_ == '_');

				if (ALIF_ISDIGIT(ch_)) {
					//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثنائي غير صحيح '%wcs'", wcs));
				}
				//if (!verify_endOfNumber(_tokInfo, wcs, L"ثنائي")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
			else {
				/*
					يجب التأكد من هذه الجزئية وطريقة عملها
				*/


			}
		}
		else {
			/* Decimal */
			ch_ = tok_decimalTail(_tokInfo);
			if (ch_ == 0) return MAKE_TOKEN(ERRORTOKEN);
			{
				/* float number */
				if (ch_ == '.') {
					ch_ = tok_nextChar(_tokInfo);
				fraction:
					// Fraction
					if (ALIF_ISDIGIT(ch_)) {
						ch_ = tok_decimalTail(_tokInfo);
						if (ch_ == 0) return MAKE_TOKEN(ERRORTOKEN);
					}
				}
				if (ch_ == 'س') { /* exponent */
					AlifIntT e{};
				exponent:
					e = ch_;
					ch_ = tok_nextChar(_tokInfo);
					if (ch_ == '+' or ch_ == '-') {
						ch_ = tok_nextChar(_tokInfo);
						if (!ALIF_ISDIGIT(ch_)) {
							tok_backup(_tokInfo, ch_);
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم عشري غير صحيح"));
						}
					}
					else if (!ALIF_ISDIGIT(ch_)) {
						tok_backup(_tokInfo, ch_);
						//if (!verify_endOfNumber(_tokInfo, e, L"عشري")) {
						//	return MAKE_TOKEN(ERRORTOKEN);
						//}
						tok_backup(_tokInfo, e);
						pStart = _tokInfo->start;
						pEnd = _tokInfo->cur;
						return MAKE_TOKEN(NUMBER);
					}
					ch_ = tok_decimalTail(_tokInfo);
					if (ch_ == 0) return MAKE_TOKEN(ERRORTOKEN);
				}
				if (ch_ == 'ت') {
					/* Imaginary part */
				imaginary:
					ch_ = tok_nextChar(_tokInfo);
					//if (!verify_endOfNumber(_tokInfo, c, L"تخيلي")) {
					//	return MAKE_TOKEN(ERRORTOKEN);
					//}
				}
				//else if (!verify_endOfNumber(_tokInfo, c, L"عشري")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
		}

		tok_backup(_tokInfo, ch_);
		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;
		return MAKE_TOKEN(NUMBER);
	}

fStringQuote:
	if ((ALIF_TOLOWER(*_tokInfo->start) == 'م' or ALIF_TOLOWER(*_tokInfo->start) == 'خ')
		and (ch_ == '\'' or ch_ == '"'))
	{
		AlifIntT quote = ch_;
		AlifIntT quoteSize = 1;

		_tokInfo->firstLineNo = _tokInfo->lineNo;
		_tokInfo->multiLineStart = _tokInfo->lineStart;

		AlifIntT afterQuote = tok_nextChar(_tokInfo);
		if (afterQuote == quote) {
			AlifIntT afterAfterQuote = tok_nextChar(_tokInfo);
			if (afterAfterQuote == quote) {
				quoteSize = 3;
			}
			else {
				tok_backup(_tokInfo, afterAfterQuote);
				tok_backup(_tokInfo, afterQuote);
			}
		}
		if (afterQuote != quote) {
			tok_backup(_tokInfo, afterQuote);
		}

		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;
		if (_tokInfo->tokModeStackIndex + 1 >= MAXFSTRING_LEVEL) {
			//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"لقد تجاوزت الحد الاقصى للنص المنسق المتداخل"));
		}
		TokenizerMode* currentTok = TOK_GETMODE(_tokInfo);
		currentTok->type = Token_FStringMode;
		currentTok->fStringQuote = quote;
		currentTok->fStringQuoteSize = quoteSize;
		currentTok->fStringStart = _tokInfo->start;
		currentTok->fStringMultiLineStart = _tokInfo->lineStart;
		currentTok->fStringLineStart = _tokInfo->lineNo;
		currentTok->fStringStartOffset = -1;
		currentTok->fStringMultiLineStartOffset = -1;
		currentTok->lastExprBuff = nullptr;
		currentTok->lastExprSize = 0;
		currentTok->lastExprEnd = -1;

		if (*_tokInfo->start == 'م') {
			currentTok->fStringRaw = ALIF_TOLOWER(*(_tokInfo->start + 1)) == 'خ';
		}
		else if (*_tokInfo->start == 'خ') {
			currentTok->fStringRaw = 1;
		}
		else {
			//ALIF_UNREACHABLE();
		}

		currentTok->curlyBracDepth = 0;
		currentTok->curlyBracExprStartDepth = -1;
		return MAKE_TOKEN(FSTRINGSTART);
	}

letterQuote:
	if (ch_ == '\'' or ch_ == '"') {
		AlifIntT quote = ch_;
		AlifIntT quoteSize = 1;
		AlifIntT endQuoteSize = 0;
		AlifIntT hasEscapedQuote = 0;

		_tokInfo->firstLineNo = _tokInfo->lineNo;
		_tokInfo->multiLineStart = _tokInfo->lineStart;

		// find the quote size and start of string
		ch_ = tok_nextChar(_tokInfo);
		if (ch_ == quote) {
			ch_ = tok_nextChar(_tokInfo);
			if (ch_ == quote) {
				quoteSize = 3;
			}
			else {
				endQuoteSize = 1; // اي انه نص فارغ
			}
		}
		if (ch_ != quote) {
			tok_backup(_tokInfo, ch_);
		}

		/* the rest of STRING */
		while (endQuoteSize != quoteSize) {
			ch_ = tok_nextChar(_tokInfo);
			if (_tokInfo->done == E_ERROR) {
				return MAKE_TOKEN(ERRORTOKEN);
			}
			if (_tokInfo->done == E_DECODE) break;

			if (ch_ == EOF or (quoteSize == 1 and ch_ == '\n')) {
				_tokInfo->cur = (char*)_tokInfo->start;
				_tokInfo->cur++;
				_tokInfo->lineStart = _tokInfo->multiLineStart;
				AlifIntT start = _tokInfo->lineNo;
				_tokInfo->lineNo = _tokInfo->firstLineNo;

				if (INSIDE_FSTRING(_tokInfo)) {
					TokenizerMode* currentToken = TOK_GETMODE(_tokInfo);
					if (currentToken->fStringQuote == quote and currentToken->fStringQuoteSize == quoteSize) {
						//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"خطأ في النص المنسق '{'", start));
					}
				}

				if (quoteSize == 3) {
					//alifTokenizer_syntaxError(_tokInfo, L"نص متعدد الاسطر لم يتم إنهاؤه" " في السطر %d", start);
					if (ch_ != '\n') {
						_tokInfo->done = E_EOFS;
					}
					return MAKE_TOKEN(ERRORTOKEN);
				}
				else {
					if (hasEscapedQuote) {
						//alifTokenizer_syntaxError(_tokInfo, L"نص لم يتم إنهاؤه" " في السطر %d", start);
					}
					else {
						//alifTokenizer_syntaxError(_tokInfo, L"نص لم يتم إنهاؤه" " في السطر %d", start);
					}
					if (ch_ != '\n') {
						_tokInfo->done = E_EOLS;
					}
					return MAKE_TOKEN(ERRORTOKEN);
				}
			}
			if (ch_ == quote) {
				endQuoteSize++;
			}
			else {
				endQuoteSize = 0;
				if (ch_ == '\\') {
					ch_ = tok_nextChar(_tokInfo);  // skip escape char
					if (ch_ == quote) {	           // record if the escape was a quote
						hasEscapedQuote = 1;
					}
					if (ch_ == '\r') {
						ch_ = tok_nextChar(_tokInfo);
					}
				}
			}
		}

		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;
		return MAKE_TOKEN(STRING);
	}

	/* line continuation */
	if (ch_ == '\\') {
		if ((ch_ = tok_continuationLine(_tokInfo)) == -1) {
			return MAKE_TOKEN(ERRORTOKEN);
		}
		_tokInfo->countLine = 1;
		goto again; // read next line
	}


	// Punctuation character
	AlifIntT isPunctuation = (ch_ == ':' or ch_ == '}' or ch_ == '!' or ch_ == '}');
	if (isPunctuation and INSIDE_FSTRING(_tokInfo) and INSIDE_FSTRING_EXPR(_currentTok)) {
		AlifIntT cursor = _currentTok->curlyBracDepth - (ch_ != '{');
		if (cursor == 0 and !alifLexer_updateFStringExpr(_tokInfo, ch_)) {
			return MAKE_TOKEN(ENDMARKER);
		}
		if (cursor == 0 and ch_ != '{' and set_fStringExpr(_tokInfo, _token, ch_)) {
			return MAKE_TOKEN(ERRORTOKEN);
		}

		if (ch_ == ':' and cursor == _currentTok->curlyBracExprStartDepth) {
			_currentTok->type = Token_FStringMode;
			pStart = _tokInfo->start;
			pEnd = _tokInfo->cur;
			return MAKE_TOKEN(alifToken_oneChar(ch_));
		}
	}


	{ // two_character token
		AlifIntT wcs2 = tok_nextChar(_tokInfo);
		AlifIntT currToken = alifToken_twoChars(ch_, wcs2);
		if (currToken != OP) {
			AlifIntT wcs3 = tok_nextChar(_tokInfo);
			AlifIntT currentTok3 = alifToken_threeChars(ch_, wcs2, wcs3);
			if (currentTok3 != OP) {
				currToken = currentTok3;
			}
			else {
				tok_backup(_tokInfo, wcs3);
			}
			pStart = _tokInfo->start;
			pEnd = _tokInfo->cur;
		}
		tok_backup(_tokInfo, wcs2);
	}

	// keep track of parentheses nesting level
	switch (ch_) {
	case '(':
	case '[':
	case '{':
		if (_tokInfo->level >= MAXLEVEL) {
			//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"تم تجاوز الحد الاقصى لتداخل الاقواس"));
		}
		_tokInfo->parenStack[_tokInfo->level] = ch_;
		_tokInfo->parenLineNoStack[_tokInfo->level] = _tokInfo->lineNo;
		_tokInfo->parenLineNoStack[_tokInfo->level] = (AlifIntT)(_tokInfo->start - _tokInfo->lineStart);
		_tokInfo->level++;
		if (INSIDE_FSTRING(_tokInfo)) {
			_currentTok->curlyBracDepth++;
		}
		break;
	case ')':
	case ']':
	case '}':
		// code here


		if (_tokInfo->level > 0) {
			_tokInfo->level--;
			AlifIntT opening = _tokInfo->parenStack[_tokInfo->level];
			if (!_tokInfo->tokExtraTokens
				and
				((opening == '(' and ch_ == ')')
				or
				(opening == '[' and ch_ == ']')
				or
				(opening == '{' and ch_ == '}')))
			{
				// code here
			}
		}

		if (INSIDE_FSTRING(_tokInfo)) {
			_currentTok->curlyBracDepth--;
			if (ch_ == '}' and _currentTok->curlyBracDepth == _currentTok->curlyBracExprStartDepth) {
				_currentTok->curlyBracExprStartDepth--;
				_currentTok->type = Token_FStringMode;
			}
		}
		break;
	default:
		break;
	}

	//if (!ALIF_UNICODE_ISPRINTABLE(wcs)) {
	//	return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"حرف غير قابل للطباعة", wcs));
	//}

	// punctuation character
	pStart = _tokInfo->start;
	pEnd = _tokInfo->cur;
	return MAKE_TOKEN(alifToken_oneChar(ch_));
}


static AlifIntT tokGet_fStringMode(TokenInfo* _tokInfo, TokenizerMode* _currentTok, AlifToken* _token) {
	const char* pStart{};
	const char* pEnd{};
	AlifIntT endQuoteSize = 0;
	AlifIntT unicodeEscape = 0;

	_tokInfo->start = _tokInfo->cur;
	_tokInfo->firstLineNo = _tokInfo->lineNo;
	_tokInfo->startingColOffset = _tokInfo->colOffset;

	// If we start with a bracket, we defer to the normal mode as there is nothing for us to tokenize
	// before it.
	AlifIntT startChar = tok_nextChar(_tokInfo);
	if (startChar == '{') {
		AlifIntT peek1 = tok_nextChar(_tokInfo);
		tok_backup(_tokInfo, peek1);
		tok_backup(_tokInfo, startChar);
		if (peek1 != '{') {
			_currentTok->curlyBracExprStartDepth++;
			if (_currentTok->curlyBracExprStartDepth >= MAX_EXPR_NESTING) {
				//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"نص منسق:التعبير المتداخل للنص المنسق وصل الحد الاقصى لعدد التداخلات"));
			}
			TOK_GETMODE(_tokinfo)->type = Token_RegularMode;
			return tokGet_normalMode(_tokInfo, _currentTok, _token);
		}
	}
	else {
		tok_backup(_tokInfo, startChar);
	}

	// Check if we are at the end of the string
	for (AlifIntT i = 0; i < _currentTok->fStringQuoteSize; i++) {
		AlifIntT quote = tok_nextChar(_tokInfo);
		if (quote != _currentTok->fStringQuote) {
			tok_backup(_tokInfo, quote);
			goto fStringMiddle;
		}
	}

	if (_currentTok->lastExprBuff != nullptr) {
		alifMem_dataFree(_currentTok->lastExprBuff);
		_currentTok->lastExprBuff = nullptr;
		_currentTok->lastExprSize = 0;
		_currentTok->lastExprEnd = -1;
	}

	pStart = _tokInfo->start;
	pEnd = _tokInfo->cur;
	_tokInfo->tokModeStackIndex--;
	return MAKE_TOKEN(FSTRINGEND);

fStringMiddle:

	// TODO: This is a bit of a hack, but it works for now. We need to find a better way to handle
	// this.
	_tokInfo->multiLineStart = _tokInfo->lineStart;
	while (endQuoteSize != _currentTok->fStringQuoteSize) {
		AlifIntT ch_ = tok_nextChar(_tokInfo);
		if (_tokInfo->done == E_ERROR) return MAKE_TOKEN(ERRORTOKEN);

		AlifIntT inFormatSpec = (_currentTok->lastExprEnd != -1 and INSIDE_FSTRING_EXPR(_currentTok));

		if (ch_ == EOF or (_currentTok->fStringQuoteSize == 1 and ch_ == '\n')) {

			// If we are in a format spec and we found a newline,
			// it means that the format spec ends here and we should
			// return to the regular mode.
			if (inFormatSpec and ch_ == '\n') {
				tok_backup(_tokInfo, ch_);
				TOK_GETMODE(_tokInfo)->type = Token_RegularMode;
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
				return MAKE_TOKEN(FSTRINGMIDDLE);
			}

			// shift the tok_state's location into
			// the start of string, and report the error
			// from the initial quote character
			_tokInfo->cur = (char*)_currentTok->fStringStart;
			_tokInfo->cur++;
			_tokInfo->lineStart = _currentTok->fStringMultiLineStart;
			AlifIntT start = _tokInfo->lineNo;

			TokenizerMode* currentTok = TOK_GETMODE(_tokInfo);
			_tokInfo->lineNo = currentTok->fStringLineStart;

			if (currentTok->fStringQuoteSize == 3) {
				//alifTokenizer_syntaxError(_tokInfo, L"نص متعدد الاسطر غير منتهي" L" (السطر %d)", start);
				if (ch_ != '\n') {
					_tokInfo->done = E_EOFS;
				}
				return MAKE_TOKEN(ERRORTOKEN);
			}
			else {
				//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"نص منسق غير منتهي" L" السطر %d)", start));
			}
		}

		if (ch_ == _currentTok->fStringQuote) {
			endQuoteSize++;
			continue;
		}
		else {
			endQuoteSize = 0;
		}

		if (ch_ == '{') {
			AlifIntT peek = tok_nextChar(_tokInfo);
			if (peek != '{' or inFormatSpec) {
				tok_backup(_tokInfo, peek);
				tok_backup(_tokInfo, ch_);
				_currentTok->curlyBracExprStartDepth++;
				if (_currentTok->curlyBracExprStartDepth >= MAX_EXPR_NESTING) {
					//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, "نص منسق:التعبير المتداخل للنص المنسق وصل الحد الاقصى لعدد التداخلات"));
				}
				TOK_GETMODE(_tokInfo)->type = Token_RegularMode;
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
			}
			else {
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur - 1;
			}
			return MAKE_TOKEN(FSTRINGMIDDLE);
		}
		else if (ch_ == '}') {
			if (unicodeEscape) {
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
				return MAKE_TOKEN(FSTRINGMIDDLE);
			}
			AlifIntT peek = tok_nextChar(_tokInfo);

			// The tokenizer can only be in the format spec if we have already completed the expression
			// scanning (indicated by the end of the expression being set) and we are not at the top level
			// of the bracket stack (-1 is the top level). Since format specifiers can't legally use double
			// brackets, we can bypass it here.
			if (peek == '}' and !inFormatSpec) {
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur - 1;
			}
			else {
				tok_backup(_tokInfo, peek);
				tok_backup(_tokInfo, ch_);
				TOK_GETMODE(_tokInfo)->type = Token_RegularMode;
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
			}
			return MAKE_TOKEN(FSTRINGMIDDLE);
		}
		else if (ch_ == '\\') {
			AlifIntT peek = tok_nextChar(_tokInfo);
			if (peek == '\r') {
				peek = tok_nextChar(_tokInfo);
			}
			// Special case when the backslash is right before a curly
			// brace. We have to restore and return the control back
			// to the loop for the next iteration.
			if (peek == '{' or peek == '}') {
				if (!_currentTok->fStringRaw) {
					//if (alifTokenizer_warnInvalidEscapeSequence(_tokInfo, peek)) {
					//	return MAKE_TOKEN(ERRORTOKEN);
					//}
				}
				tok_backup(_tokInfo, peek);
				continue;
			}

			if (!_currentTok->fStringRaw) {
				if (peek == 'N') {
					/* Handle named unicode escapes (\N{BULLET}) */
					peek = tok_nextChar(_tokInfo);
					if (peek == '{') {
						unicodeEscape = 1;
					}
					else {
						tok_backup(_tokInfo, peek);
					}
				}
			} /* else {
				skip the escaped character
			}*/
		}
	}

	// Backup the f-string quotes to emit a final FSTRINGMIDDLE and
	// add the quotes to the FSTRINGEND in the next tokenizer iteration.
	for (AlifIntT i = 0; i < _currentTok->fStringQuoteSize; i++) {
		tok_backup(_tokInfo, _currentTok->fStringQuote);
	}
	pStart = _tokInfo->start;
	pEnd = _tokInfo->cur;
	return MAKE_TOKEN(FSTRINGMIDDLE);
}


static AlifIntT token_get(TokenInfo* _tokInfo, AlifToken* _token) {
	TokenizerMode* currentTok = TOK_GETMODE(_tokInfo);
	if (currentTok->type == Token_RegularMode) {
		return tokGet_normalMode(_tokInfo, currentTok, _token);
	}
	else {
		return tokGet_fStringMode(_tokInfo, currentTok, _token);
	}
}

AlifIntT alifTokenizer_get(TokenInfo* _tokInfo, AlifToken* _token) {
	AlifIntT result = token_get(_tokInfo, _token);
	return result;
}
