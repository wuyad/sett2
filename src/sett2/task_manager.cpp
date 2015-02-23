#include <iostream>
#include <iomanip>
#include <string>
#include <wuya/config.h>
#include <wuya/get_opt.h>
#include <wuya/filestat.h>
#include <boost/foreach.hpp>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include <ace/Signal.h>
#include "task_manager.h"
#include "data_cache.h"
#include "data_finder.h"
#include "log.h"
#include "system_define.h"
#include "scan_task.h"
#include "sett_task.h"
#include "index_task.h"
#include "cycle_task.h"
#include "sett_period.h"
#include "stat_proc.h"

using namespace std;
using namespace wuya;

void my_sighandler(int signo) {
	cout << "receviced  Ctrl-C or other terminate signal, system will shutdown, wait for a white..."<< endl;
	ACE_Singleton<task_manager,ACE_Thread_Mutex>::instance()->shutdown();
}

// task_manager --------------------------------------------------------------------------
task_manager::task_manager():one_off_task_(0) {
}
void task_manager::run() {
	// 设置信号捕获
	ACE_Sig_Action sa (my_sighandler);

	ACE_Sig_Set ss;
	ss.sig_add (SIGUSR1);
	ss.sig_add (SIGTERM);
	ss.sig_add (SIGINT);
	sa.mask (ss);

	// Register the same handler function for these
	sa.register_action(SIGUSR1);
	sa.register_action(SIGINT);
	sa.register_action(SIGTERM);

	// 初始化处理线程队列
	std::map<string, file_source_info>& dirs = data_finder::get_file_source_info();
	int num_file_scan_thread = one_off_task_?filenames_.size():dirs.size();

	SETT_TASK_INS->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
							data_finder::get_max_sett_thread());
	INDEX_TASK_INS->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,1);
	SETT_PERIOD_INS->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,1);

	cout << "total thread num: " << ACE_Thread_Manager::instance()->count_threads() << endl;
	//等待所有线程到位
	cout << "all thread runing..." << endl;
	if( one_off_task_ == 1 ) {
		const std::map<string, int>& spans = INDEX_TASK_INS->get_close_span();
		for( std::map<string, int>::const_iterator i=spans.begin(); i!=spans.end(); ++i ) {
			CYCLE_TASK_INS->add_cycle_task((*i).second*60,
										   common_func("", (*i).first, "", INDEX_TASK_INS));
		}
		CYCLE_TASK_INS->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,1);
		BOOST_FOREACH(std::string& i, filenames_) {
			task_ptr search_task(new task_info(i, file_type_, collect_source_));
			SCAN_TASK_INS->do_task(search_task);
		}
		shutdown();
	} else {
		SCAN_TASK_INS->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED, 
								num_file_scan_thread);
		typedef std::map<string, file_source_info>::value_type pair_file_source_info;
		BOOST_FOREACH(const pair_file_source_info& i, dirs) {
			if( one_off_task_==2 ) {
				if( i.second.type == file_type_ ) {
					CYCLE_TASK_INS->add_cycle_task(i.second.interval, 
												   common_func(i.first, i.second.type, 
															   i.second.source, SCAN_TASK_INS));
				}
			} else {
				CYCLE_TASK_INS->add_cycle_task(i.second.interval, 
											   common_func(i.first, i.second.type,
														   i.second.source, SCAN_TASK_INS));
			}
		}
		const std::map<string, int>& spans = INDEX_TASK_INS->get_close_span();
		for( std::map<string, int>::const_iterator i=spans.begin(); i!=spans.end(); ++i ) {
			CYCLE_TASK_INS->add_cycle_task((*i).second*60,
										   common_func("", (*i).first, "", INDEX_TASK_INS));
		}
		CYCLE_TASK_INS->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,1);
	}
	// 系统阴塞,直至所有线程安全退出
	ACE_Thread_Manager::instance()->wait();
	cout << "system stopping..." << endl;
}

bool task_manager::init_data() {
	// 初始化日志
	LOGSTREAM::init(wuya::DAY_CHANGE, LOGFILE_NAME);
	// 缓存数据
	if( !ACE_Singleton<data_cache,ACE_Thread_Mutex>::instance()->init() ) {
		return false;
	}
	return true;
}

void task_manager::finish() {
	// 清除数据
	ACE_Singleton<data_cache,ACE_Thread_Mutex>::instance()->fini();
}

bool task_manager::parser_command_opt(int argc, char *argv[]) {
	if( argc == 1 ) {
		cout << "system will run as daemon mode." << endl;
	} else {
		get_opt opts(argc, (const char**)argv);
		if( opts.has_option('t') ) {
			file_type_ = opts.get_option_param('t');
			if( file_type_ == "" ) {
				cout << "must use valid file type" << endl;
				usage();
				return false;
			}
		} else {
			cout << "must use valid file type" << endl;
			usage();
			return false;
		}
		if( opts.has_option('c') ) {
			collect_source_ = opts.get_option_param('c');
		}
		if( opts.has_option('p') ) {
			SETT_PERIOD_INS->force(opts.get_option_param('p'));
		}
		if( opts.has_option('f') ) {
			one_off_task_ = 1;
			if( !opts.has_option('t') ) {
				cout << "must use -t to appoint file type" << endl;
				usage();
				return false;
			}
			int file_count = opts.get_option_param_size('f');
			for( int i=0; i<file_count; ++i ) {
				const char* file = opts.get_option_param('f', i);
				if( !filestat(file).is_dir() ) {
					filenames_.push_back(file);
				}
			}
			return true;
		} else {
			cout << "system will run as daemon mode." << endl;
			one_off_task_ = 2;
		} 
	}
	return true;
}

void task_manager::shutdown() {
	static bool is_shutdown = false;
	if( is_shutdown==true ) {
		return;
	}
	is_shutdown=true;
	SETT_PERIOD_INS->stop();
	SETT_PERIOD_INS->wait();

	SCAN_TASK_INS->stop();
	SCAN_TASK_INS->wait();

	SETT_TASK_INS->stop();
	SETT_TASK_INS->wait();

	CYCLE_TASK_INS->stop();
	CYCLE_TASK_INS->wait();

	INDEX_TASK_INS->stop();
	INDEX_TASK_INS->wait();
	// 停止日志线程
	LOGSTREAM::fini();
}

void task_manager::usage() {
	cout << "sett2 [options]:" << endl;
	cout << "option:" << endl;
	cout << "  " << setw(15) << left << "-h" << "show this help." << endl;
	cout << "  " << setw(15) << left << "-f <file_name>" << 
	"process files with <filename>, <filename> can use wildcard(*,?)." << endl;
	cout << "  " << setw(15) << " " << "for example: a/*/b? means all folds under folder <a>," << endl;
	cout << "  " << setw(15) << " " << "with filename start with b, such b1, b2." << endl;
	cout << "  " << setw(15) << " " << "to avoid unix shell expand the wildcard, you can use quote " << endl;
	cout << "  " << setw(15) << " " << "such as \"<file_name>\"." << endl;
	cout << "  " << setw(15) << left << "-t <file_type>" << endl;
	cout << "  " << setw(15) << " " << "when using with -f, means filetype ofoneoff task," << endl;
	cout << "  " << setw(15) << " " << "or means daemon task with certain filetype," << endl;
	cout << "  " << setw(15) << left << "-c <collect_source>" << endl;
	cout << "  " << setw(15) << " " << "where file collect from, for example switch id" << endl;
	cout << "  " << setw(15) << left << "-p <force_settle_period>" << endl;
	cout << "  " << setw(15) << " " << "to force current process using certain settle period, as '200803'" << endl;
	cout << endl << "If no option signed, system will run in background (daemon mode), and not exit "
	"forever until system receviced exit command from client." << endl;
}


