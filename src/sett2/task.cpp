#ifdef _MSC_VER
	#pragma warning( disable : 4996 4819 )
#endif
#include "task.h"

using namespace std;
using namespace boost;

// lf_task::follower ------------------------------------------------------------
lf_task::follower::follower(ACE_Thread_Mutex &leader_lock):beleader_cond_(leader_lock) {
	owner_ = ACE_Thread::self();
}

bool lf_task::follower::wait() {
	return beleader_cond_.wait() == 0;
}

bool lf_task::follower::signal() {
	return beleader_cond_.signal() == 0;
}

ACE_thread_t lf_task::follower::owner() {
	return owner_;
}

// lf_task -----------------------------------------------------------------------
lf_task::lf_task(): to_stop_(false), current_leader_(0), tasks_cond_(tasks_mutex_) {
}

void lf_task::stop() {
	ACE_GUARD(ACE_Thread_Mutex, leader_mon, leader_lock_);
	ACE_GUARD(ACE_Thread_Mutex, task_mon, tasks_mutex_);
	to_stop_ = true;
	tasks_cond_.broadcast();
}

bool lf_task::is_stop(){
	return to_stop_;
}
bool lf_task::leader_active() {
	return current_leader_ != 0;
}

void lf_task::leader_active(ACE_thread_t leader) {
	current_leader_ = leader;
}

bool lf_task::become_leader() {
	ACE_GUARD_RETURN(ACE_Thread_Mutex, leader_mon, leader_lock_, false);
//  if( is_stop() ) {
//  	return false;
//  }
	if( leader_active () ) {
		follower *fw;
		ACE_NEW_RETURN (fw, follower (leader_lock_), false);
		followers_.insert(fw);
		// Wait until told to do so.
		while( leader_active() )
			fw->wait();
		delete fw;
		followers_.erase(fw);
	}
	// Mark yourself as the active leader.
	leader_active(ACE_Thread::self ());
	return true;
}

bool lf_task::elect_new_leader() {
	ACE_GUARD_RETURN(ACE_Thread_Mutex, leader_mon, leader_lock_, false);
	leader_active (0);
	// Wake up a follower
	if( !followers_.empty() ) {
		// Get the old follower.
		set<follower*>::iterator i = followers_.begin();
		if( i == followers_.end() )
			return false;
		return(*i)->signal();
	} else {
		return false;
	}
}

int lf_task::svc() {
	while( true ) {
		if( !become_leader() )	// Block until this thread is the leader.
			break;
		// Get a message, elect new leader, then process message.
		task_ptr task;
		if( !getq(task) ) {
			// 没有取得任务，任务空且需要退出
			elect_new_leader(); // 产生了新的leader
			break;
		}else{
			elect_new_leader();
			ACE_Thread::yield();
			do_task(task);
		}
	}
	return 0;
}

bool lf_task::getq(task_ptr& task) {
	ACE_GUARD_RETURN(ACE_Thread_Mutex, task_mon, tasks_mutex_, false);
	while( tasks_.empty() ) {
		if( is_stop() ) {
			return false;
		}
		tasks_cond_.wait();
	}
	task = tasks_.front();
	tasks_.pop();
	return true;
}

bool lf_task::putq(task_info* task) {
	ACE_GUARD_RETURN(ACE_Thread_Mutex, task_mon, tasks_mutex_, false);
	tasks_.push(task_ptr(task));
	tasks_cond_.signal();
	return true;
}

// common_stoppable_task -----------------------------------------------------------
common_stoppable_task::common_stoppable_task():to_stop_(false), wait_cond_(wait_mutex_) {
}
bool common_stoppable_task::is_stop() {
	return to_stop_;
}
void common_stoppable_task::stop() {
	to_stop_ = true;
	ACE_GUARD(ACE_Thread_Mutex, __NOT_USE__, wait_mutex_);
	wait_cond_.signal();
}
void common_stoppable_task::sleep(int interval) {
	ACE_GUARD(ACE_Thread_Mutex, __NOT_USE__, wait_mutex_);
	ACE_Time_Value tv(ACE_OS::gettimeofday());
	tv += interval;
	wait_cond_.wait(&tv);
}


