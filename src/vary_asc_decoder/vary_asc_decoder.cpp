#include <string>
#include "vary_asc_decoder.h"
#include <cdr_sett.h>
#include <boost/foreach.hpp>
#include <boost_hpux_bug.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::algorithm;

vary_asc_decoder::vary_asc_decoder():next_begin_(0),cur_line_(0) {
}

vary_asc_decoder::vary_asc_decoder(vary_asc_field_define* field_define, const std::string& name):
next_begin_(0), name_(name), definition_(field_define),cur_line_(0) {
}

bool vary_asc_decoder::get_next_cdr(cdr_ex& cdr) {
	if( next_begin_==0 ) {
		next_begin_ = buf_;
	}
	if( next_begin_>=buf_+len_ ) {
		return false;
	}
	const char* p = next_begin_;
	string value;
	bool end_line = false;
	BOOST_FOREACH(int index, definition_->field_names) {
		while(!is_sep(*next_begin_, definition_->field_sep) 
			  && !(end_line=is_sep(*next_begin_, definition_->record_sep))) {
			++next_begin_;
		}
		value.assign(p, next_begin_);
		if(index!=-1) {
			cdr.set(index, value);
		}

		p = ++next_begin_;
		if (end_line){
			break;
		}
	}
	++cur_line_;

	// CDR中加入当前行
	cdr.set(F_SOURCE_OFFSET, cur_line_);

	// 跳过记录分隔符
	bool passed_one_record = false;
	while( next_begin_<(buf_+len_) ) {
		if (is_sep(*next_begin_, definition_->record_sep)){
			passed_one_record = true;
			++next_begin_;
		}else if(passed_one_record){
			break;
		}else{
			++next_begin_;
		}
	}
	return true;
}

string vary_asc_decoder::name() {
    return name_;
}

bool vary_asc_decoder::is_sep(char c, const string& all_sep) {
	const char* p_record_sep = all_sep.c_str();
	bool finded = false;
	while( *p_record_sep!='\0' ) {
		if( c==*p_record_sep ) {
			finded = true;
			break;
		}
		++p_record_sep;
	}
	return finded;
}


