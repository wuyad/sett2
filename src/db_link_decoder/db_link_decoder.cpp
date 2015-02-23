#include "db_link_decoder.h"
#include <string>
#include <cdr_sett.h>
#include <boost/foreach.hpp>
#include <boost_hpux_bug.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::algorithm;

db_link_decoder::db_link_decoder():cur_line_(0) {
}

db_link_decoder::db_link_decoder(std::vector<int>* field_define, const std::string& name):
name_(name), definition_(field_define),cur_line_(0) {
}

bool db_link_decoder::get_next_cdr(cdr_ex& cdr) {
	if( cur_line_==0 ) {
		vector<string> v;
		string sv(buf_, len_);
		boost::split(v, sv, bind1st(equal_to<char>(),':'));
		if( v.size()!=2 ) {
			return false;
		}
		try {
			db_.rlogon(v[0].c_str());
			v[1].insert(0, "select * from ");
			otl_stream sql;
			sql.open(1, v[1].c_str(), db_);
			otl_column_desc* ocd = sql.describe_select(n_cols_);
			if( n_cols_<=0 ) {
				return false;
			}
			for(int i=0; i<n_cols_; ++i) {
				if(ocd[i].otl_var_dbtype != otl_var_char) {
					sql_.set_column_type(i+1, otl_var_char, 40);
				}
			}
			sql.close();
			sql_.open(50, v[1].c_str(), db_);
			fields_.resize(n_cols_);
		} catch( ... ) {
			return false;
		}
	}
	if( sql_.eof() ) {
		return false;
	}
	try {
		for( int i=0; i<n_cols_; ++i ) {
			sql_ >> fields_[i];
		}
	} catch( otl_exception& p ) {
		p;
		return false;
	}
	int i=0;
	BOOST_FOREACH(int field_name, *definition_) {
		if(field_name!=-1) {
			cdr.set(field_name, fields_[i++]);
		}
		if( i>=n_cols_ ) {
			break;
		}
	}
	++cur_line_;

	// CDR中加入当前行
	cdr.set(F_SOURCE_OFFSET, cur_line_);

	return true;
}

string db_link_decoder::name() {
	return name_;
}


