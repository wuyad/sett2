#ifndef __DATA_CACHE_H__
#define __DATA_CACHE_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <map>
#include <wuya/config.h>
#include "file_decoder_manager.h"

struct file_source_info {
//  std::string name;
	std::string type;
	std::string source;
	int interval;
};

struct hcode_info {
	std::string dis_no;
	int carrier_id;
	std::string end_no;
};

struct spcode_info {
	std::string end_no;
	int id;
	std::string name;
	int type;
};

struct telno_rule {
	int     priority;			// 比较优先级
	char    startChars[7];		// 号码字冠
	int     minlength;			// 最大长度
	int     maxlength;			// 最小长度
	int     operateType;		// 操作转向，处理的号码类型。
};

struct service_telno {
//  std::string areacode;
//  std::string serviceno;
	int servcietype;
	int carrier_id;
};

struct exclude_info {
	std::string  file_type;				//话单文件类别
	std::string  index_path;			//索引文件存放目录
	int     division_unit;				//索引文件划分单位 选值 N hour
	int     store_unit;					//索引文件划分数量 
	int     field_num;					//排重字段数
	std::string  field_name;			//排重字段  多个字段用 | 分割
};

//struct cfg_file_type_info {
//    std::string field_name;
//    std::string field_display_name;
//    int field_length;
//    int type;
//};
//
struct cdrex_def_info {
	int index;
	std::string name;
	int type;
	int length;
};

struct sett_expr_info {
	std::string expr;
	std::string desc;
};

struct expr_const_info {
	std::string name;
	int value;
};

struct cicno_info{
	int carrier_id;
	int type;
	std::string no;
};

struct trunk_info{
	int carrier_id;
	int type;
	std::string peer_ac;
};

struct cell_prefix_info{
	int carrier_id;
	int net_type;
};

struct telno_segment_info{
	int term_type;
	int carrier_id;
	int sect_id;
	std::string key;
	std::string areacode;
};

struct classify_info{
	int id;
	int condi_id;
	int debit_flag;
	int feerate_id;
	int deduct;
	int toll_flag;
};

struct feerate_info{
	int id;
	int unit;
	int n_unit;
	int feerate;
	int currency;
	int max_limit;
	int hop;
};

struct group_feerate_info{
	int id;
	int type;
	int group_type;
	int lower_bound;
	int upper_bound;
	int fid;
	int deduct;
	std::string desc;
};

struct sett_item_info{
	int value;
	int flag;
	int enum_flag;
	int unit;
};

//struct sett_rc_group_info{
//    int sett_object_type;
//    int sett_object_id;
//    int item_id;
//    std::string head_desc;
//};

struct sett_table_info{
	std::string name;
	int sett_type;
	int sett_object_type;
	int sett_object_id;
	int row_id;
	int col_id;
	int limit_condi_id;
	int unit;
};      

typedef std::map<std::string, hcode_info> hcode_info_t;
typedef std::map<std::string, spcode_info> spcode_info_t;
typedef std::map<std::string, exclude_info> exclude_info_t;
typedef std::map<std::string, cicno_info> cicno_info_t;
typedef std::map<std::string, service_telno> service_telno_info_t;
typedef std::map<int, sett_item_info> sett_item_info_t;
typedef std::map<std::string, std::map<int, std::list<int> > > sett_rc_group_info_t;
typedef std::map<int, sett_table_info> sett_table_info_t;

class data_cache {
public:
	data_cache();
	~data_cache();

	bool init();
	bool fini();
	wuya::config system_config_;
private:
	bool load_file_source();
	bool load_proc_step();
	bool load_storage_type();
	bool load_workflow();
	bool load_country_id();
	bool load_special_number();
	bool load_area_code();
	bool load_cicno();
	bool load_hcode();
	bool load_spcode();
	bool load_callerno_rule();
	bool load_calledno_rule();
	bool load_telno_segment();
	bool load_section_type();
	bool load_exclude_info();
	bool load_cdrex_def();
	bool load_cdrex_indb();
	bool load_sett_expr();
	bool load_expr_const();
    bool load_report_info();
	bool load_cellno_prefix();
	bool load_trunk_info();
	bool load_sett_classify();
	bool load_feerate();
	bool load_group_feerate();
	// for stat
	bool load_province();
	bool load_carrier();
	bool load_section();
	bool load_sett_item();
	bool load_sett_rc_group();
	bool load_sett_table();
	bool load_sett_rc_max_level();
	bool load_sett_table_head();

	std::map<std::string, file_source_info> file_source_info_;
	file_decoder_manager decoder_manager_;
	std::set<std::string> proc_step_;
	std::set<std::string> storage_type_;
	std::map<std::string, std::string> workflow_;
	std::set<std::string> country_id_;
	std::map<std::string, service_telno> service_telno_;
	size_t min_service_telno_len_;
	std::map<std::string, int> area_code_;
	std::map<std::string, cicno_info> cicno_;
	size_t min_cicno_len_;
	std::set<std::string> cicno_profix_;
	size_t min_cicno_profix_len_;
	hcode_info_t hcode_;
	spcode_info_t spcode_;
	std::vector<telno_rule> callerno_rule_;
	std::vector<telno_rule> calledno_rule_;
	std::set<std::string> inter_section_;
	std::map<std::string, telno_segment_info> telno_segment_;
	size_t min_telno_segment_len_;
	exclude_info_t exclude_info_;
//  std::map<std::string, std::vector<cfg_file_type_info> > cfg_file_type_;
	std::map<int, int> sections_flag_;
	std::map<std::string, int> cdrex_index_;
	std::map<int, cdrex_def_info> cdrex_def_;
	std::map<std::string, std::vector<cdrex_def_info> > cdrex_indb_;
	std::map<int, sett_expr_info> sett_expr_;
	std::list<expr_const_info> expr_const_;
	std::map<std::string,std::set<int> > report_info_;
	std::map<std::string, cell_prefix_info> cellno_prefix_;
	size_t min_cellno_prefix_len_;
	std::map<std::string, trunk_info> trunk_;
	std::multimap<int, classify_info> sett_classify_;
	std::map<int, classify_info*> sett_classify2_;
	std::map<int, feerate_info> feerate_;
	std::map<int, std::map<int, group_feerate_info, std::greater<int> > > group_feerate_;
	std::map<int, int> group_feerate_type_;
	// for stat
	std::map<int, std::string> provinces_;
	std::map<int, std::string> carriers_;
	std::map<int, std::string> sp_ids_;
	std::map<int, std::string> sections_;
	sett_item_info_t sett_items_;
	sett_rc_group_info_t sett_rc_groups_;
	std::map<int, int> sett_rc_max_level_;
	sett_table_info_t sett_tables_;
	std::map<int, std::string> sett_table_heads_;	
	
	friend class data_finder;
};

#endif // __DATA_CACHE_H__


