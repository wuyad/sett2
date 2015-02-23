#ifndef __CYCLE_TASK_H__
#define __CYCLE_TASK_H__

#include "task.h"
#include <vector>
#include <boost/function.hpp>

typedef boost::function<void ()> task_func_t;

struct common_func {
	common_func(const std::string& file_name, const std::string& file_type, const std::string& collect_source, lf_task* task):
		file_name_(file_name),file_type_(file_type),task_(task),collect_source_(collect_source) {
		
	}
	void operator()(){
		const char* p = file_name_.c_str();
		task_->putq(new task_info(file_name_, file_type_, collect_source_));
	}
	const std::string& file_name_;
	const std::string& file_type_;
	const std::string& collect_source_;
	lf_task* task_;
};


class cycle_task:public common_stoppable_task {
public:
	void add_cycle_task(int interval, task_func_t func);
	int svc();
private:
	struct func_info {
		func_info():interval_(0),magic_num_(0) {
		}
		func_info(int interval, task_func_t func):interval_(interval),magic_num_(0),
			func_(func) {
		}
		int interval_;
		boost::function<void ()> func_;
		int magic_num_;
	};

	std::vector<func_info> tasks_;
};

#define CYCLE_TASK_INS ACE_Singleton<cycle_task, ACE_Thread_Mutex>::instance()

#endif // __CYCLE_TASK_H__

