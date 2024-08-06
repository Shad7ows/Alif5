#pragma once

#include "AlifObject.h"

#define MAXINDENT 100
#define MAXLEVEL 200
#define MAXFSTRING_LEVEL 150

#define INSIDE_FSTRING(tok) (tok->tokModeStackIndex > 0)
#define INSIDE_FSTRING_EXPR(tok) (tok->curlyBracExprStartDepth >= 0)

class AlifToken { // token
public:
	AlifIntT level{};
	AlifIntT lineNo{}, colOffset{}, endLineNo{}, endColOffset{};
	const char* start{}, * end{};
	AlifObject* data{};
};

enum TokenizerModeType {
	Token_RegularMode,
	Token_FStringMode,
};

#define MAX_EXPR_NESTING 3

class TokenizerMode {
public:
	TokenizerModeType type{};

	AlifIntT curlyBracDepth{};
	AlifIntT curlyBracExprStartDepth{};

	char fStringQuote{};
	AlifIntT fStringQuoteSize{};
	AlifIntT fStringRaw{};

	const char* fStringStart{};
	const char* fStringMultiLineStart{};
	AlifIntT fStringLineStart{};

	AlifSizeT fStringStartOffset{};
	AlifSizeT fStringMultiLineStartOffset{};

	AlifSizeT lastExprSize{};
	AlifSizeT lastExprEnd{};
	char* lastExprBuff{};
};

class TokenInfo {
public:
	char* buf{}, * cur{}, * inp{};
	const char* start{}, * end{};
	AlifIntT done{};
	FILE* fp{};
	AlifIntT tabSize{};
	AlifIntT indent{};
	AlifIntT indStack[MAXINDENT]{};
	AlifIntT atBeginOfLine{ 1 };
	AlifIntT pendInd{};
	const char* prompt{}, * nextPrompt{};
	AlifIntT lineNo{};
	AlifIntT firstLineNo{};
	AlifIntT startingColOffset{ -1 };
	AlifIntT colOffset{ -1 };
	AlifIntT level{};
	char parenStack[MAXLEVEL]{};
	AlifIntT parenLineNoStack[MAXLEVEL]{};
	AlifIntT parenColStack[MAXLEVEL]{};
	AlifObject* fn;

	AlifIntT alterIndStack[MAXINDENT]{};

	AlifIntT countLine{};
	const char* lineStart{};
	const char* multiLineStart{};

	char* string;
	char* input;

	AlifIntT comment{};

	AlifIntT (*underflow) (TokenInfo*);

	TokenizerMode tokModeStack[MAXFSTRING_LEVEL]{};
	AlifIntT tokModeStackIndex{};
	AlifIntT tokExtraTokens{};
	AlifIntT commentNewline{};
	AlifIntT implicitNewline{};
};



TokenInfo* alifTokenizer_newTokenInfo();
AlifIntT alifLexer_setupToken(TokenInfo*, AlifToken*, AlifIntT, const char*, const char*);
