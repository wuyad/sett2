#include <boost_hpux_bug.h>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <vector>
#include "workflow.h"
#include "runtime_class.h"
using namespace std;
using namespace boost;

// workflow -------------------------------------------------------------
workflow::workflow():good_(false) {
}
workflow::workflow(const char* def):good_(false) {
	define(def);
}

void workflow::define(const char* def) {
	define_ = def;
	boost::trim(define_);
	if( define_.size()==0 ) {
		return;
	}
	const char* p = define_.c_str();
	const char* end = p+define_.size();
	const char* q=p;
	bool need_match = false;
	string step_define = "";
	obj_list_t tmp_obj;// 前
	while( q<=end ) {
		char c;
		if( q==end ) {
			c=',';
		} else {
			c = *q;
		}
		if( c=='[' ) {
			if( need_match ) {
				// 此时不应需要匹配
				return;
			}
			if( p!=q ) {
				if( step_define=="" ) {
					step_define.assign(p, q);
					step_define += "_proc";
					obj_t ptr(runtime_class::create_object(step_define.c_str()));
					if( ptr ) {
						string name("pre_");
						name += step_define;
						for_each(tmp_obj.begin(), tmp_obj.end(), 
								 boost::bind(&proc_base::set_name, _1, ref(name)));
						copy(tmp_obj.begin(), tmp_obj.end(), back_inserter(objs_));
						ptr->set_name(step_define);
						ptr->set_proc_type(proc_base::NORMAL_PROC);
						objs_.push_back(ptr);
						tmp_obj.clear();
					} else {
						return;
					}
				} else {
					// 多个step定义
					return;
				}
			}
			need_match = true;
			p=++q;
		} else if( c==']' ) {
			if( need_match ) {
				if( p == q ) {
					// 空定义
					return;
				}
				obj_t storage;
				if( !make_storage_def(string(p,q), storage) ) {
					// 错误定义
					return;
				}
				if( step_define=="" ) {	// 前存贮定义
					tmp_obj.push_back(storage);
				} else { // 后存贮定义
					storage->set_name(step_define);
					objs_.push_back(storage);
				}
				p=++q;
				need_match = false;
			} else {
				// 不匹配的[]
				return;
			}
		} else if( c==',' ) {
			if( need_match ) {
				// 不匹配的[]
				return;
			}
			if( p!=q ) {
				if( step_define=="" ) {
					step_define.assign(p, q);
					step_define += "_proc";
					obj_t ptr(runtime_class::create_object(step_define.c_str()));
					if( ptr ) {
						string name("pre_");
						name += step_define;
						for_each(tmp_obj.begin(), tmp_obj.end(), 
								 boost::bind(&proc_base::set_name, _1, ref(name)));
						copy(tmp_obj.begin(), tmp_obj.end(), back_inserter(objs_));
						ptr->set_name(step_define);
						ptr->set_proc_type(proc_base::NORMAL_PROC);
						objs_.push_back(ptr);
						tmp_obj.clear();
					} else {
						return;
					}
				} else {
					// 多个step定义
					return;
				}
			}
			if( step_define=="" ) {
				// 未定义step
				return;
			}
			step_define="";
			p = ++q;
		} else {
			++q;
		}
	}
	string prov_proc_name="collect";
	for(obj_list_t::iterator i=objs_.begin(); i!=objs_.end(); ++i){
		(*i)->set_prov_name(prov_proc_name);
		if((*i)->get_proc_type() == proc_base::NORMAL_PROC) {
			prov_proc_name = (*i)->get_name();
		}
	}
	good_ = true;
}

bool workflow::make_storage_def(const std::string& def, obj_t& ret) {
	vector<string> parts;
	boost::split(parts, def, bind1st(equal_to<char>(),':'));
	try {
		string& part0 = parts.at(0);
		boost::trim(part0);
		part0+="_storage";
		obj_t ptr(runtime_class::create_object(part0.c_str()));
		if( ptr ) {
			if( parts.size()>=1 ) {
				if( parts.at(1)=="repeat" ) {
					ptr->set_proc_cdr_type(REPEAT_CDR);
				} else if( parts.at(1)=="error" ) {
					ptr->set_proc_cdr_type(ERROR_CDR);
				}
			}
			ptr->set_proc_type(proc_base::STORAGE_PROC);
			ret = ptr;
			return true;
		}
	} catch( ... ) {
	}
	return false;
}

const char* workflow::define() {
	return define_.c_str();
}

bool workflow::valid() {
	return good_;
}

workflow::iterator workflow::begin() {
	return objs_.begin();
}
workflow::const_iterator workflow::begin() const{
	return objs_.begin();
}
workflow::iterator workflow::end() {
	return objs_.end();
}
workflow::const_iterator workflow::end() const{
	return objs_.end();
}


