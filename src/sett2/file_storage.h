#ifndef __FILE_STORAGE_H__
#define __FILE_STORAGE_H__

#include "proc_base.h"
#include "cdr_sett.h"
#include "data_finder.h"
#include <string>
#include <list>
#include <fstream>
#include <map>
#include <vector>

class file_storage : public proc_base {
public:
	file_storage();
protected:
	virtual bool pre_proc_file(proc_context& ctx);
	virtual bool proc_record(cdr_ex& cdr, proc_context& ctx);
	virtual bool post_proc_file(proc_context& ctx);
private:
	std::ofstream& get_fstream(cdr_ex& cdr);
//  std::ofstream file_;
	const std::vector<cdrex_def_info>* p_file_cfg_;
	std::string filename_;
	std::map<std::string, std::ofstream*> files_;
	std::string filetype_;
};
#endif // __FILE_STORAGE_H__
