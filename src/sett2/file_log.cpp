#include "file_log.h"
#include <ace/Synch.h>
#include <db_connect.h>
#include <boost/foreach.hpp>
#include "log.h"
#include <sstream>

using namespace std;

string file_log::sql_str_;
string file_log::audit_sql_str_;
string file_log::select_sql_str_;

bool file_log::is_ftp_processed(const std::string& filename) {
	ACE_GUARD_RETURN(ACE_Thread_Mutex, __NOT_USE__, mutex_, false);
	if( first_ ) {
		load_ftp_file_index();
		first_ = false;
	}
	return ftp_file_index_.find(filename) != ftp_file_index_.end();
}

file_log::file_log():first_(true) {
	sql_str_ = "insert into log_file_process(file_type, path_name, file_name, proc_name, "
			   "file_size, record_num, normal_num, error_num, repeat_num, total_fee, total_duration, "
			   "ftp_path_name, ftp_file_name) values("
			   ":f1<char[41]>,:f2<char[256]>, :f3<char[129]>, :f4<char[41]>"
			   ",:f5<int>, :f6<int>, :f7<int>, :f8<int>, :f9<int>, :f10<int>, :f11<int>"
			   ",:f12<char[256]>, :f13<char[129]>)";
	audit_sql_str_ = "insert into log_audit(proc_name, path_name, file_name, result, audit_user) "
					 "values (:f1<char[41]>, :f2<char[256]>, :f3<char[129]>, :f4<int>, :f5<char[21]>)";
	select_sql_str_ = "select file_size, normal_num, repeat_num, error_num, total_fee, total_duration from log_file_process "
					  "where path_name = :f1<char[256]> and file_name=:f2<char[129]>"
					  " and proc_name=:f3<char[41]> and file_type=:f4<char[41]>";
}

bool file_log::log(file_log_info& info) {
	ostringstream os;
	os << info.pathname << '/' << info.filename;
	ftp_file_index_.insert(os.str());
	database db;
	if( !db ) {
		return false;
	}
	try {
		otl_stream sql(1, sql_str_.c_str(), db);
		sql << info.filetype << info.pathname << info.filename << info.procname << info.filesize << 
		info.record_num << info.normal_num << info.error_num << info.repeat_num << info.total_fee 
		<< info.total_duration << info.ftp_pathname << info.ftp_filename;
	} catch( otl_exception& e ) {
		logerr <<"log file process info error, (" <<
		info.filename << ") file repeat insert:" << e.msg << endl;
	}
	// 审核校验
	int result = 0;
	if( info.procname=="collect" ) {
		if( info.filesize != info.local_size ) {
			result = 1;
		}
		try {
			otl_stream sql(1, audit_sql_str_.c_str(), db);
			sql << info.procname << info.pathname << info.filename << result << "system";
			return true;
		} catch( otl_exception& e ) {
			logerr <<"audit log error, (" <<
			info.filename << ") file repeat insert:" << e.msg << endl;
		}
	} else {
		int file_size=0;
		int normal_num=0;
		int repeat_num=0;
		int error_num=0;
		int fee=0;
		int duration=0;

		try {
			otl_stream sql(1, select_sql_str_.c_str(), db);
			sql << info.pathname << info.filename << info.pre_procname << info.filetype;
			while( !sql.eof() ) {
				sql >> file_size >> normal_num >> repeat_num >> error_num >> fee >> duration;
			}
			if( info.pre_procname=="collect" ) {
				if( file_size!=info.filesize ) {
					result = 1;
				}
			} else {
				// 费用与时长，如果是第一次计算，认为是正确的
				if( file_size!=info.filesize ) {
					result = 1;
				} else if( fee!=0 && fee!=info.total_fee ) {
					result = 3;
				} else if( duration!=0 && duration!=info.total_duration ) {
					result = 4;
				} else { // 记录数量
					if( info.proc_cdr_type==REPEAT_CDR ) {
						if( info.record_num != repeat_num ) {
							result = 2;
						}
					} else if( info.proc_cdr_type==ERROR_CDR ) {
						if( info.record_num != error_num ) {
							result = 2;
						}
					} else {
						if( info.record_num != normal_num ) {
							result = 2;
						}
					}
				}
			}
			otl_stream sql2(1, audit_sql_str_.c_str(), db);
			sql2 << info.procname << info.pathname << info.filename << result << "system";
		} catch( otl_exception& e ) {
			logerr <<"audit log error, (" <<
			info.filename << ") file repeat insert:" << e.msg << endl;
		}
	}
	return true;
}

bool file_log::load_ftp_file_index() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select path_name||'/'||file_name from log_file_process t "
						   "where proc_name='collect' and path_name like 'ftp://%'", db);
			copy(otl_input_iterator<string>(sql), otl_input_iterator<string>(),
				 inserter(ftp_file_index_, ftp_file_index_.end()));
			return true;
		} catch( ... ) {
			return false;
		}
	}
	return false;
}


