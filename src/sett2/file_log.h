#ifndef __FILE_LOG_H__
#define __FILE_LOG_H__

#include <ace/Thread_Mutex.h>
#include <ace/Singleton.h>
#include <string>
#include <set>
#include <wuya/timer.h>
#include "cdr_sett.h"

struct file_log_info {
	std::string filetype;
	std::string pathname;
	std::string filename;
	std::string procname;
	std::string pre_procname;
	int filesize;
	int record_num;
	int normal_num;
	int error_num;
	int repeat_num;
	int total_fee;
	int total_duration;
	std::string ftp_pathname;
	std::string ftp_filename;

	int local_size;
	CDR_TYPE proc_cdr_type;
	
	file_log_info():filetype(""), pathname(""), filename(""),procname(""),
	record_num(0),normal_num(0),error_num(0),repeat_num(0),total_fee(0),
	ftp_pathname(""), ftp_filename(""), total_duration(0),filesize(0)  {
	}
};
class file_log {
public:
	file_log();
	bool is_ftp_processed(const std::string& filename);
	bool log(file_log_info& info);
private:
	bool load_ftp_file_index();
	bool first_;
	std::set<std::string> ftp_file_index_;
	ACE_Thread_Mutex mutex_;
	static std::string sql_str_;
	static std::string audit_sql_str_;
	static std::string select_sql_str_;
};

#define FILE_LOG_INS ACE_Singleton<file_log, ACE_Thread_Mutex>::instance()

#endif // __FILE_LOG_H__

