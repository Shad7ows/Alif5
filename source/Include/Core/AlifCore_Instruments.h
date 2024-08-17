#pragma once


int alifCall_instrumentationArg(
	AlifThread* , int ,
	AlifInterpreterFrame* , AlifCodeUnit* , AlifObject* );
int alifCall_instrumentation2Args(
	AlifThread* , int ,
	AlifInterpreterFrame* , AlifCodeUnit* , AlifObject* , AlifObject* );
void alifCall_instrumentationExc2(
	AlifThread* , int ,
	AlifInterpreterFrame* , AlifCodeUnit* , AlifObject* , AlifObject* );

extern AlifObject _alifInstrumentationMissing_;
extern AlifObject _alifInstrumentationDisable_;
