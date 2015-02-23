#include "data_cache.h"
#include "system_define.h"
#include "db_connect.h"
#include "data_finder.h"
#include "sett_condi.h"
#include "sett_period.h"
#include "db_storage.h"
#include "stat_proc.h"
#include <limits>
#include <boost/foreach.hpp>

using namespace std;

data_cache::data_cache() {
}

data_cache::~data_cache() {

}

bool data_cache::init() {
	system_config_.open(CONFIG_FILENAME);
	if( !system_config_.good() ) {
		system_config_.open(CONFIG_FILENAME2);
		if( system_config_.good() ) {
			return false;
		}
	}
	// 初始化数据库连接池
	const char* db_connect = system_config_.get<const char*>("db_connect", "db");
	if( db_connect==0 ) {
		return false;
	}
	conn_pool::init(db_connect, MAX_DB_CONNECT, true);
	// 测试数据库是否可以连接
	if( otl_connect* db = conn_pool::instance()->get_connect() ) {
		conn_pool::instance()->revert_connect(db);
		cout << "database connect ok" << endl;
	} else {
		cout << "database init fail with connect string \"" << db_connect << "\"" << endl;
		return false;
	}

	if( !load_file_source() || !load_proc_step()
		|| !load_storage_type() || !load_workflow() || !load_country_id()
		|| !load_special_number() || !load_area_code() || !load_cicno() 
		|| !load_callerno_rule() || !load_telno_segment()
		|| !load_calledno_rule() || !load_exclude_info() || !load_cdrex_def() 
		|| !load_report_info() || !load_cdrex_indb()
		|| !load_sett_expr() || !load_expr_const()
		|| !load_spcode() || !load_cellno_prefix() || !load_trunk_info()
		|| !load_sett_classify() || !load_feerate() || !load_group_feerate()
		|| !load_province() || !load_section_type()
		|| !load_carrier()
		|| !load_section()
		|| !load_sett_item()
		|| !load_sett_rc_group()
		|| !load_sett_table()
		|| !load_sett_table_head()
		|| !load_sett_rc_max_level()
		|| !load_hcode()
	  ) {
		return false;
	}

	// 初始化文件解码器
	const char* decoder_path = system_config_.get<const char*>("path", "decoder");
	if( decoder_path==0 ) {
		return false;
	}
	if( decoder_manager_.load_decoder(decoder_path)<=0 ) {
		if( decoder_manager_.load_decoder(DEFAULT_DECODER_PATH)<=0 ) {
			if( decoder_manager_.load_decoder(DEFAULT_DECODER_PATH2)<=0 ) {
				cout << "file decoder can not find." << endl;
				return false;
			}
		}
	}
	cout << "file decoder init OK." << endl;

	if( SETT_CONDI_INS->init() ) {
		cout << "cache sett expr OK" << endl;
	} else {
		cout << "cache sett expr FAIL" << endl;
		return false;
	}

	if (SETT_PERIOD_INS->load_sett_period()){
		cout << "settle period init OK" << endl;
	}else{
		cout << "settle period init fail" << endl;
		return false;
	}

	if (CDR_INDB_INS->init()){
		cout << "cdr indb init OK" << endl;
	}else{
		cout << "cdr indb init fail" << endl;
		return false;
	}
	
	if (STAT_TABLE_DEF_INS->init()){
		cout << "stat define init OK" << endl;
	}else{
		cout << "stat define init fail" << endl;
		return false;
	}
	
	//CDR::init();
	
	return true;
}

bool data_cache::fini() {
	decoder_manager_.unload_decoder();
	CDR_INDB_INS->fini();
	return true;
}

otl_stream& operator>>(otl_stream& s, pair<string, file_source_info>& row) {
	s >> row.first >> row.second.type >> row.second.interval >> row.second.source;
	return s;
}

bool data_cache::load_file_source() {
	database db;
	if( db ) {
		try {
			otl_stream sql(10, "select root_dir, file_type, interval, collect_source from cfg_file_source", db);
			copy(otl_input_iterator<pair<string, file_source_info> >(sql), 
				 otl_input_iterator<pair<string, file_source_info> >(),
				 inserter(file_source_info_, file_source_info_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_file_source error" << endl;
		}
	}
	return false;
}

bool data_cache::load_proc_step() {
	database db;
	if( db ) {
		try {
			otl_stream sql(10, "select name from dict_proc_step", db);
			copy(otl_input_iterator<string>(sql), otl_input_iterator<string>(),
				 inserter(proc_step_, proc_step_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_proc_step error" << endl;
		}
	}
	return false;
}

bool data_cache::load_storage_type() {
	database db;
	if( db ) {
		try {
			otl_stream sql(10, "select name from dict_file_storage_type", db);
			copy(otl_input_iterator<string>(sql), otl_input_iterator<string>(),
				 inserter(storage_type_, storage_type_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_storage_type error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& s, pair<string, string>& row) {
	s >> row.first >> row.second;
	return s;
}

otl_stream& operator>>(otl_stream& s, pair<int, int>& row) {
	s >> row.first >> row.second;
	return s;
}

otl_stream& operator>>(otl_stream& s, pair<string, int>& row) {
	s >> row.first >> row.second;
	return s;
}

bool data_cache::load_workflow() {
	bool ret = false;
	{
		database db;
		if( db ) {
			try {
				otl_stream sql(1, "select file_type,define from cfg_workflow", db);
				copy(otl_input_iterator<pair<string, string> >(sql), otl_input_iterator<pair<string, string> >(),
					 inserter(workflow_, workflow_.end()));
				ret = true;
			} catch( ... ) {
				cout<<"load_workflow error" << endl;
			}
		}
	}
	return ret;
}

bool data_cache::load_country_id() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select nationcode from cfg_country", db);
			copy(otl_input_iterator<string>(sql), otl_input_iterator<string>(),
				 inserter(country_id_, country_id_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_country_id error" << endl;
		}
	}
	return false;
}

bool data_cache::load_special_number() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select serviceno,servicetype,carrierid from cfg_service_telno", db);
			string no;
			min_service_telno_len_=numeric_limits<int>::max();
			while( !sql.eof() ) {
				sql >> no;
				service_telno& tmp = service_telno_[no];
				sql >> tmp.servcietype >> tmp.carrier_id;
				if( no.length()<min_service_telno_len_ ) {
					min_service_telno_len_ = no.length();
				}
			}
			return true;
		} catch( ... ) {
			cout<<"load_special_number error" << endl;
		}
	}
	return false;
}

bool data_cache::load_area_code() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select areacode,provindex from cfg_areacode", db);
			copy(otl_input_iterator<pair<string, int> >(sql), otl_input_iterator<pair<string, int> >(),
				 inserter(area_code_, area_code_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_area_code error" << endl;
		}
	}
	return false;
}

bool data_cache::load_cicno() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select areacode||cicno, carrierid, type, cicno from cfg_cicno", db);
			string no;
			min_cicno_len_=numeric_limits<int>::max();
			min_cicno_profix_len_=numeric_limits<int>::max();
			while( !sql.eof() ) {
				sql >> no;
				cicno_info& info = cicno_[no];
				sql >> info.carrier_id >> info.type;
				if( no.length()<min_cicno_len_ ) {
					min_cicno_len_ = no.length();
				}
				
				sql >> no;
				info.no = no;
				if( no.length()<min_cicno_profix_len_ ) {
					min_cicno_profix_len_ = no.length();
				}
				cicno_profix_.insert(no);
			}
			return true;
		} catch( ... ) {
			cout<<"load_net_code error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& s, pair<string, hcode_info>& row) {
	s >> row.first >> row.second.end_no >> row.second.dis_no >> row.second.carrier_id;
	return s;
}

bool data_cache::load_hcode() {
	database db;
	if( db ) {
		try {
			otl_stream sql(1000, "select startchars||startno,startchars||endno,disno,carrierid from cfg_cellno",
						   db);
			copy(otl_input_iterator<pair<string, hcode_info> >(sql), otl_input_iterator<pair<string, hcode_info> >(),
				 inserter(hcode_, hcode_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_net_code error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& s, pair<string, spcode_info>& row) {
	s >> row.first >> row.second.end_no >> row.second.id >> row.second.name >> row.second.type;
	return s;
}

bool data_cache::load_spcode() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select startno,endno,id,name,type from cfg_spcode", db);
			copy(otl_input_iterator<pair<string, spcode_info> >(sql), otl_input_iterator<pair<string, spcode_info> >(),
				 inserter(spcode_, spcode_.end()));
			BOOST_FOREACH(spcode_info_t::reference i, spcode_) {
				sp_ids_[i.second.id] = i.second.name.c_str();
			}
			return true;
		} catch( ... ) {
			cout<<"load_spcode error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& s, telno_rule& row) {
	s >> row.priority >> row.startChars >> row.minlength 
	>> row.maxlength >> row.operateType;
	return s;
}

bool data_cache::load_callerno_rule() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select priority, startchars, minlength, maxlength, operatetype "
						   "from cfg_callernorule order by priority, startchars" 
						   , db);
			copy(otl_input_iterator<telno_rule>(sql), otl_input_iterator<telno_rule>(),
				 back_inserter(callerno_rule_));
			return true;
		} catch( ... ) {
			cout<<"load_callerno_rule error" << endl;
		}
	}
	return false;
}

bool data_cache::load_calledno_rule() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select priority, startchars, minlength, maxlength, operatetype "
						   "from cfg_callednorule order by priority, startchars" 
						   , db);
			copy(otl_input_iterator<telno_rule>(sql), otl_input_iterator<telno_rule>(),
				 back_inserter(calledno_rule_));
			return true;
		} catch( ... ) {
			cout<<"load_calledno_rule error" << endl;
		}
	}
	return false;
}

bool data_cache::load_section_type() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select caller_carrierid||','||caller_sectionid||','||called_carrierid"
						   " from cfg_section_type"
						   , db);
			copy(otl_input_iterator<string>(sql), otl_input_iterator<string>(),
				 inserter(inter_section_, inter_section_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_section_type error" << endl;
		}
	}
	return false;
}

bool data_cache::load_telno_segment() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select areacode||prefix, termtype, carrierid, sectionid, "
						   "carrierid||sectionid, areacode from cfg_telno_segment"
						   , db);
			string no;
			min_telno_segment_len_ = numeric_limits<int>::max();
			while( !sql.eof() ) {
				sql >> no;
				telno_segment_info& tmp = telno_segment_[no];
				sql >> tmp.term_type >> tmp.carrier_id >> tmp.sect_id >> tmp.key >> tmp.areacode;
				if( no.length() < min_telno_segment_len_ ) {
					min_telno_segment_len_ = no.length();
				}
			}
			return true;
		} catch( ... ) {
			cout<<"load_telno_segment error" << endl;
		}
	}
	return false;
}


bool data_cache::load_exclude_info() {
	database db;
	if( db ) {
		try {
			otl_stream i(50, "select filetype, indexpath, division_unit, store_unit, "
							 "fieldnum, fieldname from cfg_exclude" 
						 , db);
			string filetype;
			while( !i.eof() ) {
				i >> filetype;
				i >> exclude_info_[filetype].index_path;
				i >> exclude_info_[filetype].division_unit;
				i >> exclude_info_[filetype].store_unit;
				i >> exclude_info_[filetype].field_num;
				i >> exclude_info_[filetype].field_name;
				exclude_info_[filetype].file_type = filetype;
			}
			return true;
		} catch( ... ) {
			cout<<"load_exclude_info error" << endl;
		}
	}
	return false;
}

bool data_cache::load_cdrex_def() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select field_index,field_name,field_type,filed_len "
							   "from cfg_cdrex_def", db);
			string field_name;
			int index;
			while( !sql.eof() ) {
				sql >> index;
				sql >> field_name;
				sql >> cdrex_def_[index].type;
				sql >> cdrex_def_[index].length;
				cdrex_def_[index].name = field_name;
				cdrex_index_[field_name] = index;
			}
			return true;
		} catch( ... ) {
			cout<<"load_cdrex_def error" << endl;
		}
	}
	return false;
}

bool data_cache::load_cdrex_indb(){
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select a.file_type, b.field_type, a.field_index, b.filed_len, "
							   "b.field_name from cfg_cdr_indb a, cfg_cdrex_def b where "
							   "a.field_index=b.field_index order by a.file_type,a.field_order", db);
			string file_type;
			cdrex_def_info info;
			while( !sql.eof() ) {
				sql >> file_type >> info.type >> info.index >> info.length >> info.name;
				cdrex_indb_[file_type].push_back(info);
			}
			return true;
		} catch( ... ) {
			cout<<"load_cdrex_indb error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& s, pair<int, sett_expr_info>& row) {
	s >> row.first >> row.second.expr >> row.second.desc;
	return s;
}

bool data_cache::load_sett_expr() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select id, expression, description from cfg_sett_expr", db);
			copy(otl_input_iterator<pair<int, sett_expr_info> >(sql), 
				 otl_input_iterator<pair<int, sett_expr_info> >(),
				 inserter(sett_expr_, sett_expr_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_sett_expr error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& o, expr_const_info& info) {
	o >> info.name >> info.value;
	return o;
}

bool data_cache::load_expr_const() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select name, value from cfg_expr_const", db);
			copy(otl_input_iterator<expr_const_info>(sql), 
				 otl_input_iterator<expr_const_info>(),
				 back_inserter(expr_const_));
			return true;
		} catch( ... ) {
			cout<<"load_sett_expr error" << endl;
		}
	}
	return false;
}

bool data_cache::load_cellno_prefix() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select prefix, carrierid, net_type from cfg_cellno_prefix", db);
			string no;
			min_cellno_prefix_len_=numeric_limits<int>::max();
			while( !sql.eof() ) {
				sql >> no;
				sql >> cellno_prefix_[no].carrier_id >> cellno_prefix_[no].net_type;
				if( no.length()<min_cellno_prefix_len_ ) {
					min_cellno_prefix_len_ = no.length();
				}
			}
			return true;
		} catch( ... ) {
			cout<<"load_cellno_prefix error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& o, pair<string, trunk_info>& info) {
	o >> info.first >> info.second.carrier_id >> info.second.type >> info.second.peer_ac;
	return o;
}

bool data_cache::load_trunk_info() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select areacode||collect_source||trunk, carrierid, type, peer_areacode"
						   " from cfg_trunk where use_flag=1", db);
			copy(otl_input_iterator<pair<string, trunk_info> >(sql), 
				 otl_input_iterator<pair<string, trunk_info> >(),
				 inserter(trunk_, trunk_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_trunk_info error" << endl;
		}
	}
	return false;
}


bool data_cache::load_report_info() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select table_name, classify_id from cfg_report_dispose " 
						   , db);
			string table_name;
			int  cls_id;
			while( !sql.eof() ) {
				sql >> table_name >> cls_id ;
				report_info_[table_name].insert(cls_id);
			}
			return true;
		} catch( ... ) {
			cout<<"load_report_info error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& o, pair<int, classify_info>& info) {
	o >> info.first >> info.second.condi_id >> info.second.debit_flag >> info.second.feerate_id
	>> info.second.id >> info.second.deduct >> info.second.toll_flag;
	return o;
}

bool data_cache::load_sett_classify() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select sett_object*100+sett_type*10+sett_object_type,condition_id,"
						   "debit_flag,feerate_id,id, deduct, toll_flag from cfg_sett_classify", db);
			copy(otl_input_iterator<pair<int, classify_info> >(sql), 
				 otl_input_iterator<pair<int, classify_info> >(),
				 inserter(sett_classify_, sett_classify_.end()));
			typedef multimap<int, classify_info>::reference ref;
			BOOST_FOREACH(ref i, sett_classify_) {
				sett_classify2_[i.second.id] = &i.second;
			}
			return true;
		} catch( ... ) {
			cout<<"load_sett_classify error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& o, pair<int, feerate_info>& info) {
	o >> info.first >> info.second.unit >> info.second.n_unit >> info.second.feerate
	>> info.second.currency >> info.second.max_limit >> info.second.hop;
	return o;
}

bool data_cache::load_feerate() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select id,unit,n_unit,feerate,currency,max_limit,hop from cfg_feerate", db);
			copy(otl_input_iterator<pair<int, feerate_info> >(sql), 
				 otl_input_iterator<pair<int, feerate_info> >(),
				 inserter(feerate_, feerate_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_feerate error" << endl;
		}
	}
	return false;
}

bool data_cache::load_group_feerate() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select id, lower_bound, type, group_type, "
						   " upper_bound,fid, deduct, description from cfg_fee_group", db);
			int id;
			int lower_bound;
			int type;
			while( !sql.eof() ) {
				sql >> id >> lower_bound;
				sql >> type;
				group_feerate_[id][lower_bound].type = type;
				group_feerate_[id][lower_bound].id = id;
				group_feerate_[id][lower_bound].lower_bound = lower_bound;
				sql >> group_feerate_[id][lower_bound].group_type;
				sql >> group_feerate_[id][lower_bound].upper_bound;
				sql >> group_feerate_[id][lower_bound].fid;
				sql >> group_feerate_[id][lower_bound].deduct;
				sql >> group_feerate_[id][lower_bound].desc;
				
				group_feerate_type_[id] = type;
			}
			return true;
		} catch( ... ) {
			cout<<"load_ladder_feerate error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& s, pair<int, string>& row) {
	s >> row.first >> row.second;
	return s;
}

bool data_cache::load_province() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select provindex, provname||'通信公司' "
						   "from cfg_province", db);
			copy(otl_input_iterator<pair<int, string> >(sql), otl_input_iterator<pair<int, string> >(),
				 inserter(provinces_, provinces_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_province error" << endl;
		}
	}
	return false;
}

bool data_cache::load_carrier() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select carrierid, chinesename from cfg_carrier where flag=1", db);
			copy(otl_input_iterator<pair<int, string> >(sql), otl_input_iterator<pair<int, string> >(),
				 inserter(carriers_, carriers_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_carrier error" << endl;
		}
	}
	return false;
}

bool data_cache::load_section() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select sectionid, sectionname, inner_city from dict_section_info where sectionid!=1", db);
			int sectid;
			while(!sql.eof()) {
				sql >> sectid;
				sql >> sections_[sectid];
				sql >> sections_flag_[sectid];
			}
			return true;
		} catch( ... ) {
			cout<<"load_section error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& s, pair<int, sett_item_info>& row) {
	s >> row.first >> row.second.value >> row.second.flag >> row.second.enum_flag >> row.second.unit;
	return s;
}
bool data_cache::load_sett_item() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select id, value, flag, enum_flag, unit from cfg_stat_item", db);
			copy(otl_input_iterator<pair<int, sett_item_info> >(sql), otl_input_iterator<pair<int, sett_item_info> >(),
				 inserter(sett_items_, sett_items_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_sett_item error" << endl;
		}
	}
	return false;
}

bool data_cache::load_sett_rc_group() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select id||','||sett_object_type||','||sett_object_id, "
							   "group_level, stat_item_id from cfg_stat_rowcol_group "
							   "order by id, group_level, sett_object_type, sett_object_id,display_order", db);
			string key;
			int level, id;
			while( !sql.eof() ) {
				sql >> key >> level >> id;
				sett_rc_groups_[key][level].push_back(id);
			}
			return true;
		} catch( ... ) {
			cout<<"load_sett_rc_group error" << endl;
		}
	}
	return false;
}

bool data_cache::load_sett_rc_max_level() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select id, max(group_level)+1 from cfg_stat_rowcol_group group by id", db);
			copy(otl_input_iterator<pair<int, int> >(sql),
				 otl_input_iterator<pair<int, int> >(),
				 inserter(sett_rc_max_level_, sett_rc_max_level_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_sett_rc_max_level error" << endl;
		}
	}
	return false;
}

otl_stream& operator>>(otl_stream& s, pair<int, sett_table_info>& row) {
	s >> row.first >> row.second.name >>row.second.sett_type >> row.second.sett_object_type 
		>> row.second.sett_object_id >> row.second.row_id >> row.second.col_id 
		>> row.second.limit_condi_id >> row.second.unit;
	return s;
}

bool data_cache::load_sett_table() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select id, table_name, sett_type, sett_object_type, sett_object_id, "
						   "row_id, col_id, condi_id, unit from cfg_stat_table", db);
			copy(otl_input_iterator<pair<int, sett_table_info> >(sql),
				 otl_input_iterator<pair<int, sett_table_info> >(),
				 inserter(sett_tables_, sett_tables_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_sett_table error" << endl;
		}
	}
	return false;
}

bool data_cache::load_sett_table_head() {
	database db;
	if( db ) {
		try {
			otl_stream sql(50, "select id, name from cfg_stat_table_head ", db);
			copy(otl_input_iterator<pair<int, string> >(sql), otl_input_iterator<pair<int, string> >(),
				 inserter(sett_table_heads_, sett_table_heads_.end()));
			return true;
		} catch( ... ) {
			cout<<"load_sett_table_head error" << endl;
		}
	}
	return false;
}

