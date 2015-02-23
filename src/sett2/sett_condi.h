#ifndef __SETT_CONDI_POOL_H__
#define __SETT_CONDI_POOL_H__

#include <wuya/object_pool.h>
#include <wuya/ace_ipc.h>
#include <cdr_sett.h>
#include <muParserInt.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <ace/Thread_Mutex.h>
#include <boost/shared_ptr.hpp>

class expr_adaptor {
public:
	void open(const std::string& expr_str);
	void close(const std::string& expr_str);
	
	int eval(cdr_ex& cdr);
private:
	mu::ParserInt parser_;
	std::vector<int> index_;
	std::list<mu::value_type> value_;
};

typedef wuya::pool_t<expr_adaptor, std::string, wuya::ace_mutex, wuya::ace_condition> expr_pool;

class sett_condition {
public:
	bool init();
	int eval(int index, cdr_ex& cdr);
	bool fit(int index, cdr_ex& cdr);
private:
	std::map<int, boost::shared_ptr<expr_pool> > pools_;

};

#define SETT_CONDI_INS ACE_Singleton<sett_condition, ACE_Thread_Mutex>::instance()

#endif // __SETT_CONDI_POOL_H__

