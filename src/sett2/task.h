#ifndef __TASK_H__
#define __TASK_H__

#include <string>
#include <queue>
#include <set>
#include <boost/shared_ptr.hpp>
#include <ace/Task.h>
#include <ace/Synch.h>

struct task_info {
	task_info(const std::string& p, const std::string& q, const std::string& s):
		file_name(p), file_type(q),collect_source(s) {
	}
	std::string file_name;
	std::string file_type;
	std::string collect_source;
};

typedef boost::shared_ptr<task_info> task_ptr;

class stoppable_task {
public:
	virtual bool is_stop()=0;
	virtual void stop()=0;
};

class common_stoppable_task:public ACE_Task_Base,
	public stoppable_task {
public:
	common_stoppable_task();
	bool is_stop();
	void stop();
	void sleep(int interval);
protected:
	ACE_Thread_Mutex wait_mutex_;
	ACE_Condition<ACE_Thread_Mutex> wait_cond_;
	bool to_stop_;
};

/**
 * leader/follower modle thread pool
 */
class lf_task:public ACE_Task_Base, public stoppable_task {
public:
	lf_task ();
	int svc();
	void stop();
	bool is_stop();

	bool getq(task_ptr& task);
	bool putq(task_info* task);
protected:
	virtual bool do_task(task_ptr task)=0;

	class follower {
	public:
		follower(ACE_Thread_Mutex &leader_lock);
		bool wait();
		bool signal();
		ACE_thread_t owner();
	private:
		ACE_Condition<ACE_Thread_Mutex> beleader_cond_;
		ACE_thread_t owner_;
	};

	bool to_stop_;
	ACE_thread_t current_leader_;
	ACE_Thread_Mutex leader_lock_;
	std::set<follower*> followers_;

	std::queue<task_ptr> tasks_;
	ACE_Thread_Mutex tasks_mutex_;
	ACE_Condition<ACE_Thread_Mutex> tasks_cond_;
private:
	bool become_leader();
	bool elect_new_leader();
	bool leader_active();
	void leader_active(ACE_thread_t leader);
};

#endif // __TASK_H__

