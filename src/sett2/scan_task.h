#ifndef __SCAN_TASK_H__
#define __SCAN_TASK_H__

#include "task.h"
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include <ace/Task.h>
#include <ace/Synch.h>
#include "ftp_client.h"

class scan_task : public lf_task{
public:
	scan_task();
	bool init(const std::string& filename);
	void search();
	void reset();
	bool do_task(task_ptr task);
private:
	void parser_wildcard(const char* path_name);
	void parser_wildcard_ftp(wuya::ftp_client& ftp, const char* path_name);
	void file_find(const char* filename);
	void path_find(const char* filename, std::list<std::string>& l);
	int get_match_file_or_path(std::vector<wuya::ftp_file_stat>& vfs, const char* matcher, 
							   bool is_file, const char* path_name, std::vector<std::string>& path);
	void process_ftp_file(wuya::ftp_client& ftp, const char* file_name);
	void revise_path(std::string& path);

	void process_sett_period();
	
	std::string full_filename_;	// 文件名，含目录与文件名，可以含通配符
	std::list<std::string> pathnames_;
	std::string filetype_;
	std::string collect_source_;

	// for ftp
	bool is_ftp_;
	std::string user_name_;
	std::string password_;
	std::string ip_;
	unsigned short int port_;
	std::string local_root_;
	int keep_last_n_subpath_;
	std::string backup_path_;
	int repeat_type_;

	// for db_link
	bool is_db_link_;
	std::string content_;
};

#define SCAN_TASK_INS ACE_Singleton<scan_task, ACE_Thread_Mutex>::instance()

#endif // __SCAN_TASK_H__


