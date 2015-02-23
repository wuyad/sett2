#include "exclude_proc.h"
#include<string>
#include<iostream>
#include "data_cache.h"
#include "data_finder.h"
#include<boost/tokenizer.hpp>
#include "index_task.h"
#include <my_datetime.h>
#include "log.h"
#include "cdr_sett.h"
using namespace std;
using namespace boost;
using namespace wuya;

bool exclude_proc::proc_record(cdr_ex& cdr, proc_context& ctx) {

	string& filetype = ctx.get<string>(F_FILETYPE);
	string& collect_source = ctx.get<string>(F_COLLECT_SOURCE);

	const exclude_info_t &exclude_info_ = data_finder::get_exclude_info();
	exclude_info_t::const_iterator it = exclude_info_.find(filetype);
	if (it == exclude_info_.end()){
		return false;
	}
	const exclude_info& info = (*it).second;

	index_info  indexinfo;

	indexinfo.file_type = filetype;
	indexinfo.collect_source = collect_source;
	datetime& std_time = cdr.get<datetime>(F_STD_BEGIN_DATETIME);

	indexinfo.time  = std_time;
	get_keystring(info.field_name,indexinfo.query_str,cdr);

	index_task*  task = ACE_Singleton<index_task, ACE_Thread_Mutex>::instance();

	if( !task->is_unique(indexinfo) ) {
		cdr.set(F_REPEAT, 1);
	}

	return true;
}

exclude_proc::~exclude_proc() {
}

void exclude_proc::get_keystring(string field_name, String& index_string, cdr_ex& cdr) {
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep("|");
	tokenizer tokens(field_name, sep);
	char buf[256];
	int offset = 0;
	for( tokenizer::const_iterator  tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter ) {
		if( *tok_iter == "std_begin_datetime" ) {
			datetime& dt = cdr.get<datetime>(F_STD_BEGIN_DATETIME);
			string tmp = dt.time_str();
			strcpy(buf+offset, tmp.c_str());
			offset+=tmp.size();
		} else {
			string& tmp = cdr.get<string>(data_finder::get_cdrex_index(*tok_iter));
			strcpy(buf+offset, tmp.c_str());
			offset+=tmp.size();
		}
	}
	buf[offset]='\0';
	index_string = buf;
	return;

}

