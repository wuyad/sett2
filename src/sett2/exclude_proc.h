#ifndef __EXCLUDE_PROC_H__
#define __EXCLUDE_PROC_H__

#include "proc_base.h"
#include "index_task.h"



class exclude_proc : public proc_base {
protected:
	bool proc_record(cdr_ex& cdr, proc_context& ctx);
    ~exclude_proc();
private:
	void get_keystring(std::string field_name,String& index_string,cdr_ex& cdr);
	

};
#endif // __EXCLUDE_PROC_H__
