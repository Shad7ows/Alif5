#pragma once


class AlifHashTableEntryT { // 28
public:

	void* key{};
	void* value{};
};


class AlifHashTableT; // 41 
typedef class AlifHashTableT AlifHashTableT; // 42

typedef AlifHashTableEntryT* (*AlifHashTableGetEntryFunc)(AlifHashTableT*  ,const void* ); // 47

class AlifHashTableT { // 60
public:
	size_t nentries{}; // Total number of entries in the table
	size_t nbuckets{};

	AlifHashTableGetEntryFunc getEntryFunc{};

};


void* alifHashTable_get(AlifHashTableT* , const void* ); // 134
