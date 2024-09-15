#pragma once


class AlifBytesWriter{ // 67
public:
	AlifObject* buffer{};

	AlifSizeT allocated{};

	AlifSizeT minSize{};

	AlifIntT useByteArray{};

	AlifIntT overAllocate{};

	AlifIntT useSmallBuffer{};
	char smallBuffer[512]{};
};
