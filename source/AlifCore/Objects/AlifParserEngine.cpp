#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_State.h"
#include "ErrorCode.h"

#include "AlifLexer.h"
#include "Tokenizer.h"
#include "AlifParserEngine.h"
#include "AlifCore_Memory.h"



//int alifParserEngine_insertMemo(AlifParser* _p, int _mark, int _type, void* _node) { 
//	Memo* m = (Memo*)alifASTMem_malloc(_p->astMem, sizeof(Memo));
//	if (m == nullptr) return -1;
//	m->type = _type;
//	m->node = _node;
//	m->mark_ = _p->mark_;
//	m->next = _p->tokens[_mark]->memo;
//	_p->tokens[_mark]->memo = m;
//	return 0;
//}
//
//int alifParserEngine_updateMemo(AlifParser* _p, int _mark, int _type, void* _node) { 
//	for (Memo* m = _p->tokens[_mark]->memo; m != nullptr; m = m->next) {
//		if (m->type == _type) {
//			m->node = _node;
//			m->mark_ = _p->mark_;
//			return 0;
//		}
//	}
//	return alifParserEngine_insertMemo(_p, _mark, _type, _node);
//}
//
//static int get_keywordOrName(AlifParser* _p, AlifToken* _token) { 
//	int nameLen = _token->endColOffset - _token->colOffset;
//
//	if (nameLen >= _p->nKeywordList or _p->keywords[nameLen] == nullptr
//		or _p->keywords[nameLen]->type == -1) return NAME;
//	for (KeywordToken* k = _p->keywords[nameLen]; k != nullptr and k->type != -1; k++) {
//		if (wcsncmp(k->str, _token->start, nameLen) == 0) {
//			return k->type;
//		}
//	}
//
//	return NAME;
//}
//
//
//static int initialize_token(AlifParser* _p, AlifPToken* _pToken, AlifToken* _token, int _tokType) { 
//	_pToken->type = (_tokType == NAME) ? get_keywordOrName(_p, _token) : _tokType;
//	_pToken->bytes = alifBytes_fromStringAndSize(_token->start, (_token->end - _token->start) * sizeof(wchar_t));
//
//	if (_pToken->bytes == nullptr) {
//		return -1;
//	}
//	if (alifASTMem_listAddAlifObj(_p->astMem, _pToken->bytes) < 0) {
//		ALIF_DECREF(_pToken->bytes);
//		return -1;
//	}
//
//	_pToken->data = nullptr;
//	if (_token->data != nullptr) {
//		if (alifASTMem_listAddAlifObj(_p->astMem, _token->data) < 0) {
//			ALIF_DECREF(_pToken->data);
//			return -1;
//		}
//	}
//
//	_pToken->level = _token->level;
//	_pToken->lineNo = _token->lineNo;
//	_pToken->colOffset = _p->tok->lineNo == _p->startingLineNo ? _p->startingColOffset + _token->colOffset : _token->colOffset;
//
//	_pToken->endLineNo = _token->endLineNo;
//	_pToken->endColOffset = _p->tok->lineNo == _p->startingLineNo ? _p->startingColOffset + _token->endColOffset : _token->endColOffset;
//
//	_p->fill_ += 1;
//
//	if (_tokType == ERRORTOKEN) {
//		// error
//	}
//
//	//return (_tokType == ERRORTOKEN ? alifParserEngine_tokenizerError(_p) : 0);
//	return 0;//
//}
//
//static int resize_tokensArr(AlifParser* _p) { 
//	int newSize = _p->size_ * 2;
//	AlifPToken** newTokens = (AlifPToken**)alifMem_dataRealloc(_p->tokens, newSize * sizeof(AlifPToken*)); // error when using realloc, need to review
//
//	_p->tokens = newTokens;
//	for (int i = _p->size_; i < newSize; i++) {
//		_p->tokens[i] = (AlifPToken*)calloc(1, sizeof(AlifPToken));
//	}
//
//	_p->size_ = newSize;
//	return 0;
//}
//
//int alifParserEngine_fillToken(AlifParser* _p) { 
//	AlifToken newToken{};
//	int type = alifTokenizer_get(_p->tok, &newToken);
//
//	AlifPToken* pT{};
//	//while (type == TYPEIGNORE) {
//	//	AlifSizeT len = newToken.endColOffset - newToken.colOffset;
//	//	wchar_t* tag = (wchar_t*)malloc(len + 1);
//	//	wcsncpy(tag, newToken.start, len);
//	//	tag[len] = L'0';
//	//	if (!growableCommentArr_add(&_p->typeIgnoreComments, _p->tok->lineNo, tag)) {
//	//		//goto error;
//	//	}
//	//	type = alifTokenizer_get(_p->tok, &newToken);
//	//}
//
//	if (_p->startRule == ALIFSINGLE_INPUT and type == ENDMARKER and _p->parsingStarted) {
//		type = NEWLINE;
//		_p->parsingStarted = 0;
//		if (_p->tok->indent) {
//			_p->tok->pendInd = -_p->tok->indent;
//			_p->tok->indent = 0;
//		}
//		else {
//			_p->parsingStarted = 1;
//		}
//	}
//
//	if ((_p->fill_ == _p->size_) and (resize_tokensArr(_p)) != 0) {
//		goto error;
//	}
//
//	pT = _p->tokens[_p->fill_];
//	return initialize_token(_p, pT, &newToken, type);
//
//error:
//	//alifToken_free(&newToken);
//	return -1;
//}
//
//int alifParserEngine_isMemorized(AlifParser* _p, int _type, void* _pres) { 
//	if (_p->mark_ == _p->fill_) {
//		if (alifParserEngine_fillToken(_p) < 0) {
//			_p->errorIndicator = 1;
//			return -1;
//		}
//	}
//
//	AlifPToken* t_ = _p->tokens[_p->mark_];
//
//	for (Memo* m_ = t_->memo; m_ != nullptr; m_ = m_->next) {
//		if (m_->type == _type) {
//			_p->mark_ = m_->mark_;
//			*(void**)_pres = m_->node;
//			return 1;
//		}
//	}
//	return 0;
//}

AlifIntT alifParserEngine_lookaheadWithInt(AlifIntT _positive,
	AlifPToken* (_func)(AlifParser*, AlifIntT), AlifParser* _p, AlifIntT _arg) { // 397
	AlifIntT mark_ = _p->mark;
	void* res = _func(_p, _arg);
	_p->mark = mark_;
	return (res != nullptr) == _positive;
}

AlifIntT alifParserEngine_lookahead(AlifIntT _positive,
	void* (_func)(AlifParser*), AlifParser* _p) { // 406
	AlifIntT mark_ = _p->mark;
	void* res = _func(_p);
	_p->mark = mark_;
	return (res != nullptr) == _positive;
}

AlifPToken* alifParserEngine_expectToken(AlifParser* _p, AlifIntT _type) { // 415
	/*
		إذا وصل المؤشر mark
		الى مؤشر الملء fill
		هذا يعني أنه لم يعد هنالك رموز المأخوذة tokens
		قابلة للإستخدام وبالتالي يجب جلب رمز جديد
		-------------------------------------------
		الفرق بين المؤشر الحالي ومؤشر الملء يدل على عدد الرموز المأخوذة tokens
	*/
	if (_p->mark == _p->fill) {
		if (alifParserEngine_fillToken(_p) < 0) {
			_p->errorIndicator = 1;
			return nullptr;
		}
	}
	AlifPToken* t = _p->tokens[_p->mark];
	if (t->type != _type) {
		return nullptr;
	}
	_p->mark += 1;
	return t;
}

//AlifPToken* alifParserEngine_expectTokenForced(AlifParser* _p, int _type, const wchar_t* _expected) { 
//	if (_p->errorIndicator == 1) return nullptr;
//
//	if (_p->mark_ == _p->fill_) {
//		if (alifParserEngine_fillToken(_p) < 0) {
//			_p->errorIndicator = 1;
//			return nullptr;
//		}
//	}
//
//	AlifPToken* t_ = _p->tokens[_p->mark_];
//	if (t_->type != _type) {
//		// error
//		return nullptr;
//	}
//	_p->mark_ += 1;
//	return t_;
//}
//
//AlifPToken* alifParserEngine_getLastNonWhitespaceToken(AlifParser* _p) { 
//	AlifPToken* token = nullptr;
//	for (int i = _p->mark_ - 1; i >= 0; i--) {
//		token = _p->tokens[i];
//		if (token->type != ENDMARKER and (token->type < NEWLINE or token->type > DEDENT)) {
//			break;
//		}
//	}
//
//	return token;
//}
//
//AlifObject* alifParserEngine_newIdentifier(AlifParser* _p, const wchar_t* _s) { 
//	AlifObject* id = alifUStr_decodeStringToUTF8(_s);
//
//	if (alifASTMem_listAddAlifObj(_p->astMem, id) < 0) {
//		// error
//	}
//	return id;
//}
//
//static Expression* alifParserEngine_nameFromToken(AlifParser* _p, AlifPToken* _t) { 
//	if (_t == nullptr) {
//		return nullptr;
//	}
//
//	const wchar_t* s = _alifWBytes_asString(_t->bytes);
//
//	if (!s) {
//		_p->errorIndicator = 1;
//		return nullptr;
//	}
//	AlifObject* id = alifParserEngine_newIdentifier(_p, s); // need work on it
//
//	return alifAST_name(id, Load, _t->lineNo, _t->colOffset, _t->endLineNo, _t->endColOffset, _p->astMem);
//}
//
//Expression* alifParserEngine_nameToken(AlifParser* _p) { 
//	AlifPToken* tok = alifParserEngine_expectToken(_p, NAME);
//	return alifParserEngine_nameFromToken(_p, tok);
//}
//
//void* alifParserEngine_stringToken(AlifParser* _p) { 
//	return alifParserEngine_expectToken(_p, STRING);
//}
//
//static AlifObject* parseNumber_raw(const wchar_t* _s) { 
//	const wchar_t* end{};
//	int64_t x{};
//	double dx{};
//	//int imflag{};
//
//	errno = 0;
//	end = _s + wcslen(_s) - 1;
//	if (_s[0] == L'0') {
//
//	}
//	else {
//		x = alifOS_strToLong(_s); // need edit
//		end++; // temp
//	}
//	if (*end == L'\0') {
//		if (errno != 0) {
//
//		}
//		return alifInteger_fromLongLong(x);
//	}
//
//	return nullptr; //
//}
//
//static AlifObject* parse_number(const wchar_t* _s) { 
//	wchar_t* dup{};
//	wchar_t* end{};
//	AlifObject* res{};
//
//	if (wcsstr(_s, L"_") == nullptr) {
//		return parseNumber_raw(_s);
//	}
//
//	dup = (wchar_t*)malloc(wcslen(_s) + 2);
//	end = dup;
//	for (; *_s; _s++) {
//		if (*_s != L'_') {
//			*end++ = *_s;
//		}
//	}
//	*end = L'\0';
//	res = parseNumber_raw(dup);
//	free(dup);
//	return res;
//}
//
//Expression* alifParserEngine_numberToken(AlifParser* _p) { 
//	AlifPToken* tok = alifParserEngine_expectToken(_p, NUMBER);
//	if (tok == nullptr) return nullptr;
//
//	const wchar_t* rawNum = _alifWBytes_asString(tok->bytes);
//
//	AlifObject* num = parse_number(rawNum);
//
//	if (alifASTMem_listAddAlifObj(_p->astMem, num) < 0) {
//		// error
//		return nullptr;
//	}
//
//	return alifAST_constant(num, nullptr, tok->lineNo, tok->colOffset, tok->endLineNo, tok->endColOffset, _p->astMem); // type should be NUMBER not nullptr!
//}


static AlifIntT compute_parserFlags(AlifCompilerFlags* flags) { // 769
	AlifIntT parser_flags = 0;
	if (!flags) {
		return 0;
	}
	if (flags->flags & ALIFCF_DONT_IMPLY_DEDENT) {
		parser_flags |= ALIFPARSE_DONT_IMPLY_DEDENT;
	}
	if (flags->flags & ALIFCF_IGNORE_COOKIE) {
		parser_flags |= ALIFPARSE_IGNORE_COOKIE;
	}
	if (flags->flags & CO_FUTURE_BARRY_AS_BDFL) {
		parser_flags |= ALIFPARSE_BARRY_AS_BDFL;
	}
	if (flags->flags & ALIFCF_TYPE_COMMENTS) {
		parser_flags |= ALIFPARSE_TYPE_COMMENTS;
	}
	if (flags->flags & ALIFCF_ALLOW_INCOMPLETE_INPUT) {
		parser_flags |= ALIFPARSE_ALLOW_INCOMPLETE_INPUT;
	}
	return parser_flags;
}


AlifParser* alifParserEngine_parserNew(TokenState* _tokState,
	AlifIntT _startRule, AlifIntT _flags, AlifIntT _featureVersion,
	AlifIntT* _error, AlifASTMem* _astMem) { // 796

	AlifParser* p_ = (AlifParser*)alifMem_dataAlloc(sizeof(AlifParser));
	if (p_ == nullptr) {
		//return (AlifParser*)alifErr_noMemory();
		return nullptr;
	}
	//_tokState->typeComments = (_flags & ALIFPARSE_TYPE_COMMENTS) > 0;

	p_->tok = _tokState;
	p_->keywords = nullptr;
	p_->nKeywordList = -1;
	p_->softKeyword = nullptr;
	p_->tokens = (AlifPToken**)alifMem_dataAlloc(sizeof(AlifPToken*));
	if (!p_->tokens) {
		alifMem_dataFree(p_);
		//return (AlifParser*)alifErr_noMemory();
		return nullptr; // temp
	}
	p_->tokens[0] = (AlifPToken*)alifMem_dataAlloc(sizeof(AlifPToken));
	if (!p_->tokens[0]) {
		alifMem_dataFree(p_->tokens);
		alifMem_dataFree(p_);
		//return (AlifParser*)alifErr_noMemory();
		return nullptr; // temp
	}
	//if (!growableComment_arrayInit(&p_->typeIgnoreComments, 10)) {
	//	alifMem_dataFree(p_->tokens[0]);
	//	alifMem_dataFree(p_->tokens);
	//	alifMem_dataFree(p_);
	//	return (AlifParser*)alifErr_noMemory();
	//}

	p_->mark = 0;
	p_->fill = 0;
	p_->size = 1;

	p_->errorCode = _error;
	p_->astMem = _astMem;
	p_->startRule = _startRule;
	p_->parsingStarted = 0;
	p_->normalize = nullptr;
	p_->errorIndicator = 0;
	p_->startingLineNo = 0;
	p_->startingColOffset = 0;
	p_->flags = _flags;
	p_->featureVersion = _featureVersion;
	p_->KnownErrToken = nullptr;
	p_->level = 0;
	p_->callInvalidRules = 0;

	return p_;
}

void alifParserEngine_parserFree(AlifParser* _p) { // 852
	ALIF_XDECREF(_p->normalize);
	for (int i = 0; i < _p->size; i++) {
		alifMem_dataFree(_p->tokens[i]);
	}
	alifMem_dataFree(_p->tokens);
	//growableComment_arrayDeallocate(&_p->typeIgnoreComments);
	alifMem_dataFree(_p);
}


void* alifParserEngine_runParser(AlifParser* _p) { // 883

	void* res = alifParserEngine_parse(_p);
	if (res == nullptr) {
		//if ((_p->flags & ALIFPARSE_ALLOW_INCOMPLETE_INPUT) and is_endOfSource(_p)) {
		//	alifErr_clear();
		//	return alifParserEngine_raiseError(_p, ALIFEXC_INCOMPLETEINPUTERROR, 0, "incomplete input");
		//}
		//if (alifErr_occurred() and !alifErr_exceptionMatches(_alifExcSyntaxError_)) {
		//	return nullptr;
		//}
		AlifPToken* lastToken = _p->tokens[_p->fill - 1];
		//reset_parserStateForErrorPass(_p);
		alifParserEngine_parse(_p);

		//alifParserEngine_setSyntaxError(_p, lastToken);
		return nullptr;
	}

	//if (_p->startRule == ALIF_SINGLE_INPUT and bad_singleStatement(_p)) {
	//	_p->tok->done = E_BADSINGLE;
	//	return RAISE_SYNTAX_ERROR("multiple statements found while compiling a single statement");
	//}

	return res;
}

ModuleTy alifParser_astFromFile(FILE* _fp, AlifIntT _startRule,
	AlifObject* _fn, const char* _enc, const char* _ps1, const char* _ps2,
	AlifCompilerFlags* _flags, AlifIntT* _error, AlifObject** _interactiveSrc,
	AlifASTMem* _astMem) { // 928 // _alifPegen_run_parser_from_file_pointer() 

	TokenState* tokState = alifTokenizerInfo_fromFile(_fp, _enc, _ps1, _ps2);
	if (tokState == nullptr) {
		//if (alifErr_occurred()) {
		//	alifParserEngine_raiseTokenizerInitError(_fn);
		//	return nullptr;
		//}
		return nullptr;
	}
	if (!tokState->fp or _ps1 != nullptr or _ps2 != nullptr
		or alifUStr_compareWithASCIIString(_fn, "<stdin>") == 0) {
		tokState->interactive = 1;
	}

	tokState->fn = ALIF_NEWREF(_fn);

	ModuleTy result{};
	AlifIntT parserFlags = compute_parserFlags(_flags);
	AlifParser* p_ = alifParserEngine_parserNew(tokState, _startRule,
		parserFlags, ALIF_MINOR_VERSION, _error, _astMem);
	if (p_ == nullptr) goto error;

	result = (ModuleTy)alifParserEngine_runParser(p_);
	alifParserEngine_parserFree(p_);

	if (tokState->interactive and tokState->interactiveSrcStart and result and _interactiveSrc != NULL) {
		*_interactiveSrc = alifUStr_fromString(tokState->interactiveSrcStart);
		if (!_interactiveSrc or alifASTMem_listAddAlifObj(_astMem, *_interactiveSrc) < 0) {
			ALIF_XDECREF(_interactiveSrc);
			result = nullptr;
			goto error;
		}
	}
error:
	alifTokenizer_free(tokState);
	return result;
}
