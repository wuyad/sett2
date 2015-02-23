#include <string>
#include "fix_asc_decoder.h"
#include <cdr_sett.h>
#include <boost/foreach.hpp>
#include <boost_hpux_bug.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::algorithm;

fix_asc_decoder::fix_asc_decoder():next_begin_(0),cur_line_(0){
}

fix_asc_decoder::fix_asc_decoder(vector<fix_asc_field_define>* field_define, const std::string& name):
    next_begin_(0), name_(name), definition_(field_define),cur_line_(0){
}

bool fix_asc_decoder::get_next_cdr(cdr_ex& cdr) {
    if(next_begin_==0) {
        next_begin_ = buf_;
    }
    if(next_begin_>=buf_+len_) {
        return false;
    }
    string value;
    BOOST_FOREACH(fix_asc_field_define& def, *definition_){
		const char* e = next_begin_+def.offset+def.length;
		if (e > buf_+len_){
			e = buf_+len_;
		}
        value.assign(next_begin_+def.offset, e);
        if(def.left_justify) {
            trim_right_if(value, bind1st(equal_to<char>(), def.filler));
        }else{
            trim_left_if(value, bind1st(equal_to<char>(), def.filler));
		}
		if(def.index!=-1) {
//  		cdr[def.name] = value;
			cdr.set(def.index, value);
		}
    }
    ++cur_line_;

    // CDR中加入当前行
	cdr.set(F_SOURCE_OFFSET, cur_line_);
	
    while(next_begin_<(buf_+len_) && (*next_begin_!='\r' && *next_begin_!='\n')) {
        ++next_begin_;
    }
    while(next_begin_<(buf_+len_) && (*next_begin_=='\r' || *next_begin_=='\n')) {
        ++next_begin_;
    }
    return true;
}

string fix_asc_decoder::name() {
    return name_;
}


