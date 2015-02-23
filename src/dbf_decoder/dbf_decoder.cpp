#include <string>
#include "dbf_decoder.h"
#include "from_little_endian.h"
#include <cdr_sett.h>
#include <boost/foreach.hpp>
#include <boost_hpux_bug.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::algorithm;

#define HEAD_TERMINATOR (char)0xD
#define FILE_TERMINATOR (char)0x1A
#define DELETE_FLAG '*'


dbf_decoder::dbf_decoder():total_record_(-1),cur_record_(0),bytes_of_head_(0),bytes_per_record_(0){
}

dbf_decoder::dbf_decoder(vector<int>* field_define, const std::string& name):
    total_record_(-1), name_(name), definition_(field_define),cur_record_(0),
	bytes_of_head_(0),bytes_per_record_(0){
}

bool dbf_decoder::parse_dbf_head(){
	total_record_ = get_long_le(buf_+4); // 4-7 bit 总记录数
	if (total_record_ == 0){
		return false;
	}
	bytes_of_head_ = get_short_le(buf_+8); // 8-9 bit 文件头的长度
	bytes_per_record_ = get_short_le(buf_+10); // 10-11 bit 每条记录的字节数
	const char* p = buf_+32;
	while(true) {
		if (*p == HEAD_TERMINATOR){
			break;
		}
		dbf_item_define tmp;
		tmp.name.assign(p, 11);
		tmp.type = *(p+11);
		tmp.offset = get_long_le(p+12);
		tmp.length = *(p+16);
		dbf_item_define_.push_back(tmp);
		p+=32;
	}
	return true;
}

bool dbf_decoder::get_next_cdr(cdr_ex& cdr) {
	if(total_record_==-1) {
		if(parse_dbf_head() == false) {
			return false;
		}
	}
	const char* record_begin = buf_+bytes_of_head_+bytes_per_record_*cur_record_;
	// 跳过已删除记录
	while(*record_begin==DELETE_FLAG) {
		++cur_record_;
		record_begin = buf_+bytes_of_head_+bytes_per_record_*cur_record_;
		if(cur_record_>=total_record_ || *record_begin==FILE_TERMINATOR) {
			return false;
		}
	}
	if(cur_record_>=total_record_ || *record_begin==FILE_TERMINATOR) {
		return false;
	}
	vector<dbf_item_define>::const_iterator it = dbf_item_define_.begin();
	BOOST_FOREACH(int item, *definition_){
		if(it!=dbf_item_define_.end()) {
			string tmp(record_begin+it->offset, it->length);
			trim(tmp);
			if(item!=-1) {
//  			cdr[item] = tmp;
				cdr.set(item, tmp);
			}
			++it;
		}else{
			break;
		}
	}
    // CDR中加入当前行
	cdr.set(F_SOURCE_OFFSET, cur_record_);
	++cur_record_;
    return true;
}

string dbf_decoder::name() {
    return name_;
}


