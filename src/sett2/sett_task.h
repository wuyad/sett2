#ifndef __SETT_TASK_H__
#define __SETT_TASK_H__

#include "task.h"
#include <ace/Task.h>
#include <ace/Synch.h>

class sett_task:public lf_task {
public:
	bool do_task(task_ptr task);
private:
	static std::pair<bool, std::string> move_working(const char* filename);
	static std::pair<bool, std::string> move_bak(const char* filename);
	static std::pair<bool, std::string> move_fail(const char* filename);
};

#define SETT_TASK_INS ACE_Singleton<sett_task, ACE_Thread_Mutex>::instance()

#endif // __SETT_TASK_H__


