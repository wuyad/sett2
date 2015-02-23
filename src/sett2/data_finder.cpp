#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include "data_finder.h"
#include "system_define.h"
#include "data_cache.h"
#include "cdr_sett.h"
#include <my_datetime.h>

using namespace std;
using namespace wuya;
typedef std::map<string, string> string_pair;

int data_finder::get_max_sett_thread() {
	return CONFIG.get<int>("max_sett_thread", "thread", 1);
}

const char* data_finder::get_ftp_default_local_root() {
	return  CONFIG.get<const char*>("default_local_root", "ftp", 0);
}
int data_finder::get_ftp_keep_last_n_subpath() {
	return  CONFIG.get<int>("keep_last_n_subpath", "ftp", 0);
}

std::map<string,file_source_info>& data_finder::get_file_source_info() {
	return DATA_CACHE->file_source_info_;
}

decoder_ptr data_finder::create_decoder(const string& name) {
	return DATA_CACHE->decoder_manager_.create_decoder(name);
}

bool data_finder::is_valid_proc_step(const char* name) {
	std::set<string>& c = DATA_CACHE->proc_step_;
	return(c.find(name) != c.end());
}
bool data_finder::is_valid_storage_type(const char* name) {
	std::set<string>& c = DATA_CACHE->storage_type_;
	return(c.find(name) != c.end());
}

const char* data_finder::get_workflow(const char* name) {
	return DATA_CACHE->workflow_[name].c_str();
}

const char* data_finder::get_file_storage_root_path() {
	const char* p = CONFIG.get<const char*>("root_path", "file_storage");
	if( p==0 || strcmp(p, "")==0 ) {
		return ".";
	} else {
		return p;
	}
}

const char* data_finder::get_file_storage_path_name_policy(int cdr_type) {
	const char* key = "path_name_policy";
	if( cdr_type==1 ) {
		key = "path_name_policy_err";
	} else if( cdr_type==2 ) {
		key = "path_name_policy_repeat";
	} else if( cdr_type==3 ) {
		key = "path_name_policy_indb_fail";
	}
	return CONFIG.get<const char*>(key, "file_storage");
}

const char* data_finder::get_db_storage_table_name_policy(int cdr_type) {
	const char* key = "table_name";
	if( cdr_type==1 ) {
		key = "table_name_error";
	} else if( cdr_type==2 ) {
		key = "table_name_repeat";
	}
	return CONFIG.get<const char*>(key, "db_storage");
}

int data_finder::get_db_storage_record_num_per_batch() {
	return CONFIG.get<int>("record_num_per_batch", "db_storage");
}

int data_finder::get_file_storage_keep_last_n_subpath() {
	return CONFIG.get<int>("keep_last_n_subpath", "file_storage");
}

bool data_finder::is_valid_country_id(const char* country_id) {
	if( country_id==0 ) {
		return false;
	}
	if( *country_id=='\0' ) {
		++country_id;
	}
	if( *country_id=='\0' ) {
		++country_id;
	}
	return DATA_CACHE->country_id_.find(country_id)!=DATA_CACHE->country_id_.end();
}

const service_telno_info_t::value_type* data_finder::get_special_number(const char* special_number) {
	if( special_number==0 ) {
		return false;
	}
	std::map<string, service_telno>::const_iterator it = DATA_CACHE->service_telno_.upper_bound(special_number);
	reverse_iterator<std::map<string, service_telno>::const_iterator> i(it);
	reverse_iterator<std::map<string, service_telno>::const_iterator> i_end(DATA_CACHE->service_telno_.begin());
	for( ;i!=i_end;++i ) {
		service_telno_info_t::const_reference r = *i;
		if( strncmp(special_number, r.first.c_str(), r.first.length()) == 0 ) {
			return &r;
		}
		if( r.first.length() <= DATA_CACHE->min_service_telno_len_ ) {
			return 0;
		}
	}
	return 0;
}

bool data_finder::is_special_number(const char* special_number) {
	if( special_number==0 ) {
		return false;
	}
	std::map<string, service_telno>::const_iterator it = DATA_CACHE->service_telno_.find(special_number);
	if( it!=DATA_CACHE->service_telno_.end() ) {
		return true;
	}
	return false;
}

bool data_finder::is_valid_area_code(const char* area_code) {
	if( area_code==0 ) {
		return false;
	}
	return DATA_CACHE->area_code_.find((area_code))!=DATA_CACHE->area_code_.end();
}

int data_finder::get_prov_from_area_code(int area_code) {
	char buf[8];
	sprintf(buf, "%d", area_code);
	std::map<string, int>::const_iterator it;
	if( (it=DATA_CACHE->area_code_.find(buf))!=DATA_CACHE->area_code_.end() ) {
		return(*it).second;
	}
	return 0;
}

const char* data_finder::get_cicno(const char* telno) {
	if( telno==0 ) {
		return 0;
	}
	std::set<string>::const_iterator it = DATA_CACHE->cicno_profix_.upper_bound(telno);
	reverse_iterator<std::set<string>::const_iterator> i(it);
	reverse_iterator<std::set<string>::const_iterator> i_end(DATA_CACHE->cicno_profix_.begin());
	for( ;i!=i_end;++i ) {
		if( strncmp(telno, i->c_str(), i->length()) == 0 ) {
			return i->c_str();
		}
		if( i->length() <= DATA_CACHE->min_cicno_len_ ) {
			return 0;
		}
	}
	return 0;
}

const cicno_info_t::value_type* data_finder::get_cicno_info(const telno& no) {
	string key;
	key = CONFIG.get<const char*>("area_code", "me", 0);
	key+=no.net_code;
	std::map<string, cicno_info>::const_iterator it = DATA_CACHE->cicno_.upper_bound(key);
	reverse_iterator<std::map<string, cicno_info>::const_iterator> i(it);
	reverse_iterator<std::map<string, cicno_info>::const_iterator> i_end(DATA_CACHE->cicno_.begin());
	for( ;i!=i_end;++i ) {
		if( strncmp(key.c_str(), (*i).first.c_str(), (*i).first.length()) == 0 ) {
			return &(*i);
		}
		if( (*i).first.length() <= DATA_CACHE->min_cicno_profix_len_ ) {
			return 0;
		}
	}
	return 0;
}

const hcode_info* data_finder::get_mobile_info(const char* mobile_no) {
	if( mobile_no==0 || strlen(mobile_no)<6 ) {
		return 0;
	}
	hcode_info_t::const_iterator it=DATA_CACHE->hcode_.upper_bound(mobile_no);
	if( it!=DATA_CACHE->hcode_.begin() ) {
		--it;
		if( strncmp(
				   (*it).first.c_str(), 
				   mobile_no, 
				   (*it).first.length() 
				   )<=0 
			&& strncmp(
					  (*it).second.end_no.c_str(), 
					  mobile_no, 
					  (*it).second.end_no.length()
					  )>=0 ) {
			return &(*it).second;
		}
	}
	return 0;
}

const spcode_info* data_finder::get_sp_id(const char* user_code) {
	if( user_code==0 ) {
		return 0;
	}
	spcode_info_t::const_iterator it=DATA_CACHE->spcode_.upper_bound(user_code);
	if( it!=DATA_CACHE->spcode_.begin() ) {
		--it;
		if( strncmp(
				   (*it).first.c_str(), 
				   user_code, 
				   (*it).first.length() 
				   )<=0 
			&& strncmp(
					  (*it).second.end_no.c_str(), 
					  user_code, 
					  (*it).second.end_no.length()
					  )>=0 ) {
			return &(*it).second;
		}
	}
	return 0;
}

const string& data_finder::get_sp_name(int id) {
	return DATA_CACHE->sp_ids_[id];
}

vector<telno_rule>& data_finder::get_caller_rule() {
	return DATA_CACHE->callerno_rule_;
}

vector<telno_rule>& data_finder::get_called_rule() {
	return DATA_CACHE->calledno_rule_;
}

const exclude_info_t&  data_finder::get_exclude_info() {
	return  DATA_CACHE->exclude_info_;
}

int data_finder::get_cdrex_index(const std::string& name) {
	std::map<std::string, int>::iterator i = DATA_CACHE->cdrex_index_.find(name);
	if( i == DATA_CACHE->cdrex_index_.end() ) {
		return -1;
	}
	return(*i).second;
}

const string& data_finder::get_cdrex_name(int index) {
	return get_cdrex_info(index)->name;
}

const cdrex_def_info* data_finder::get_cdrex_info(int index) {
	std::map<int, cdrex_def_info>::iterator i = DATA_CACHE->cdrex_def_.find(index);
	if( i == DATA_CACHE->cdrex_def_.end() ) {
		return 0;
	}
	return &(*i).second;
}

const std::map<std::string, int>& data_finder::get_cdrex_all_index() {
	return DATA_CACHE->cdrex_index_;
}

const std::vector<cdrex_def_info>& data_finder::get_cdrex_indb(const std::string& filetype) {
	return DATA_CACHE->cdrex_indb_[filetype];
}

std::map<int, sett_expr_info>& data_finder::get_sett_expr() {
	return DATA_CACHE->sett_expr_;
}

list<expr_const_info>& data_finder::get_expr_const() {
	return DATA_CACHE->expr_const_;
}

std::set<int>& data_finder::get_report_info(const string&  tablename) {
	return DATA_CACHE->report_info_[tablename];
}

bool data_finder::itisme(int carr_id) {
	return  CONFIG.get<int>("id", "me", 0)==carr_id;
}

bool data_finder::is_local_province(int area_code) {
	return get_prov_from_area_code(area_code) == 
	get_prov_from_area_code(CONFIG.get<int>("area_code", "me", -1));
}

bool data_finder::is_local_ac(int area_code) {
	return(area_code==CONFIG.get<int>("area_code", "me", -1));
}

int data_finder::get_cur_carr_id() {
	return CONFIG.get<int>("id", "me", 0);
}
int data_finder::get_cur_province() {
	return get_prov_from_area_code(CONFIG.get<int>("area_code", "me", -1)); 
}
int data_finder::get_cur_ac() {
	return CONFIG.get<int>("area_code", "me", -1);
}

const telno_segment_info* data_finder::get_fix_term_info(const telno& no) {
	string user_no;
	if( strcmp(no.area_code, "") == 0 ) {
		user_no = CONFIG.get<const char*>("area_code", "me", 0);
	} else {
		user_no = no.area_code;
	}
	user_no += no.user_code;
	std::map<string, telno_segment_info>::const_iterator it = DATA_CACHE->telno_segment_.upper_bound(user_no);
	reverse_iterator<std::map<string, telno_segment_info>::const_iterator> i(it);
	reverse_iterator<std::map<string, telno_segment_info>::const_iterator> i_end(DATA_CACHE->telno_segment_.begin());
	for( ;i!=i_end;++i ) {
		if( strncmp(user_no.c_str(), (*i).first.c_str(), (*i).first.length()) == 0 ) {
			return &(*i).second;
		}
		if( (*i).first.length() <= DATA_CACHE->min_telno_segment_len_ ) {
			return 0;
		}
	}
	return 0;
}

const cell_prefix_info* data_finder::get_cell_prefix_info(const char* user_code) {
	std::map<string, cell_prefix_info>::const_iterator it = DATA_CACHE->cellno_prefix_.upper_bound(user_code);
	reverse_iterator<std::map<string, cell_prefix_info>::const_iterator> i(it);
	reverse_iterator<std::map<string, cell_prefix_info>::const_iterator> i_end(DATA_CACHE->cellno_prefix_.begin());
	for( ;i!=i_end;++i ) {
		if( strncmp(user_code, (*i).first.c_str(), (*i).first.length()) == 0 ) {
			return &(*i).second;
		}
		if( (*i).first.length() <= DATA_CACHE->min_cellno_prefix_len_ ) {
			return 0;
		}
	}
	return 0;
}

const trunk_info* data_finder::get_trunk_info(const char* area_code, const char* collect_source, const char* trunk) {
	if( area_code==0 || strcmp(area_code, "")==0 ) {
		area_code = CONFIG.get<const char*>("area_code", "me", 0);
	}
	string key = area_code;
	key += collect_source;
	key += trunk;
	std::map<string, trunk_info>::iterator i = DATA_CACHE->trunk_.find(key);
	if( i == DATA_CACHE->trunk_.end() ) {
		return 0;
	} else {
		return &((*i).second);
	}
}

int data_finder::is_intersection(int carr_id1, int sec1, int carr_id2, int sec2) {
	if( sec1==sec2 ) {
		return 0; // 区内
	}
	bool r = is_inner_city(sec2);
	if( is_inner_city(sec1) &&  r) {
		return 0;
	}
	if( r ) {
		ostringstream os;
		os << carr_id1 << "," << sec1 << "," << carr_id2;
		std::set<string>::iterator i = DATA_CACHE->inter_section_.find(os.str());
		if( i!=DATA_CACHE->inter_section_.end() ) {
			return 2; // 区间不经电路
		}
	}
	return 1; // 区间经电路
}

const multimap<int, classify_info>& data_finder::get_sett_classify() {
	return DATA_CACHE->sett_classify_;
}
const feerate_info* data_finder::get_feerate(int id) {
	std::map<int, feerate_info>::iterator i = DATA_CACHE->feerate_.find(id);
	if( i != DATA_CACHE->feerate_.end() ) {
		return &(*i).second;
	}
	return 0;
}
const std::map<int, group_feerate_info, greater<int> >* data_finder::get_group_feerate(int id) {
	std::map<int, std::map<int, group_feerate_info, greater<int> > >::iterator i = DATA_CACHE->group_feerate_.find(id);
	if( i != DATA_CACHE->group_feerate_.end() ) {
		return &(*i).second;
	}
	return 0;
}

int data_finder::get_group_feerate_type(int id) {
	std::map<int, int>::iterator i = DATA_CACHE->group_feerate_type_.find(id);
	if( i != DATA_CACHE->group_feerate_type_.end() ) {
		return(*i).second;
	}
	return 0;
}

const classify_info* data_finder::get_sett_classify(int id) {
	std::map<int, classify_info*>::iterator i = DATA_CACHE->sett_classify2_.find(id);
	if( i != DATA_CACHE->sett_classify2_.end() ) {
		return(*i).second;
	}
	return 0;
}

const std::map<int, string>& data_finder::get_all_province() {
	return DATA_CACHE->provinces_;
}

const std::map<int, string>& data_finder::get_all_carrier() {
	return DATA_CACHE->carriers_;
}

const std::map<int, string>& data_finder::get_all_sp() {
	return DATA_CACHE->sp_ids_;
}

const std::map<int, string>& data_finder::get_all_section() {
	return DATA_CACHE->sections_;
}

const sett_item_info* data_finder::get_sett_item(int id) {
	sett_item_info_t::const_iterator i = DATA_CACHE->sett_items_.find(id);
	if( i == DATA_CACHE->sett_items_.end() ) {
		return 0;
	} else {
		return &(*i).second;
	}
}

const std::map<int, list<int> >* data_finder::get_sett_rc_group(int id, int sett_obj_type, int sett_obj_id) {
	ostringstream os;
	os << id << "," << sett_obj_type << "," << sett_obj_id;
	sett_rc_group_info_t::const_iterator i = DATA_CACHE->sett_rc_groups_.find(os.str());
	if( i == DATA_CACHE->sett_rc_groups_.end() ) {
		ostringstream os;
		os << id << ",0,0";
		sett_rc_group_info_t::const_iterator i = DATA_CACHE->sett_rc_groups_.find(os.str());
		if( i == DATA_CACHE->sett_rc_groups_.end() ) {
			return 0;
		} else {
			return &(*i).second;
		}
	} else {
		return &(*i).second;
	}
}

int data_finder::get_sett_rc_max_level(int id) {
	std::map<int,int>::const_iterator i = DATA_CACHE->sett_rc_max_level_.find(id);
	if( i == DATA_CACHE->sett_rc_max_level_.end() ) {
		return -1;
	} else {
		return(*i).second;
	}
}

const sett_table_info_t& data_finder::get_sett_table() {
	return DATA_CACHE->sett_tables_;
}

const char* data_finder::get_table_head(int id) {
	std::map<int, string>::const_iterator i = DATA_CACHE->sett_table_heads_.find(id);
	if( i == DATA_CACHE->sett_table_heads_.end() ) {
		return 0;
	} else {
		return(*i).second.c_str();
	}
}

bool data_finder::is_inner_city(int section_id) {
	return DATA_CACHE->sections_flag_[section_id]==0;
}



