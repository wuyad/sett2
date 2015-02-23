#include "sett_condi.h"
#include "data_finder.h"
#include "data_cache.h"
#include "log.h"

using namespace std;
using namespace mu;
using namespace boost;

bool sett_condition::init() {
	try {
		std::map<int,sett_expr_info>& exprs = data_finder::get_sett_expr();
		for( std::map<int,sett_expr_info>::iterator i = exprs.begin();
		   i != exprs.end(); ++i ) {
			int id = (*i).first;
			shared_ptr<expr_pool> ptr(new expr_pool);
			ptr->set_param((*i).second.expr);
			ptr->init(20, true);
			pools_[id] = ptr;
		}
	} catch( ParserInt::exception_type& e ) {
		cout << "expr:" << e.GetExpr() << " error" <<endl;
		return false;
	}
	return true;
}
int sett_condition::eval(int index, cdr_ex& cdr) {
	expr_adaptor& expr = pools_[index]->get_object();
	int ret =  expr.eval(cdr);
	pools_[index]->revert_object(expr);
	return ret;
}
bool sett_condition::fit(int index, cdr_ex& cdr) {
	expr_adaptor& expr = pools_[index]->get_object();
	int ret =  expr.eval(cdr);
	pools_[index]->revert_object(expr);
	return ret!=0;
}

// expr_adaptor -----------------------------------------------------------
void expr_adaptor::open(const std::string& expr_str) {
	for( list<expr_const_info>::iterator j=data_finder::get_expr_const().begin();
	   j != data_finder::get_expr_const().end(); ++j ) {
		parser_.DefineConst(j->name.c_str(), j->value);
	}
	parser_.SetExpr(expr_str);
	mu::varmap_type vars = parser_.GetUsedVar();
	mu::varmap_type::const_iterator it = vars.begin();
	for( ;it!=vars.end(); ++it ) {
		const string& var_name=(*it).first;
		string var_name2 = "cls_";
		const char* name = var_name.c_str();
		int index=0;
		if( strncmp(name, "id_", 3)==0 ) {
			index = -atoi(name+3);
		}else{
			var_name2 += var_name;
			index = data_finder::get_cdrex_index(var_name2);
		}
		index_.push_back(index);
		value_.push_back(0);
		parser_.DefineVar((*it).first, &value_.back());
	}
}
void expr_adaptor::close(const std::string& expr_str) {
}
struct tmp_var {
	typedef list<mu::value_type> value_t;
	value_t v;
	cdr_ex* cdr;
};
int expr_adaptor::eval(cdr_ex& cdr) {
	try {
//  	mu::varmap_type::const_iterator it = parser_.GetVar().begin();
//  	for( ;it!=parser_.GetVar().end(); ++it ) {
//  		const string& s_name = (*it).first;
//  		const char* name = s_name.c_str();
//  		if( strncmp(name, "id_", 3)==0 ) {
//  			vars_[s_name] = SETT_CONDI_INS->eval(atoi(name+3), cdr);
//  		} else {
//  			char buf[40];
//  			sprintf(buf, "cls_%s", name);
//  			vars_[s_name] = cdr.get<int>(buf);
//  		}
//  	}
		int x=0;
		for(std::list<mu::value_type>::iterator i= value_.begin(); i!=value_.end(); ++i,++x) {
			int& index = index_[x];
			mu::value_type& value = *i;
			if(index<0) {
				value = SETT_CONDI_INS->eval(-index, cdr);
			}else{
				value = cdr.get<int>(index);
			}
		}
		int ret = (int)parser_.Eval();
		return ret;
	} catch( ParserInt::exception_type &e ) {
		logerr << "expr eval error: " << e.GetMsg() << endl;
		return 0;
	}
}




