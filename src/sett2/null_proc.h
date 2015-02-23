#ifndef __NULL_PROC_H__
#define __NULL_PROC_H__

#include "proc_base.h"

class null_proc : public proc_base {
protected:
	virtual bool proc_record(cdr_ex& cdr, proc_context& ctx);
};
#endif // __STAT_PROC_H__
