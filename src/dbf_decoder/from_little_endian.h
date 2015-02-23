#ifndef __FROM_LITTLE_ENDIAN_H__
#define __FROM_LITTLE_ENDIAN_H__
#include <ace/Basic_Types.h>

#if defined (ACE_LITTLE_ENDIAN)
	#define LONG_FROM_LE(X) X
	#define SHORT_FROM_LE(X) X
#else
	#define LONG_FROM_LE(X) ACE_SWAP_LONG (X)
	#define SHORT_FROM_LE(X) ACE_SWAP_WORD(X)
#endif

inline int get_long_le(const char* p){
	int ret = 0;
	memcpy(&ret, p, sizeof(int));
	return LONG_FROM_LE(ret);
}

inline short int get_short_le(const char* p){
	short int ret = 0;
	memcpy(&ret, p, sizeof(short int));
	return SHORT_FROM_LE(ret);
}

#endif // __FROM_LITTLE_ENDIAN_H__

