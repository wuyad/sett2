#include "scan_task.h"
#include "data_finder.h"
#include "log.h"
#include <wuya/filefind.h>
#include <wuya/filestat.h>
#include <wuya/wildcard.h>
#include <wuya/fileopt.h>
#include <boost_hpux_bug.h>
#include <boost/algorithm/string.hpp>
#include <boost/scoped_array.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include "sett_task.h"
#include "sett_period.h"
#include "data_cache.h"
#include "system_define.h"
#include "file_log.h"

using namespace std;
using namespace boost;
using namespace wuya;

// scan_task -----------------------------------------------------------------------
scan_task::scan_task():is_ftp_(false),port_(21),is_db_link_(false),repeat_type_(0) {
}
bool scan_task::init(const string& filename) {
	full_filename_ = trim_copy(filename);
	// ftp://user20:user2020@61.132.87.163:2100/12/*,/local/dir,keep_last_n_dir[int],repeat_type[int])
	// repeat_type 重复策略，目前仅对ftp有效 0-移至当前目录的bak子目录下 1-不移动原始文件，通过索引排重
	if( istarts_with(full_filename_ , "ftp://") ) {
		is_ftp_ = true;
		const char* p = full_filename_.c_str()+6; // 6--"ftp://"
		const char* end = full_filename_.c_str()+full_filename_.length();
		const char* p0 = p;
		const char* p1 = 0;
		string tmp_dir;
		int state=0;
		char c;
		while( p<=end ) {
			if( p==end ) {
				c = ',';
			} else {
				c=*p;
			}
			if( state < 2 ) {
				if( c==':'  && state==0 ) {
					p1=p++;
				} else if( c=='@'  && state==0 ) {
					if( p1>=p0 ) {
						user_name_.assign(p0, p1);
						password_.assign(p1+1, p);
					} else {
						user_name_.assign(p0, p);
					}
					p0 = ++p;
				} else if( c=='/' && state==0 ) {
					if( p1>=p0 ) {
						ip_.assign(p0, p1);
						port_ = atoi(string(p1+1, p).c_str());
					} else {
						ip_.assign(p0, p);
					}
					p0 = p++;
					state = 1;
				} else if( c==','  && state==1 ) {
					state = 2;
					tmp_dir.assign(p0, p);
					p0 = ++p;
				} else {
					++p;
				}
			} else { // state >= 2,
				if( c==',' ) {
					if( state == 2 ) {
						local_root_.assign(p0, p);
						state = 3;
					} else if( state==3 ) {
						keep_last_n_subpath_ = atoi(string(p0, p).c_str());
						state=4;
					} else if( state==4 ) {
						repeat_type_ = atoi(string(p0, p).c_str());
						state=5;
					}
					p0 = ++p;
				} else {
					++p;
				}
			}
		}
		if( local_root_=="" ) {
			local_root_ = data_finder::get_ftp_default_local_root();
			keep_last_n_subpath_ = data_finder::get_ftp_keep_last_n_subpath();
		}
		full_filename_ = tmp_dir;
		if( full_filename_[full_filename_.length()-1]=='/' ) {
			full_filename_+='*';
		}
		// db_link://user/pwd@orcl:table_name
	} else if( istarts_with(full_filename_ , "db_link://") ) {
		is_db_link_ = true;
		full_filename_.erase(0, 10); // 10--"db_link://"
		content_ = full_filename_;
		string::size_type i;
		string user,pwd,orcl,table_name;
		if( (i = full_filename_.find('/')) != string::npos ) {
			user.assign(full_filename_, 0, i);
			full_filename_.erase(0, i+1);
		}
		if( (i = full_filename_.find('@')) != string::npos ) {
			pwd.assign(full_filename_, 0, i);
			full_filename_.erase(0, i+1);
		}
		if( (i = full_filename_.find(':')) != string::npos ) {
			orcl.assign(full_filename_, 0, i);
			full_filename_.erase(0, i+1);
		}
		table_name = full_filename_;
		full_filename_ = CONFIG.get<const char*>("root_path", "db_link");
		if( full_filename_.length()>0 && full_filename_[full_filename_.length()-1]!=FILE_SEP ) {
			full_filename_.append(1, FILE_SEP);
		}
		full_filename_.append(user);
		full_filename_.append(1, '_');
		full_filename_.append(orcl);
		full_filename_.append(1, '_');
		full_filename_.append(table_name);
		filestat fs(full_filename_.c_str());
		full_filename_ = fs.get_fullname();
	} else {
		if( full_filename_[full_filename_.length()-1]==FILE_SEP ) {
			full_filename_ += '*';
		}
		filestat fs(full_filename_.c_str());
		full_filename_ = fs.get_fullname();
		if( fs.is_dir() ) {
			if( full_filename_[full_filename_.length()-1]!=FILE_SEP ) {
				full_filename_ += FILE_SEP;
			}
			full_filename_ += '*';
		}
	}
	return true;
}

void scan_task::parser_wildcard(const char* path_name0) {
	char* path_name = new char[strlen(path_name0)+1];
	scoped_array<char> sa_path_name(path_name);
	strcpy(path_name, path_name0);
	char* p=path_name, *q=path_name;
	bool has_wildcard = false;
	bool is_end = false;
	char c;
	for( ; p<=path_name+strlen(path_name); ++p ) {
		if( p==path_name+strlen(path_name) ) {
			is_end = true;
			c = FILE_SEP;
		} else {
			c = *p;
		}
		if( c=='*' || c=='?' ) {
			has_wildcard = true;
		} else if( c==FILE_SEP ) {
			if( has_wildcard ) {
				*p = '\0';
				*q = '\0';
				filefind ff(path_name, q+1, 0, filefind::only_dir);
				list<string> l;
				ff.scan(bind(&scan_task::path_find, this, _1, ref(l)));
				for( list<string>::iterator i=l.begin(); i!=l.end(); ++i ) {
					if( is_end ) {
						pathnames_.push_back(*i);
					} else {
						(*i).append(1, FILE_SEP);
						(*i).append(p+1);
						parser_wildcard((*i).c_str());
					}
				}
				return;
			} else {
				q = p;
				if( is_end ) {
					pathnames_.push_back(string(path_name, p));
				}
			}
		}
	}
}

/**
 *
 * @param vfs
 * @param matcher
 * @param is_file
 * @param path
 *
 * @return 0--没有匹配
 */
int scan_task::get_match_file_or_path(vector<ftp_file_stat>& vfs, const char* matcher,
									  bool is_file, const char* path_name, vector<string>& path) {
	for( vector<ftp_file_stat>::iterator i=vfs.begin();
	   i<vfs.end(); ++i ) {
		if( i->is_dir!=is_file ) {
			if( match(i->filename.c_str(), matcher) ) {
				string temp=path_name;
				temp += '/';
				temp += i->filename;
				path.push_back(temp);
			}
		}
	}
	return 0;
}

void scan_task::process_ftp_file(wuya::ftp_client& ftp, const char* file_name) {
	if( is_stop() ) {
		return;
	}
	size_t len=0;
	if( file_name==0 || (len=strlen(file_name))==0 ) {
		return;
	}
	// 如果已重复，则不采集
	if( repeat_type_==1 ) {
		ostringstream os;
		os << "ftp://" << user_name_ << "@" << ip_ << ":" << port_ << file_name;
		if( FILE_LOG_INS->is_ftp_processed(os.str()) ) {
			loginfo << "ftp remote file <" << file_name << "> has processed" << endl;
			return;
		}
	}
	string remote_pathname;
	string remote_filename;	//不含路径
	string local_filename = local_root_; //含路径
	string local_pathname = local_root_;
	const char* p=file_name+strlen(file_name)-1;
	const char* q = p;
	int n_dir=keep_last_n_subpath_;
	while( p>=file_name ) {
		if( *p=='/' ) {
			if( n_dir==keep_last_n_subpath_ ) {
				remote_pathname.assign(file_name, p);
				remote_filename.assign(p+1);
				q=p;
			}
			if( n_dir==0 ) {
				local_filename+=p;
				local_pathname.append(p,q);
				break;
			}
			--n_dir;
		}
		--p;
	}
	// 如果保留目录设置过大
	if( n_dir>0 ) {
		local_filename+=p;
		local_pathname.append(p,q);
	}
	revise_path(local_pathname);
	revise_path(local_filename);
	make_dir(local_pathname.c_str());
	if( filestat(local_pathname.c_str()).exist() ==false ) {
		logerr << "ftp local mkdir <" << local_pathname << "> error" << endl;
		return;
	}
	if( !ftp.download_file(file_name, local_filename) ) {
		logerr << "ftp download file <" << local_filename << "> error" << endl;
		return;
	} else {
		loginfo << "ftp download file <" << local_filename << "> OK" << endl;
	}
	{
		file_log_info info;
		info.filetype = filetype_;
		vector<ftp_file_stat> ftp_fs;
		ftp.ll(file_name, ftp_fs);
		info.filesize = ftp_fs[0].file_size;
		ostringstream os;
		os << "ftp://" << user_name_ << '@' << ip_ << ':' << port_ << remote_pathname;
		info.pathname = local_pathname;
		info.filename = remote_filename;
		info.procname = "collect";
		info.ftp_filename = os.str();
		info.ftp_pathname = remote_filename;
		filestat fs(local_filename.c_str());
		info.local_size = fs.length();
		FILE_LOG_INS->log(info);
	}

	if( repeat_type_==0 ) {
		remote_pathname.append(1, '/');
		remote_pathname.append("bak");
		// 移动文件
		ftp.mkdir(remote_pathname);

		remote_pathname.append(1, '/');
		remote_pathname.append(remote_filename);
		if( !ftp.rename(file_name, remote_pathname) ) {
			logerr << "ftp move file <" << remote_pathname << "> error" << endl;
			return;
		} else {
			loginfo << "ftp move file <" << remote_pathname << "> OK" << endl;
		}
	}
	// 处理本地文件
	SETT_TASK_INS->putq(new task_info(local_filename, filetype_, collect_source_));
	loginfo << "file:" << local_filename << " found." << endl;
	ACE_Thread::yield();
}

void scan_task::parser_wildcard_ftp(ftp_client& ftp, const char* path_name0) {
	char* path_name = new char[strlen(path_name0)+1];
	scoped_array<char> sa_path_name(path_name);
	strcpy(path_name, path_name0);
	char* p=path_name, *q=path_name;
	bool has_wildcard = false;
	bool is_end = false;
	char c;
	for( ; p<=path_name+strlen(path_name); ++p ) {
		if( p==path_name+strlen(path_name) ) {
			is_end = true;
			c = '/';
		} else {
			c = *p;
		}
		if( c=='*' || c=='?' ) {
			has_wildcard = true;
		} else if( c=='/' ) {
			if( has_wildcard ) {
				*p = '\0';
				*q = '\0';

				vector<ftp_file_stat> vfs;
				if( path_name==q ) {
					if( !ftp.ll("/", vfs) ) {
						return;
					}
				} else {
					if( !ftp.ll(path_name, vfs) ) {
						return;
					}
				}
				vector<string> file_or_path_name;
				get_match_file_or_path(vfs, q+1, is_end, path_name, file_or_path_name);
				if( is_end ) { // 文件
					BOOST_FOREACH(string& i, file_or_path_name) {
						process_ftp_file(ftp, i.c_str());
					}
					return;
				} else { // 目录
					BOOST_FOREACH(string& i, file_or_path_name) {
						i += '/';
						i += (p+1);
						parser_wildcard_ftp(ftp, i.c_str());
					}
				}
				return;
			} else {
				q = p;
				if( is_end ) {
					process_ftp_file(ftp, string(path_name, p).c_str());
					return;
				}
			}
		}
	}
}

void scan_task::revise_path(string& path) {
#ifdef WIN32
	boost::replace_all(path, "/", "\\");
	boost::replace_all(path, "\\\\", "\\");
	if( path[0]=='\\' ) {
		path = "C:"+path;
	}
#else
	boost::replace_all(path, "//", "/");
#endif
}


void scan_task::reset() {
	pathnames_.clear();
}

void scan_task::search() {
	reset();
	process_sett_period();
	if( is_ftp_ ) {
		ftp_client ftp;
		if( !ftp.login(ip_, port_, user_name_, password_) ) {
			logerr << "ftp login error." << endl;
			return;
		}
		parser_wildcard_ftp(ftp, full_filename_.c_str());
		ftp.logout();
	} else if( is_db_link_ ) {
		ofstream f(full_filename_.c_str());
		f << content_;
		f.close();
		file_find(full_filename_.c_str());
	} else {
		// full_filename_为绝对路径
		// 取文件名中的各个部分,比如/a/b/b*/bb/*,将变成/a/b b* bb,filename==*
		filestat fs(full_filename_.c_str());
		// 分析目录名中的通配符，对unix/linux的命令行运行无效，因为shell会展开文件名
		parser_wildcard(fs.get_filepath());
		for( list<string>::iterator i = pathnames_.begin(); i != pathnames_.end(); ++i ) {
			filefind finder((*i).c_str(), fs.get_filename() , 0, filefind::no_dir);
			finder.scan(bind(&scan_task::file_find, this, _1));
		}
	}
}

void scan_task::file_find(const char* filename) {
	if( is_stop() ) {
		return;
	}
	{
		// 记录文件采集日志
		file_log_info info;
		info.filetype = filetype_;
		filestat fs(filename);
		info.filesize = fs.length();
		info.local_size = info.filesize;
		info.pathname = fs.get_filepath();
		info.filename = fs.get_filename();
		info.procname = "collect";
		FILE_LOG_INS->log(info);
	}
	SETT_TASK_INS->putq(new task_info(filename, filetype_, collect_source_));
	loginfo << "file:" << filename << " found." << endl;
	ACE_Thread::yield();
}

void scan_task::path_find(const char* filename, list<string>& v) {
	v.push_back(filename);
}

void scan_task::process_sett_period() {
	wuya::datetime sp(SETT_PERIOD_INS->get_current().c_str());
	int month = sp.month();
	char buf[3];
	char buf2[3];
	sprintf(buf, "%d", month);
	sprintf(buf2, "%02d", month);
	string::size_type i;
	while( (i=full_filename_.find("$M")) != string::npos ) {
		full_filename_.replace(i, 2, buf2);
	}
	while( (i=full_filename_.find("$m")) != string::npos ) {
		full_filename_.replace(i, 2, buf);
	}
}

bool scan_task::do_task(task_ptr task) {
	filetype_ = task->file_type;
	collect_source_ = task->collect_source;
	is_ftp_ = false;
	port_ = 21;
	is_db_link_ = false;
	repeat_type_=0;
	init(task->file_name);
	search();
	return true;
}

