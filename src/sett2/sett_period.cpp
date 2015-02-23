#include "sett_period.h"
#include "db_connect.h"
#include <wuya/timespan.h>

using namespace std;
using namespace wuya;

sett_period::sett_period():force_(false), cur_str_(""){
	
}
void sett_period::close_last() {
	database db;
	if( db ) {
		string sql_str = "update from cfg_sett_period set flag=1 where "
						 "period='";
		datetime cur_datetime(cur_str_.c_str());
		int year = cur_datetime.year();
		int month = cur_datetime.month();
		if(month==1) {
			--year;
			month = 12;
		}else{
			--month;
		}
		string last_time = datetime(year, month, 1).format("%Y%m");
		sql_str += last_time;
		sql_str += "'";
		otl_cursor::direct_exec(db, sql_str.c_str(), otl_exception::disabled);
		ACE_WRITE_GUARD(ACE_RW_Thread_Mutex, __NOT_USE__, mutex_);
		periods_[last_time] = 1;
	}
}

const string& sett_period::get_current() {
	ACE_READ_GUARD_RETURN(ACE_RW_Thread_Mutex, __NOT_USE__, mutex_, cur_str_);
	return cur_str_;
}

void sett_period::force(const std::string& cur){
	ACE_WRITE_GUARD(ACE_RW_Thread_Mutex, __NOT_USE__, mutex_);
	force_ = true;
	cur_str_ = cur;
}

bool sett_period::force(){
	ACE_READ_GUARD_RETURN(ACE_RW_Thread_Mutex, __NOT_USE__, mutex_, false);
	return force_;
}

int sett_period::svc(){
	int mday = CONFIG.get<int>("last_day", "sett_period", 10);
	while( !is_stop() ) {
		// ������
		datetime today = wuya::datetime::current_time();
		if(mday!=-1) { //  �Զ�����
			if( today.day()<=mday ) { //  ���һ������Զ����ʵĽ�ֹ��
				int year = today.year();
				int month = today.month();
				int day = mday+1;
				datetime next_time(year, month, day);
				timespan span = next_time-today;
				sleep(span.get_total_seconds());
				if(is_stop()) {
					break;
				}
				// �Զ�����
				close_last();
			}
		}
		// 
		int year = today.year();
		int month = today.month();
		if(month==12) {
			++year;
			month = 1;
		}else{
			++month;
		}
		datetime next_mon(year, month, 1);
		timespan span = next_mon-today;
		sleep(span.get_total_seconds());
		{
			ACE_WRITE_GUARD_RETURN(ACE_RW_Thread_Mutex, __NOT_USE__, mutex_, -1);
			cur_str_ = next_mon.format("%Y%m");
			database db;
			if( db ) {
				// �����µ�����
				cur_str_ = wuya::datetime::current_time().format("%Y%m");
				string sql_str = "insert into cfg_sett_period(period,flag) values('";
				sql_str += cur_str_;
				sql_str += "', 0)";
				otl_cursor::direct_exec(db, sql_str.c_str(), otl_exception::disabled);
				periods_[cur_str_] = 0;
			}
		}
	}
	return 0;
}

otl_stream& operator>>(otl_stream& s, pair<string, int>& row);

bool sett_period::load_sett_period(){
	database db;
	if( db ) {
		otl_stream sql(1, "select period from cfg_sett_period where "
					   "flag=0 and rownum=1 order by period", db);
		while( !sql.eof() ) {
			sql >> cur_str_;
		}
		if( cur_str_ == "" ) {
			// ���û�м�¼�����Ե�ǰʱ��Ϊ��һ������
			wuya::datetime today = wuya::datetime::current_time();
			cur_str_ = today.format("%Y%m");
			string sql_str = "insert into cfg_sett_period(period,flag) values('";
			sql_str += cur_str_;
			sql_str += "', 0)";
			otl_cursor::direct_exec(db, sql_str.c_str(), otl_exception::disabled);
			periods_[cur_str_] = 0;
		}
		try {
			otl_stream sql(50, "select period, flag from cfg_sett_period", db);
			copy(otl_input_iterator<pair<string, int> >(sql), otl_input_iterator<pair<string, int> >(),
				 inserter(periods_, periods_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_sett_period error" << endl;
		}
	}
	return false;
}

bool sett_period::is_sett_period_closed(const std::string& cur){
	ACE_READ_GUARD_RETURN(ACE_RW_Thread_Mutex, __NOT_USE__, mutex_, false);
	return periods_[cur]==1;
}

string sett_period::parse_sett_period(cdr_ex& cdr){
	// �����ǿ������
	string sett_period_s;
	if( SETT_PERIOD_INS->force() ) {
		sett_period_s = get_current();
	}
	if( cdr.has(F_STD_BEGIN_DATETIME) && cdr.has(F_STD_DURATION) ) {
		wuya::datetime& begin = cdr.get<wuya::datetime>(F_STD_BEGIN_DATETIME);
		int duration = cdr.get<int>(F_STD_DURATION);
		wuya::datetime end = begin+wuya::timespan(duration);
		wuya::datetime end_sett(end.year(), end.month(), 1);
		string cur_period = end_sett.format("%Y%m");
		if( is_sett_period_closed(cur_period) ) {
			cur_period = get_current();
		}
		sett_period_s = cur_period;
	}
	if(sett_period_s == "") {
		sett_period_s = SETT_PERIOD_INS->get_current();
	}
	cdr.set<string>(F_CLS_SETT_PERIOD, sett_period_s);
	return sett_period_s;
}


