#ifndef __WUYA_HASH_SET_H__
#define __WUYA_HASH_SET_H__
#include <hash/hash_func.h>

#ifdef _MSC_VER
	#include <hash_set>
using stdext::hash_set;
using stdext::hash_multiset;
#endif

#ifdef __HP_aCC
	#include <rw/stdex/hashset.h>
	#include <rw/stdex/hashmset.h>
template<class K>
class hash_set : public rw_hashset<K, _Hash<K>, std::equal_to<K>, std::allocator<K> > {
public:
	typedef rw_hashset<K, _Hash<K>, std::equal_to<K>, std::allocator<K> > super;
	hash_set(typename super::size_type sz=1024)
	: super(sz, _Hash<K>(), equal_to<K>()) {
	}
};

template<class K>
class hash_multiset : public rw_hashmultiset<K, _Hash<K>, std::equal_to<K>, std::allocator<K> > {
public:
	typedef rw_hashmultiset<K, _Hash<K>, std::equal_to<K>, std::allocator<K> > super;
	hash_multiset(typename super::size_type sz=1024)
	: super(sz, _Hash<K>(), equal_to<K>()) {
	}
};
#endif

#ifdef __GNUC__
using __gnu_cxx::std::set;
using __gnu_cxx::hash_multiset;
#endif
#endif // __WUYA_HASH_SET_H__

