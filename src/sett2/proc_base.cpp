#include "proc_base.h"
#include "data_finder.h"
using namespace std;
using namespace boost;

proc_base::proc_base():proc_cdr_type_(NORMAL_CDR),proc_type_(NORMAL_PROC) {
}
proc_base::proc_base(const std::string& name):proc_cdr_type_(NORMAL_CDR),name_(name),
proc_type_(NORMAL_PROC) {
	log_.procname = name;
}
proc_base::~proc_base() {
}
void proc_base::set_name(const std::string& name) {
	name_ = name;
	log_.procname = name;
}
std::string& proc_base::get_name() {
	return name_;
}

void proc_base::set_prov_name(const std::string& name) {
	prov_name_ = name;
	log_.pre_procname = name;
}
std::string& proc_base::get_prov_name() {
	return prov_name_;
}

void proc_base::set_proc_type(PROC_TYPE type) {
	proc_type_ = type;
}
proc_base::PROC_TYPE proc_base::get_proc_type() {
	return proc_type_;
}

bool proc_base::do_pre_proc_file(proc_context& ctx) {
	return pre_proc_file(ctx);
}
bool proc_base::do_post_proc_file(proc_context& ctx) {
	bool ret =  post_proc_file(ctx);
	// 写文件处理日志
	log_.filetype = ctx.get<string>(F_FILETYPE);
	log_.pathname = ctx.get<string>(F_FILEPATH);
	log_.filename = ctx.get<string>(F_FILENAME);
	log_.filesize = ctx.get<int>(F_FILESIZE);
	log_.proc_cdr_type = proc_cdr_type_;
	log_.normal_num = log_.record_num-log_.error_num-log_.repeat_num;
	FILE_LOG_INS->log(log_);
	return ret;
}

bool proc_base::do_proc_record(cdr_ex& cdr, proc_context& ctx) {
	bool ret = false;
	if( cdr.get<int>(F_REPEAT) > 0 ) {
		if( proc_cdr_type_==REPEAT_CDR ) {
			++log_.record_num;
			ret = proc_record(cdr, ctx);
		}
	} else if( cdr.get<int>(F_ERROR) > 0 ) {
		if( proc_cdr_type_==ERROR_CDR ) {
			++log_.record_num;
			ret = proc_record(cdr, ctx);
		}
	} else {
		if( proc_cdr_type_==NORMAL_CDR ) {
			++log_.record_num;
			ret = proc_record(cdr, ctx);
			if( cdr.get<int>(F_ERROR) > 0) {
				++log_.error_num;
			} else if( cdr.get<int>(F_REPEAT) > 0 ) {
				++log_.repeat_num;
			}
		}
	}
	return ret;
}
void proc_base::set_proc_cdr_type(CDR_TYPE limit_cdr_type) {
	proc_cdr_type_ = limit_cdr_type;
}
bool proc_base::pre_proc_file(proc_context& ctx) {
	return true;
}
bool proc_base::post_proc_file(proc_context& ctx) {
	return true;
}


