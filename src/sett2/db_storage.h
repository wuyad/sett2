#ifndef __DB_STORAGE_H__
#define __DB_STORAGE_H__

#include "proc_base.h"
#include <string>
#include <list>
#include <fstream>
#include <ace/Thread_Mutex.h>
#include "db_connect.h"

struct cdrex_def_info;
class db_storage;

class cdr_indb {
public:
	bool init();
	void fini();
	bool indb(cdr_ex& cdr, proc_context& ctx, db_storage* p_storage);
private:
	database db_;
	std::map<std::string/*filetype*/, otl_stream*> streams_;

	int record_num_per_batch_;
	ACE_Thread_Mutex mutex_;
};

class db_storage : public proc_base {
protected:
	virtual bool pre_proc_file(proc_context& ctx);
	virtual bool proc_record(cdr_ex& cdr, proc_context& ctx);
	virtual bool post_proc_file(proc_context& ctx);
private:
	const std::vector<cdrex_def_info>* p_file_cfg_;
	friend class cdr_indb;
};

#define CDR_INDB_INS ACE_Singleton<cdr_indb, ACE_Thread_Mutex>::instance()

#endif // __DB_STORAGE_H__
