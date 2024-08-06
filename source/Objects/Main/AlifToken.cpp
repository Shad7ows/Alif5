
#include "alif.h"
#include "AlifCore_AlifToken.h"




/* Token names */

const char* const alifParserTokenNames[] = {
	"ENDMARKER",
	"NAME",
	"NUMBER",
	"STRING",
	"NEWLINE",
	"INDENT",
	"DEDENT",
	"LPAR",
	"RPAR",
	"LSQB",
	"RSQB",
	"COLON",
	"COMMA",
	"PLUS",
	"MINUS",
	"STAR",
	"DOT",
	"SQRT",
	"EQUAL",
	"AMPER",
	"LEFTSHIFTEQUAL",
	"RIGHTSHIFTEQUAL",
	"DOUBLECIRCUMFLEXEQUAL",
	"VBAREQUAL",
	"AMPEREQUAL",
	"DOUBLESLASHEQUAL",
	"SLASHSTAREQUAL",
	"SLASHEQUAL",
	"STAREQUAL",
	"MINEQUAL",
	"PLUSEQUAL",
	"DOUBLESTAR",
	"EQEQUAL",
	"NOTEQUAL",
	"LESSEQUAL",
	"LESS",
	"GREATEREQUAL",
	"GREATER",
	"VBAR",
	"STARVBAR",
	"RIGHTSHIFT",
	"LEFTSHIFT",
	"DOUBLESLASH",
	"SLASH",
	"SLASHSTAR",
	"CIRCUMFLEX",
	"SLASHCIRCUMFLEX",
	"LBRACE",
	"RBRACE",
	"EXCLAMATION",
	"OP",
	"COMMENT",
	"FSTRING_START",
	"FSTRING_MIDDLE",
	"FSTRING_END",
	"NL",
	"<ERRORTOKEN>",

	"SOFT_KEYWORD",
	"<ENCODING>",
	"<N_TOKENS>",
};

AlifIntT alifToken_oneChar(AlifIntT _c1) {

	switch (_c1) {
	case '(': return LPAR;
	case ')': return RPAR;
	case '[': return LSQR;
	case ']': return RSQR;
	case '*': return STAR;
	case '+': return PLUS;
	case '-': return MINUS;
	case '.': return DOT;
	case ':': return COLON;
	case ',': return COMMA;
	case '&': return AMPER;
	case '<': return LESSTHAN;
	case '=': return EQUAL;
	case '>': return GREATERTHAN;
	case '!': return EXCLAMATION;
	case '/': return SLASH;
	case '^': return CIRCUMFLEX;
	case '{': return LBRACE;
	case '}': return RBRACE;
	case '|': return VBAR;
	}
	return OP;
}

AlifIntT alifToken_twoChars(AlifIntT _c1, AlifIntT _c2) {

	switch (_c1) {
	case '!':
		if (_c2 == '=') return NOTEQUAL;
		break;
	case '&':
		if (_c2 == '=') return AMPEREQUAL;
		break;
	case '*':
		if (_c2 == '*') return DOUBLESTAR;
		else if (_c2 == '=') return STAREQUAL;
		else if (_c2 == '|') return STARVBAR;
		break;
	case '+':
		if (_c2 == '=') return PLUSEQUAL;
		break;
	case '-':
		if (_c2 == '=') return MINUSEQUAL;
		break;
	case '/':
		if (_c2 == '/') return DOUBLESLASH;
		else if (_c2 == '=') return SLASHEQUAL;
		else if (_c2 == '^') return SLASHCIRCUMFLEX;
		else if (_c2 == '*') return SLASHSTAR;
		break;
	case '<':
		if (_c2 == '<') return LSHIFT;
		else if (_c2 == '=') return LESSEQUAL;
		break;
	case '=':
		if (_c2 == '=') return EQUALEQUAL;
		break;
	case '>':
		if (_c2 == '=') return GREATEREQUAL;
		else if (_c2 == '>') return RSHIFT;
		break;
	case '^':
		if (_c2 == '=') return CIRCUMFLEXEQUAL;
		break;
	case '|':
		if (_c2 == '=') return VBAREQUAL;
		break;
	}
	return OP;
}

AlifIntT alifToken_threeChars(AlifIntT _c1, AlifIntT _c2, AlifIntT _c3) {

	switch (_c1) {
	case '/':
		if (_c2 == '/') { if (_c3 == '=') return DOUBLESLASHEQUAL; }
		else if (_c2 == '*') { if (_c3 == '=') return SLASHSTAREQUAL; }
		break;
	case '<':
		if (_c2 == '<') { if (_c3 == '=') return LSHIFTEQUAL; }
		break;
	case '>':
		if (_c2 == '>') { if (_c3 == '=') return RSHIFTEQUAL; }
		break;
	case '^':
		if (_c2 == '^') { if (_c3 == '=') return DOUBLECIRCUMFLEXEQUAL; }
		break;
	}
	return OP;
}
