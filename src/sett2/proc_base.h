#ifndef __PROC_BASE_H__
#define __PROC_BASE_H__

#include <list>
#include <hash/hash_map.h>
#include "cdr_sett.h"
#include "file_log.h"

typedef cdr_ex_base proc_context;

class proc_base {
public:
	enum PROC_TYPE{
		NORMAL_PROC, STORAGE_PROC
	};
	proc_base();
	proc_base(const std::string& name);
	virtual ~proc_base()=0;

	void set_name(const std::string& name);
	std::string& get_name();
	
	void set_prov_name(const std::string& name);
	std::string& get_prov_name();
	
	void set_proc_type(PROC_TYPE type);
	PROC_TYPE get_proc_type();
	
	bool do_pre_proc_file(proc_context& ctx);
	bool do_post_proc_file(proc_context& ctx);

	bool do_proc_record(cdr_ex& cdr, proc_context& ctx);
	void set_proc_cdr_type(CDR_TYPE limit_cdr_type);
protected:
	virtual bool proc_record(cdr_ex& cdr, proc_context& ctx)=0;
	virtual bool pre_proc_file(proc_context& ctx);
	virtual bool post_proc_file(proc_context& ctx);
	
	CDR_TYPE proc_cdr_type_;
	std::string name_;
	std::string prov_name_;
	file_log_info log_;
	PROC_TYPE proc_type_;
};

#endif // __PROC_BASE_H__

