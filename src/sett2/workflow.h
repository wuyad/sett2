#ifndef __WORKFLOW_H__
#define __WORKFLOW_H__

#include <list>
#include "proc_base.h"
#include <boost/shared_ptr.hpp>

class workflow {
public:
	typedef boost::shared_ptr<proc_base> obj_t;
	typedef std::list<obj_t> obj_list_t;
	typedef obj_list_t::iterator iterator;
	typedef obj_list_t::const_iterator const_iterator;

	workflow();
	workflow(const char* def);
	
	void define(const char* def);
	const char* define();

	bool valid();
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;

private:
	bool make_storage_def(const std::string& def, obj_t& ret);
	
	obj_list_t objs_; //所有step与其对象实例
	std::string define_;
	bool good_;
};

#endif // __WORKFLOW_H__
