#include "file_storage.h"
#include "data_cache.h"
#include "log.h"
#include "sett_period.h"
#include <boost_hpux_bug.h>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <wuya/filestat.h>
#include <wuya/fileopt.h>

using namespace std;
using namespace wuya;
using namespace boost;

bool file_storage::proc_record(cdr_ex& cdr, proc_context& ctx) {
	if( p_file_cfg_->size()==0 ) {
		get_fstream(cdr) << cdr;
		return true;
	}
	ofstream& file = get_fstream(cdr);
	file << left;
	BOOST_FOREACH(const cdrex_def_info& i, *p_file_cfg_) {
		if (cdr.has(i.index)){
  		file << setw(i.length) << cdr.get_variant(i.index);
		}else{
			file << setw(i.length) << "";
		}
	}
	file << endl;
	return true;
}

file_storage::file_storage():p_file_cfg_(0) {
}

bool file_storage::pre_proc_file(proc_context& ctx) {
	ostringstream os;
	os << data_finder::get_file_storage_root_path() << FILE_SEP;
	string path_name_policy = data_finder::get_file_storage_path_name_policy(proc_cdr_type_);
	filetype_ = ctx.get<string>(F_FILETYPE);
	boost::replace_all(path_name_policy, "$PROC", get_name());
	boost::replace_all(path_name_policy, "$FILETYPE", filetype_);
	boost::trim(path_name_policy);
	os << path_name_policy << FILE_SEP;

	int n_path = data_finder::get_file_storage_keep_last_n_subpath();
	string& file_name = ctx.get<string>(F_FULL_FILENAME);
	const char* p = file_name.c_str()-1;
	const char* q = file_name.c_str()+file_name.size()-1;
	while( q>p&&n_path>=0 ) {
		if( *q==FILE_SEP ) {
			--n_path;
		}
		--q;
	}
	++q;
	while( *q==FILE_SEP || *q==':' ) {
		++q;
	}
	os << q;

	filename_ = os.str();
	
	string::size_type i;
	i = filename_.find("_proc");
	if( i!=string::npos ) {
		filename_.erase(i, 5);
	}
	
	p_file_cfg_ = &data_finder::get_cdrex_indb(filetype_);
	return true;
}

bool file_storage::post_proc_file(proc_context& ctx) {
	log_.procname += "_file";
	switch( proc_cdr_type_ ) {
	case NORMAL_CDR:
		log_.procname+="_normal";break;
	case ERROR_CDR:
		log_.procname+="_error";break;
	case REPEAT_CDR:
		log_.procname+="_repeat";break;
	}

	typedef std::map<std::string, std::ofstream*>::reference ref_t;
	BOOST_FOREACH(ref_t r, files_) {
		r.second->close();
		filestat fs(r.first.c_str());
		if( fs.length()==0 ) {
			wuya::remove_file(filename_.c_str());
		}
		delete r.second;
	}
	return true;
}

std::ofstream& file_storage::get_fstream(cdr_ex& cdr) {
	string& s_sett_period = cdr.get<string>(F_CLS_SETT_PERIOD);
	if(s_sett_period=="") {
		s_sett_period = SETT_PERIOD_INS->parse_sett_period(cdr);
	}
	// 第一次打开
	string filename = filename_;
	boost::replace_all(filename, "$MONTH", s_sett_period);
	
	ostringstream os;
	os << filetype_ << "," << s_sett_period << "," << proc_cdr_type_ << "," << get_name();

	map<string, ofstream*>::iterator i = files_.find(os.str());
	if(i!=files_.end()) {
		return *(*i).second;
	}else{
		filestat fs(filename.c_str());
		wuya::make_dir(fs.get_filepath());
		ofstream* of = new ofstream(filename.c_str());
		files_[os.str()] = of;
		return *of;
	}
}


