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
 * 当前帐期为当前自然月份。
 * 判断话单时按照话单结束时间判断其所属帐期，如果所属帐期已关闭，
 * 则将此话单计入当前帐期。 
 * 系统可配置自动关闭帐期的时间，一般为下个月的5日左右，晚于此时间 
 * 的话单被认为是迟来话单 
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
