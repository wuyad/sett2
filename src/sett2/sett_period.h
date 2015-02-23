#ifndef __SETT_PERIOD_H__
#define __SETT_PERIOD_H__

#include <string>
#include <ace/Synch.h>
#include <ace/RW_Thread_Mutex.h>
#include <ace/Singleton.h>
#include "data_cache.h"
#include "system_define.h"
#include <my_datetime.h>
#include "task.h"
#include "cdr_sett.h"

/**
 * ��ǰ����Ϊ��ǰ��Ȼ�·ݡ�
 * �жϻ���ʱ���ջ�������ʱ���ж����������ڣ�������������ѹرգ�
 * �򽫴˻������뵱ǰ���ڡ� 
 * ϵͳ�������Զ��ر����ڵ�ʱ�䣬һ��Ϊ�¸��µ�5�����ң����ڴ�ʱ�� 
 * �Ļ�������Ϊ�ǳ������� 
 */
class sett_period:public common_stoppable_task{
public:
	sett_period();
	const std::string& get_current();
	void close_last();
	void force(const std::string& cur);
	bool force();
	bool load_sett_period();
	bool is_sett_period_closed(const std::string& cur);
	std::string parse_sett_period(cdr_ex& cdr);
	
	int svc();
private:
	std::string cur_str_;
	ACE_RW_Thread_Mutex mutex_;
	bool force_;
	std::map<std::string, int> periods_;
};

#define SETT_PERIOD_INS ACE_Singleton<sett_period, ACE_Thread_Mutex>::instance()

#endif // __SETT_PERIOD_H__
