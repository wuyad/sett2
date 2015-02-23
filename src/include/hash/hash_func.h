#ifndef __WUYA_HASH_FUNC_H__
#define __WUYA_HASH_FUNC_H__
#include <stdlib.h>
#include <string>

#ifndef _MSC_VER

	#define _HASH_SEED	(size_t)0xdeadbeef

template <class _InIt>
inline size_t _Hash_value(_InIt _Begin, _InIt _End) {	// hash range of elements
	size_t _Val = 2166136261U;
	while( _Begin != _End )
		_Val = 16777619U * _Val ^ (size_t)*_Begin++;
	return(_Val);
}

template<class _Elem,
class _Traits,
class _Alloc> inline
size_t hash_value(const std::basic_string<_Elem, _Traits, _Alloc>& _Str) {	 // hash string to size_t value
	const _Elem *_Ptr = _Str.c_str();
	return(_Hash_value(_Ptr, _Ptr + _Str.size()));
}


template<class _Kty> inline
size_t hash_value(const _Kty& _Keyval) {   // hash _Keyval to size_t value one-to-one
	return((size_t)_Keyval ^ _HASH_SEED);
}

inline size_t hash_value(const char *_Str) {   // hash NTBS to size_t value
	return(_Hash_value(_Str, _Str + ::strlen(_Str)));
}

inline size_t hash_value(const wchar_t *_Str) {	  // hash NTWCS to size_t value
	return(_Hash_value(_Str, _Str + ::wcslen(_Str)));
}


template<class _Kty>
class _Hash {	// traits class for hash function
public:
	size_t operator()(const _Kty& _Keyval) const {	 // hash _Keyval to size_t value by pseudorandomizing transform
		ldiv_t _Qrem = ldiv((size_t)hash_value(_Keyval), 127773);
		_Qrem.rem = 16807 * _Qrem.rem - 2836 * _Qrem.quot;
		if( _Qrem.rem < 0 )
			_Qrem.rem += 2147483647;
		return((size_t)_Qrem.rem);
	}
};

	#ifdef __GNUC__
#include <ext/hash_fun.h>
namespace __gnu_cxx {
	template<> struct hash<std::string> {
		size_t operator()(const std::string& __s) const {
			return hash_value(__s);
		}
	};

}
	#endif

#endif
#endif // __WUYA_HASH_FUNC_H__
