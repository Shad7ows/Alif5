#pragma once

enum AlifSubTimeRoundT {
    AlifSubTime_Round_Floor = 0,

    AlifSubTime_Round_Ceiling = 1,

    AlifSubTime_Round_Half_Even = 2,

    AlifSubTime_Round_Up = 3,

    AlifSubTime_Round_Timeout = AlifSubTime_Round_Up
} ;

int alifSubTime_fromSecondsObject(AlifTimeT* , AlifObject*, AlifSubTimeRoundT);

AlifTimeT alifSubTime_asMicroseconds(AlifTimeT , AlifSubTimeRoundT );

AlifTimeT alifSubTime_add(AlifTimeT, AlifTimeT );

class AlifClockInfoT {
public:
	const wchar_t* implementation;
	int monotonic;
	int adjustable;
	double resolution;
} ;


class AlifTimeFraction{
public:
	AlifTimeT numer;
	AlifTimeT denom;
} ;
