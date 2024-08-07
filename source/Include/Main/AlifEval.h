#pragma once




AlifObject* alifEval_evalCode(AlifObject*, AlifObject* , AlifObject*);
AlifObject* alifEval_getBuiltins(AlifThread* );



AlifObject* alifEval_evalFrameDefault(AlifThread*, class AlifInterpreterFrame*, AlifIntT);
