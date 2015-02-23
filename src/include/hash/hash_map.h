#ifndef __WUYA_HASH_MAP_H__
#define __WUYA_HASH_MAP_H__
#include <hash/hash_func.h>

#ifdef _MSC_VER
	#include <hash_map>
using stdext::hash_map;
using stdext::hash_multimap;
#endif

#ifdef __HP_aCC
	#include <rw/stdex/hashmap.h>
	#include <rw/stdex/hashmmap.h>
template<class K, class V>
class hash_map: public rw_hashmap<K, V, _Hash<K>, std::equal_to<K>, std::allocator<std::pair<const K, V> > > {
public:
	typedef rw_hashmap<K, V, _Hash<K>, std::equal_to<K>, std::allocator<std::pair<const K, V> > > super;
	hash_map(typename super::size_type sz=1024)
	: super(sz, _Hash<K>(), equal_to<K>()) {
	}
};

template<class K, class V>
class hash_multimap : public rw_hashmultimap<K, V, _Hash<K>, std::equal_to<K>, std::allocator<std::pair<const K, V> > > {
public:
	typedef rw_hashmultimap<K, V, _Hash<K>, std::equal_to<K>, std::allocator<std::pair<const K, V> > > super;
	hash_multimap(typename super::size_type sz=1024)
	: super(sz, _Hash<K>(), equal_to<K>()) {
	}
};
#endif

#ifdef __GNUC__
#include <ext/hash_map>
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_multimap;
#endif
#endif // __WUYA_HASH_MAP_H__

