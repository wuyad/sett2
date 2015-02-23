#ifndef __DATA_FINDER_H__
#define __DATA_FINDER_H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include <decoder_base.h>
#include "task_manager.h"
#include "data_cache.h"

class data_finder {
public:
	static const char* get_ftp_default_local_root();
	static int get_ftp_keep_last_n_subpath();
	static int get_max_sett_thread();
	static std::map<std::string, file_source_info>& get_file_source_info();
	static decoder_ptr create_decoder(const std::string& name);
	static bool is_valid_proc_step(const char* name);
	static bool is_valid_storage_type(const char* name);
	static const char* get_workflow(const char* name);
	static const char* get_file_storage_root_path();
	static const char* get_file_storage_path_name_policy(int cdr_type);
	static int get_file_storage_keep_last_n_subpath();
	static const char* get_db_storage_table_name_policy(int cdr_type);
	static int get_db_storage_record_num_per_batch();
	static bool is_valid_country_id(const char* country_id);
	static bool is_inner_city(int section_id);
	static const service_telno_info_t::value_type* get_special_number(const char* special_number);
	static bool is_special_number(const char* special_number);
	static bool is_valid_area_code(const char* area_code);
	static int get_prov_from_area_code(int area_code);
	/**
	 * 
	 * @param telno
	 * 
	 * @return 如果没有匹配值，返回0，否则返回网号，网号代码长度可通过strlen取得
	 */
	static const char* get_cicno(const char* telno);
	static const cicno_info_t::value_type* get_cicno_info(const telno& no);
	/**
	 * 根据手机H码获取区号
	 * 
	 * @param MobNum   不为0,且大于等于6位
	 * @param areaCode
	 * 
	 * @return 区号，没有匹配的返回0
	 */
	static const hcode_info* get_mobile_info(const char* mobile_no);
	static const spcode_info* get_sp_id(const char* user_code);
	static const std::string& get_sp_name(int id);
	static std::vector<telno_rule>& get_caller_rule();
	static std::vector<telno_rule>& get_called_rule();
	/**
	 * 
	 * @param t1
	 * @param t2
	 * 
	 * @return -1-没有匹配记录
	 *         0-区间
	 *         1-区内
	 */
	static int is_intersection(int carr_id1, int sec1, int carr_id2, int sec2);

	static const exclude_info_t& get_exclude_info();
//  static std::vector<cfg_file_type_info>& get_cfg_file_type(const std::string& file_type);
	static int get_cdrex_index(const std::string& name);
	static const std::string& get_cdrex_name(int index);
	static const std::map<std::string, int>& get_cdrex_all_index();
	static const std::vector<cdrex_def_info>& get_cdrex_indb(const std::string& filetype);
	static const cdrex_def_info* get_cdrex_info(int index);
	static std::map<int, sett_expr_info>& get_sett_expr();
	static std::list<expr_const_info>& get_expr_const();
	static bool itisme(int carr_id);
	static bool is_local_province(int area_code);
	static bool is_local_ac(int area_code);
	static int get_cur_carr_id();
	static int get_cur_province();
	static int get_cur_ac();
	static const telno_segment_info* get_fix_term_info(const telno& no);
	static const cell_prefix_info* get_cell_prefix_info(const char* area_code);
	static std::set<int>& get_report_info(const std::string&  tablename);
	static const trunk_info* get_trunk_info(const char* area_code, const char* collect_source, const char* trunk);
	static const std::multimap<int, classify_info>& get_sett_classify();
	static const classify_info* get_sett_classify(int id);
	static const feerate_info* get_feerate(int id);
	static const std::map<int, group_feerate_info, std::greater<int> >* get_group_feerate(int id);
	static int get_group_feerate_type(int id);

	// for stat
	static const std::map<int, std::string>& get_all_province();
	static const std::map<int, std::string>& get_all_sp();
	static const std::map<int, std::string>& get_all_carrier();
	static const std::map<int, std::string>& get_all_section();
	static const sett_item_info* get_sett_item(int id);
	static const std::map<int, std::list<int> >* get_sett_rc_group(int id, int sett_obj_type, int sett_obj_id);
	static int get_sett_rc_max_level(int id);
	static const sett_table_info_t& get_sett_table();
	static const char* get_table_head(int id);
};

#endif // __DATA_FINDER_H__
