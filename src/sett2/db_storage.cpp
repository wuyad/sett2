#include "db_storage.h"
#include "sett_period.h"
#include "data_finder.h"
#include "data_cache.h"
#include "log.h"
#include "cdr_sett.h"
#include <boost_hpux_bug.h>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>
#include <wuya/filestat.h>
#include "my_datetime.h"
#include <iomanip>
#include <sstream>
using namespace std;
using namespace boost;
using namespace wuya;

class otl_stream_visitor
: public boost::static_visitor<> {
public:
	otl_stream_visitor(otl_stream& sql, int len):sql_(sql), len_(len) {
	}
	void operator()(int & i) const {
		sql_ << i;
	}
	void operator()(long long & i) const {
		sql_ << (OTL_BIGINT)i;
	}
	void operator()(std::string& s) const {
		if( s.size() > (size_t)len_ ) {
			s.erase(len_);
		}
		sql_ << s;
	}
	void operator()(wuya::datetime& d) const {
		otl_datetime tm;
		tm.year=d.year();
		tm.month=d.month();
		tm.day=d.day();
		tm.hour=d.hour();
		tm.minute=d.minute();
		tm.second=d.second();
		sql_ << tm;
	}
	void operator()(telno& t) const {
		char buf[MAX_TELNO_LEN];
		memset(buf,0, MAX_TELNO_LEN);
		int offset=0;
		size_t l = strlen(t.net_code);
		if( l!=0 ) {
			sprintf(buf+offset, "%s", t.net_code);
			offset += l;
		}
		l = strlen(t.country_code);
		if( l!=0 ) {
			sprintf(buf+offset, "%s", t.country_code);
			offset += l;
		}
		l = strlen(t.area_code);
		if( l!=0 ) {
			sprintf(buf+offset, "%s", t.area_code);
			offset += l;
		}
		l = strlen(t.user_code);
		if( l!=0 ) {
			sprintf(buf+offset, "%s", t.user_code);
			offset += l;
		}
		buf[len_] = '\0';
		sql_ << buf;
	}
private:
	otl_stream& sql_;
	int len_;
};

bool cdr_indb::init() {
	record_num_per_batch_ = data_finder::get_db_storage_record_num_per_batch();
	if( record_num_per_batch_==0 ) {
		record_num_per_batch_ = 5000;
	}
	return true;
}

void cdr_indb::fini() {
	typedef std::map<string/*filetype*/, otl_stream*> stream_t;

	BOOST_FOREACH(stream_t::reference k, streams_) {
		k.second->flush();
		delete k.second;
		k.second = 0;
	}
}

bool cdr_indb::indb(cdr_ex& cdr, proc_context& ctx, db_storage* p_storage) {
	if( p_storage->p_file_cfg_==0 ) {
		return false;
	}
	string& s_sett_period = cdr.get<string>(F_CLS_SETT_PERIOD);
	if( s_sett_period=="" ) {
		s_sett_period = SETT_PERIOD_INS->parse_sett_period(cdr);
	}
	// 当前帐期有没有otl_stream对象实例
	ACE_GUARD_RETURN(ACE_Thread_Mutex, __NOT_USE__, mutex_, false);
	otl_stream* p_sql;
	string& filetype = ctx.get<string>(F_FILETYPE);
	ostringstream os;
	os << filetype << "," << s_sett_period << "," << p_storage->proc_cdr_type_ << "," << p_storage->get_name();
	map<std::string, otl_stream*>::iterator i=streams_.find(os.str());
	if( i==streams_.end() ) {
		// 构建表名，帐期暂不解析
		string tablename = data_finder::get_db_storage_table_name_policy(p_storage->proc_cdr_type_);
		boost::replace_all(tablename, "$FILETYPE", filetype);
		boost::replace_all(tablename, "$PROC", p_storage->get_name());
		boost::replace_all(tablename, "$MONTH", s_sett_period);
		string::size_type i;
		while( (i = tablename.find_first_of(". /\\")) != string::npos ) {
			tablename.replace(i, 1, 1, '_');
		}
		i = tablename.find("_proc");
		if( i!=string::npos ) {
			tablename.erase(i, 5);
		}
		boost::trim(tablename);
		// 创建表
		{
			ostringstream os;
			os << "create table " << tablename << " (";
			bool first = true;
			BOOST_FOREACH(const cdrex_def_info& i, *p_storage->p_file_cfg_) {
				if( first ) {
					first = false;
				} else {
					os << ", ";
				}
				os << i.name << " ";
				switch( i.type ) {
				case 0:// int
					os << "number(10)";
					break;
				case 1:// long long
					os << "number(20)";
					break;
				case 2:// string
					os << "VARCHAR2(" << i.length << ")";
					break;
				case 3:// date
					os << "date";
					break;
				case 4:// telno
					os << "VARCHAR2(" << i.length << ")";
					break;
				}
			}
			os << ")";
			try {
				otl_cursor::direct_exec(db_, os.str().c_str());
			} catch( otl_exception& e ) {
				if( e.code==955 ) {
//  				loginfo << "create table (" << os.str() << ") fail, table existed" << endl;
				} else {
					logerr << "create table (" << os.str() << ") fail" << endl;
					return false;
				}
			}
		}

		// 创建插入sql
		{
			ostringstream os2;
			os2 << "insert into " << tablename << " (";
			bool first = true;
			BOOST_FOREACH(const cdrex_def_info& i, *p_storage->p_file_cfg_) {
				if( first ) {
					first = false;
					os2 << i.name;
				} else {
					os2 << ", " << i.name;
				}
			}
			os2 << ") values (";
			int index=0;
			first = true;
			BOOST_FOREACH(const cdrex_def_info& i, *p_storage->p_file_cfg_) {
				if( first ) {
					first = false;
				} else {
					os2 << ", ";
				}
				switch( i.type ) {
				case 0:// int
					os2 << ":f" << index++ << "<int>";
					break;
				case 1:// long long
					os2 << ":f" << index++ << "<bigint>";
					break;
				case 2:// string
					os2 << ":f" << index++ << "<char[" << i.length+1 <<"]>";
					break;
				case 3:// date
					os2 << ":f" << index++ << "<timestamp>";
					break;
				case 4:// telno
					os2 << ":f" << index++ << "<char[" << i.length+1 <<"]>";
					break;
				}
			}
			os2 << ")";

			try {
				p_sql = new otl_stream(record_num_per_batch_, os2.str().c_str(), db_);
			} catch( ... ) {
				logerr << "insert cdr (" << os2.str() << ") fail" << endl;
				return false;
			}
			streams_[os.str()] = p_sql;
		} 
	} else {
		p_sql = (*i).second;
	}
	// 入库
	try {
		BOOST_FOREACH(const cdrex_def_info& i, *p_storage->p_file_cfg_) {
			if(cdr.has(i.index)) {
				boost::apply_visitor(otl_stream_visitor(*p_sql, i.length) , cdr.get_variant(i.index));
			}else{
				*p_sql << otl_null();
			}
		}
	}catch(otl_exception& e){
		logerr << e.msg << endl;
		return false;
	}
	return true;
}

bool db_storage::proc_record(cdr_ex& cdr, proc_context& ctx) {
	if( p_file_cfg_ ) {
		return CDR_INDB_INS->indb(cdr, ctx, this);
	} else {
		return false;
	}
}

bool db_storage::post_proc_file(proc_context& ctx) {
	log_.procname += "_db";
	switch( proc_cdr_type_ ) {
	case NORMAL_CDR:
		log_.procname+="_normal";break;
	case ERROR_CDR:
		log_.procname+="_error";break;
	case REPEAT_CDR:
		log_.procname+="_repeat";break;
	}
	return true;
}

bool db_storage::pre_proc_file(proc_context& ctx) {
	const vector<cdrex_def_info>& file_cfg = data_finder::get_cdrex_indb(ctx.get<string>(F_FILETYPE));
	p_file_cfg_ = 0;
	if( file_cfg.size()==0 ) {
		logerr << "db storeage fail case file type " << ctx.get<string>(F_FILETYPE) << 
		" not config in table cfg_file_type_info" << endl;
		return false;
	}
	p_file_cfg_ = &file_cfg;

	return true;

}

