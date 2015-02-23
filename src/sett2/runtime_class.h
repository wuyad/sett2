#ifndef __RUNTIME_CLASS_H__
#define __RUNTIME_CLASS_H__

#include <string>
#include <map>

class proc_base;
typedef proc_base* (*fn_create_object)();
class runtime_class {
public:
	typedef std::map<std::string, fn_create_object> map_runtime_class_t;
	static std::map<std::string, fn_create_object> all_proces_;
	static proc_base* create_object(const char* name){
		map_runtime_class_t::iterator i = all_proces_.find(name);
		if(i==all_proces_.end()) {
			return 0;
		}else{
			return ((*i).second)();
		}
	}
};

#define INIT_RUNTIME(class_name) \
	class_name* class_name##create_object(){ \
		return new class_name; \
	} \
	struct class_name##_runtime_init { \
		class_name##_runtime_init(){ \
			runtime_class::all_proces_[#class_name]=(fn_create_object)&class_name##create_object; \
		} \
	}; \
	class_name##_runtime_init class_name##_runtime_init__;
		

#endif // __RUNTIME_CLASS_H__
