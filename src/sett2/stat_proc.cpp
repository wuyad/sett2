#include "stat_proc.h"
#include "data_finder.h"
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include "sett_period.h"
#include "log.h"
#include "sett_condi.h"
#include "db_connect.h"
#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>
#include <wuya/timespan.h>

using namespace std;
using namespace boost;

#define foreach BOOST_FOREACH

bool stat_proc::proc_record(cdr_ex& cdr, proc_context& ctx) {
	// 分析表结构
	// 填充表头
	const sett_table_info_t& tables =  data_finder::get_sett_table();
	// 每个表
	foreach(sett_table_info_t::const_reference per_table, tables) {
		const sett_table_info& table = per_table.second;
		// 是否适合表级限制
		if( table.limit_condi_id!=-1 && !SETT_CONDI_INS->fit(table.limit_condi_id, cdr) ) {
			continue;
		}
		if( table.sett_object_type==0 ) { // 需要为每个结算对象产生统计表
			typedef std::map<int, string> tmp_type;

			const std::map<int, string>* objs=0;
			switch( table.sett_type ) {
			case 1:
				objs = &data_finder::get_all_carrier();
				break;
			case 2:
				objs = &data_finder::get_all_province();
				break;
			case 3:
				objs = &data_finder::get_all_sp();
				break;
			default:
				return false;
			}
			foreach(tmp_type::const_reference r, *objs) {
				if( cdr.get<int>(F_CLS_SETT_OBJ) != r.first ) {
					continue;
				}
				int day = 1;
				if( cdr.has(F_STD_BEGIN_DATETIME) && cdr.has(F_STD_DURATION) ) {
					wuya::datetime& begin = cdr.get<wuya::datetime>(F_STD_BEGIN_DATETIME);
					int duration = cdr.get<int>(F_STD_DURATION);
					wuya::datetime end = begin+wuya::timespan(duration);
					day = end.day();
				}
				string& cdr_sett_period = cdr.get<string>(F_CLS_SETT_PERIOD);

				ostringstream os;
				os << table.name << "," << table.sett_type << "," << r.first << "," << cdr_sett_period << "," << day;
				ACE_Thread_Mutex& mutex = STAT_UPDATE_TASK_INS->mutex_;
				ACE_GUARD_RETURN(ACE_Thread_Mutex, __NOT_USE__, mutex, false);
				table_data& data = STAT_UPDATE_TASK_INS->tables_[os.str()];
				data.table_name = table.name;
				data.sett_obj_type = table.sett_type;
				data.sett_obj_id = r.first;
				data.sett_period = cdr_sett_period;
				data.day = day;
				if( data.init() ) {
					compu_stat_value(data, cdr, table.unit);
				}
			}
		} else {
			int day = 1;
			if( cdr.has(F_STD_BEGIN_DATETIME) && cdr.has(F_STD_DURATION) ) {
				wuya::datetime& begin = cdr.get<wuya::datetime>(F_STD_BEGIN_DATETIME);
				int duration = cdr.get<int>(F_STD_DURATION);
				wuya::datetime end = begin+wuya::timespan(duration);
				day = end.day();
			}
			string& cdr_sett_period = cdr.get<string>(F_CLS_SETT_PERIOD);

			ostringstream os;
			os << table.name << "," << table.sett_object_type << "," << table.sett_object_id << "," 
			<< cdr_sett_period << "," << day;
			ACE_Thread_Mutex& mutex = STAT_UPDATE_TASK_INS->mutex_;
			ACE_GUARD_RETURN(ACE_Thread_Mutex, __NOT_USE__, mutex, false);
			table_data& data = STAT_UPDATE_TASK_INS->tables_[os.str()];
			data.table_name = table.name;
			data.sett_obj_type = table.sett_object_type;
			data.sett_obj_id = table.sett_object_id;
			data.sett_period = cdr_sett_period;
			data.day = day;
			if( data.init() ) {
				compu_stat_value(data, /*data2, */cdr, table.unit);
			}
		}
	}
	return true;
}

void stat_proc::compu_stat_value(table_data& all_grid,cdr_ex& cdr, int table_unit) {
	const grid_head& col_grid = *all_grid.col_head;
	const grid_head& row_grid = *all_grid.row_head;
	grid_data& data = all_grid.data;

	for( int i=0; i<row_grid.get_y_len(); ++i ) { // 每一行每一级,row中包含的标题不参与循环
		bool row_match = true;
		for( int j=0; j<row_grid.get_x_len(); ++j ) { // 某一行的每一级(level)
			item_info& row_info = row_grid.get(j, i);
			if( row_info.type==8||row_info.type==9 ) {
				row_match = false;
				break;
			}
			if( !item_eval::get_item_result(row_info, cdr, true) ) {  // 如果行条件满足,则接着判断列条件
				row_match = false;
				break;
			}
		}
		if( !row_match ) {
			continue;
		}
		for( int x=0; x<col_grid.get_x_len(); ++x ) { // 每一列每一级
			bool col_match = true;
			for( int y=0; y<col_grid.get_y_len(); ++y ) { // 某一列的每一级(level)
				item_info& col_info = col_grid.get(x, y);
				if( col_info.type==8||col_info.type==9 ) {
					col_match = false;
					break;
				}
				if( !item_eval::get_item_result(col_info, cdr, false) ) { // 如果列条件也满足，则可以开始计算了
					col_match = false;
					break;
				}
			}
			if( !col_match ) {
				continue;
			}

			item_info& row_info = row_grid.get(row_grid.get_x_len()-1, i);
			item_info& col_info = col_grid.get(x, col_grid.get_y_len()-1);
			if( row_info.type==8||row_info.type==9||col_info.type==8||col_info.type==9 ) {
				continue;
			}
			// 统计
			int stat_unit = -1;	  // 判断使用哪个统计变量
			if( row_info.type == 15 ) {	  // 如果使用的行统计变量枚举
				stat_unit = row_info.unit;
			} else if( col_info.type == 15 ) { // 如果使用的列统计变量枚举
				stat_unit = col_info.unit;
			} else {
				if( row_info.unit == 0 || col_info.unit == 0 ) { // 如果行属性为不统计,则不统计
					stat_unit = 0;
				} else {
					if( col_info.unit!=0 && col_info.unit!=-1 ) { // 优先使用列统计变量
						stat_unit = col_info.unit;
					} else {
						stat_unit = row_info.unit;
					}
				}
			}
			if( stat_unit==-1 ) {
				stat_unit = table_unit;	  // 表级的单位有最低优先级
			}
			// 结算表的长途通话，只累加时长
			if( all_grid.table_name == "stat_sett_all" ) {
				if( row_info.is_toll ) {
					stat_unit = 2;
				}
			}
			// 开始计算
			long long value = 0;
			switch( stat_unit ) {
			case 1:	  // 话单张数
				value = data.get(x, i);
				++value;
				data.set(x, i, value);
				break;
			case 2:	  // 时长
				value = data.get(x, i);
				value += cdr.get<int>(F_STD_DURATION);
				data.set(x, i, value);
				break;
			case 3:	  // 金额
				value = data.get(x, i);
				value += cdr.get<long long>(F_CLS_FEE);
				data.set(x, i, value);
				break;
			case 4:	  // 跳次
				value = data.get(x, i);
				value += cdr.get<long long>(F_CLS_SETT_HOP);
				data.set(x, i, value);
				break;
			}
		}
	}

	// 计算小计与总计
	for( int i=0; i<row_grid.get_y_len(); ++i ) { // 每一行每一级,row中包含的标题不参与循环
		for( int j=0; j<row_grid.get_x_len(); ++j ) { // 某一行的每一级(level)
			item_info& row_info = row_grid.get(j, i);
			if( row_info.type==8 ) { // 小计
				for( int x=0; x<col_grid.get_x_len(); ++x ) { // 每一列
					long long value = 0;
					for( int k=0; k<row_info.value;++k ) {
						int type = row_grid.get(j, i-1-k).type;
						if( type!=8 && type!=9 ) { // 非小计与总计
							value+=data.get(x, i-1-k);
						}
					}
					data.set(x, i, value);
				}
				break;
			} else if( row_info.type==9 ) {	  // 总计
				for( int x=0; x<col_grid.get_x_len(); ++x ) { // 每一列
					long long value = 0;
					int p = i;
					while( (p -= row_info.value)>=0 ) {
						int type = row_grid.get(j, p).type;
						if( type!=8 && type!=9 ) { // 非小计与总计
							value+=data.get(x, p);
						}
					}
					data.set(x, i, value);
				}
				break;
			}
		}
	}

	for( int i=0; i<col_grid.get_x_len(); ++i ) { // 每一列每一级
		for( int j=0; j<col_grid.get_y_len(); ++j ) { // 某一列的每一级(level)
			item_info& col_info = row_grid.get(i, j);
			if( col_info.type==8 ) { // 小计
				for( int x=0; x<row_grid.get_y_len(); ++x ) { // 每一行
					long long value = 0;
					for( int k=0; k<col_info.value;++k ) {
						value+=data.get(i-1-k, x);
					}
					data.set(i, x, value);
				}
				break;
			} else if( col_info.type==9 ) {	  // 总计
				for( int x=0; x<row_grid.get_y_len(); ++x ) { // 每一行
					long long value = 0;
					int p = i;
					while( (p -= col_info.value)>=0 ) {
						value+=data.get(p, x);
					}
					data.set(i, x, value);
				}
				break;
			}
		}
	}
}

////////////////////////////////////// stat_update_task //////////////////////////////////////////

void stat_update_task::flush() {
	ACE_GUARD(ACE_Thread_Mutex,__NOT_USE__, mutex_);
	typedef std::map<std::string, table_data> map_data;
	//对于每个日结算表
	foreach(map_data::reference i, tables_) {
		table_data& table = i.second;
		list<string> insert_str;
		list<string> update_str;

		ostringstream o_create;
		ostringstream o_create_pk;

		o_create << "create table " << table.table_name << " (sett_period VARCHAR2(6), day number(10), mday VARCHAR2(8),"
		"sett_obj_type number(1), sett_obj_id number(10), "
		"display_order number(10) ";
		if( table.table_name=="stat_sett_all" ) {
			o_create << ", is_toll number(1), feegroup_id number(5)";
		}

		o_create_pk << "alter table " << table.table_name << " add constraint PK_" << table.table_name
		<< " primary key (sett_period, day, sett_obj_type, sett_obj_id, display_order)";

		int x_len = table.data.get_x_len();
		int y_len = table.data.get_y_len();

		ostringstream o_insert;
		o_insert << "insert into " <<table.table_name << " (sett_period, day, mday, sett_obj_type, sett_obj_id, display_order";
		if( table.table_name=="stat_sett_all" ) {
			o_insert << ", is_toll, feegroup_id";
		}
		// 加入行标题
		for( int a=0; a<table.row_head->get_x_len(); ++a ) {
			o_create << ", head" << a << " varchar2(120)";
			o_insert << ", head" << a;
		}
		for( int a=0; a<x_len; ++a ) {
			o_create << ", col" << a << " number(20)";
			o_insert << ", col" << a;
		}
		o_create << ")";
		o_insert << ") values( '" << table.sett_period << "', " << table.day << ",'" << table.sett_period
		<< setw(2) << setfill('0') << table.day <<  "'," << table.sett_obj_type
		<< "," << table.sett_obj_id << ",";
		// 数据
		int order = 0;
		for( int a=0; a<y_len; ++a ) {
			ostringstream o_insert2;
			ostringstream o_update;
			o_insert2 << o_insert.str() << order;

			o_update << "update " <<table.table_name << " set ";
			bool has_set_value = false;
			if( table.table_name=="stat_sett_all" ) {
				item_info& tmp = table.row_head->get(table.row_head->get_x_len()-1, a);
				o_insert2 << "," << (tmp.is_toll?1:0) << "," << tmp.fee_id;

				o_update << "is_toll=" << (tmp.is_toll?1:0) << " , feegroup_id=" << tmp.fee_id;
				has_set_value = true;
			}
			// 加入行标题
			for( int b=0; b<table.row_head->get_x_len(); ++b ) {
				o_insert2 << ",'" << table.row_head->get(b, a).subject << "'";
			}
			// 数据
			for( int b=0; b<x_len; ++b ) {
				long long grid_v = table.data.get(b, a);
				if( has_set_value ) {
					o_update << ',';
				}
				has_set_value = true;
				o_update << " col" << b << "= (col" << b
				<< " + " << table.data.get(b, a) << ")";
				o_insert2 << ',' << table.data.get(b, a);
			}
			o_insert2 <<")";
			o_update << " where sett_period = '" << table.sett_period << "' and day=" << table.day 
			<< " and sett_obj_type=" << table.sett_obj_type << " and sett_obj_id=" << table.sett_obj_id
			<< " and display_order=" << order;
			insert_str.push_back(o_insert2.str());
			update_str.push_back(o_update.str());
			++order;
		}
		// 入库
		list<string>::iterator l=insert_str.begin();
		list<string>::iterator l2=update_str.begin();
		database db;
		if( db ) {
			if( otl_cursor::direct_exec(db, o_create.str().c_str(), otl_exception::disabled)<0 ) {
//  				loginfo << "stat: create stat table <" << table.table_name << "> fail" <<endl;
			}
			if( otl_cursor::direct_exec(db, o_create_pk.str().c_str(), otl_exception::disabled)<0 ) {
//  				loginfo << "stat: create stat table primary key <" << table.table_name << "> fail" <<endl;
			}
			for( ; l!=insert_str.end(); ++l, ++l2 ) {
				if( otl_cursor::direct_exec(db,l2->c_str(), otl_exception::disabled)<=0 ) {
					if( otl_cursor::direct_exec(db,l->c_str(), otl_exception::disabled)<=0 ) {
						if( otl_cursor::direct_exec(db,l2->c_str(), otl_exception::disabled)<=0 ) {
							logerr << "stat: insert or update fail" << endl;
						}
					}
				}
				db->commit();
			}
		}

		table.clear();
	}

}

////////////////////////////////////// table_data //////////////////////////////////////////
bool table_data::init() {
	if( inited==true ) {
		return true;
	}
	stat_table_def* def = STAT_TABLE_DEF_INS;
	table_desc* desc;
	if( (desc=def->get_table_def(table_name, sett_obj_type, sett_obj_id))==0 ) {
		return false;
	}
	col_head=&desc->col_head;
	row_head=&desc->row_head;
//  col_head.init(desc->col_head.get_x_len(), desc->col_head.get_y_len());
//  for( int i=0; i<desc->col_head.get_x_len(); ++i ) {
//  	for( int j=0; j<desc->col_head.get_y_len(); ++j ) {
//  		col_head.set(i, j, desc->col_head.get(i, j));
//  	}
//  }
//  row_head.init(desc->row_head.get_x_len(), desc->row_head.get_y_len());
//  for( int i=0; i<desc->row_head.get_x_len(); ++i ) {
//  	for( int j=0; j<desc->row_head.get_y_len(); ++j ) {
//  		row_head.set(i, j, desc->row_head.get(i, j));
//  	}
//  }
	data.init(col_head->get_x_len(), row_head->get_y_len());
	inited = true;
	return true;
}

////////////////////////////////////// stat_table_def ////////////////////////////////////////////////

bool stat_table_def::init() {
	// 分析表结构
	// 填充表头
	const sett_table_info_t& tables =  data_finder::get_sett_table();
	// 每个表
	foreach(sett_table_info_t::const_reference per_table, tables) {
		const sett_table_info& table = per_table.second;
		if( table.sett_object_type == 0 ) {
			typedef std::map<int, string> tmp_type;

			const std::map<int, string>* objs=0;
			switch( table.sett_type ) {
			case 1:
				objs = &data_finder::get_all_carrier();
				break;
			case 2:
				objs = &data_finder::get_all_province();
				break;
			case 3:
				objs = &data_finder::get_all_sp();
				break;
			default:
				return false;
			}
			foreach(tmp_type::const_reference r, *objs) {
				if( !init_one_def(table.sett_type, r.first, table) ) {
					return false;
				}
			}
		} else {
			if( !init_one_def(table.sett_object_type, table.sett_object_id, table) ) {
				return false;
			}
		}
	}
	return true;
}

bool stat_table_def::init_one_def(int sett_obj_type, int sett_obj_id, const sett_table_info& table_info) {
	ostringstream os;
	os << table_info.name << "," << sett_obj_type << "," << sett_obj_id;
	table_desc& desc = table_def_[os.str()];

	// 左上
	// 每列
	const std::map<int, list<int> >* col_group = data_finder::get_sett_rc_group(table_info.col_id, sett_obj_type, sett_obj_id);
	if( col_group==0 ) {
		return false;
	}
	const std::map<int, list<int> >* row_group = data_finder::get_sett_rc_group(table_info.row_id, sett_obj_type, sett_obj_id);
	if( row_group==0 ) {
		return false;
	}
	int col_max_lvl = data_finder::get_sett_rc_max_level(table_info.col_id);
	int row_max_lvl = data_finder::get_sett_rc_max_level(table_info.row_id);
	// 右上, 计算宽度以及每个单元格的长度
	// 计算宽度，分配内存
	int* col_count = new int[col_max_lvl];
	scoped_array<int> scoped_array_col_count(col_count);
	int* row_count = new int[row_max_lvl];
	scoped_array<int> scoped_array_row_count(row_count);
	int width = compu_rc_len(col_group, col_count);
	if( width==0 ) {
		return false;
	}
	int high = compu_rc_len(row_group, row_count);
	if( high==0 ) {
		return false;
	}

	desc.col_head.init(width, col_max_lvl);
	desc.row_head.init(row_max_lvl, high);
	// 填充表头info
	if( !compu_head_info(col_group, desc.col_head, col_count, col_max_lvl, table_info.name, false) ) {
		return false;
	}
	if( !compu_head_info(row_group, desc.row_head, row_count, row_max_lvl, table_info.name, true) ) {
		return false;
	}
	return true;
}

int stat_table_def::compu_rc_len(const std::map<int, list<int> >* rc_group, int* count) {
	std::map<int, list<int> >::const_iterator i = rc_group->begin();
	int ret = 1;
	for( int x=0;i!=rc_group->end();++i,++x ) {	// 每一级
		int len = 0;
		list<int>::const_iterator j = (*i).second.begin();
		for( ;j!=(*i).second.end();++j ) { // 每一次
			const sett_item_info* item = data_finder::get_sett_item(*j);
			if( item==0 ) {
				return 0;
			}
			if( item->enum_flag==0 ) {
				len += 1;
			} else {
				int tmp = item_eval::get_enum_item_size(*j);
				if( tmp==0 ) {
					return 0;
				}
				len += tmp;
			}
		}
		ret *= len;
		count[x] = len;
	}
	return ret;
}

bool stat_table_def::compu_head_info(const std::map<int, list<int> >* rc_group, grid_head& head, int* count, int max_lvl,
									 const string& table_name, bool is_row) {
	// 计算总体与个体的循环次数
	int *count2 = new int[max_lvl];	// 总体循环次数
	int *count3 = new int[max_lvl];	// 个体循环次数
	scoped_array<int> scoped_array_count2(count2);
	scoped_array<int> scoped_array_count3(count3);

	for( int i=0; i<max_lvl; ++i ) {
		if( i==0 ) {
			count2[i] = 1;
		} else {
			count2[i]=count2[i-1]*count[i-1];
		}
	}

	for( int i=max_lvl-1; i>=0; --i ) {
		if( i==max_lvl-1 ) {
			count3[i] = 1;
		} else {
			count3[i]=count3[i+1]*count[i+1];
		}
	}

	std::map<int, list<int> >::const_iterator i = rc_group->begin();
	int y=0;
	for( ;i!=rc_group->end();++i,++y ) { // 每一级
		int len = 0;
		for( int a0=0; a0<count2[y];++a0 ) { // 总体循环次数

			list<int>::const_iterator j = (*i).second.begin();
			for( ;j!=(*i).second.end();++j ) { // 每一次
				const sett_item_info* item = data_finder::get_sett_item(*j);
				if( item==0 ) {
					return false;
				}
				if( item->enum_flag==0 ) {	// 非枚举
					for( int a1=0; a1<count3[y]; ++a1 ) { // 个体循环次数
						item_info tmp;
						if( !item_eval::get_single_item_info(*j, tmp) ) {
							return false;
						}
						if( tmp.type == 7 ) {// 费率标准，特殊处理
							item_info& up_item = head.get(y-tmp.value, len);
							tmp.unit = up_item.unit;
							if( up_item.type==0 ) {
								tmp.subject = 
								(*data_finder::get_group_feerate(
																data_finder::get_sett_classify(up_item.value)->feerate_id
																)->begin()).second.desc;
							}
						}
						// 对于结算表要加上费率组合ID
						if( table_name == "stat_sett_all" && tmp.type==0 ) {
							const classify_info* info = data_finder::get_sett_classify(tmp.value);
							if( info==0 ) {
								return false;
							}
							tmp.fee_id = info->feerate_id;
							tmp.is_toll = info->toll_flag;
						}
						if( is_row ) {
							head.set(y,len,tmp);
						} else {
							head.set(len,y,tmp);
						}
						++len;
					}// 个体循环次数
				} else { // 枚举
					list<item_info> vitem_info;
					if( !item_eval::get_enum_item_info(*j, vitem_info) ) {
						return false;
					}
					for( list<item_info>::iterator it=vitem_info.begin(); 
					   it!=vitem_info.end(); ++it ) {
						for( int a1=0; a1<count3[y]; ++a1 ) { // 个体循环次数
							if( is_row ) {
								head.set(y,len,*it);
							} else {
								head.set(len,y,*it);
							}
							++len;
						}// 个体循环次数
					}
				}
			}// 每一次
		}// 总体循环次数
	}// 每一级
	return true;
}

table_desc* stat_table_def::get_table_def(const std::string& table_name, int sett_obj_type, int sett_obj_id) {
	ostringstream os;
	os << table_name << "," << sett_obj_type << "," << sett_obj_id;
	std::map<std::string, table_desc>::iterator it = table_def_.find(os.str());
	if( it != table_def_.end() ) {
		return &(*it).second;
	}
	return 0;
}

////////////////////////////////////// item_eval ////////////////////////////////////////////////

bool item_eval::get_single_item_info(int item_id, item_info& info) {
	const sett_item_info* item = data_finder::get_sett_item(item_id);
	if( item==0 ) {
		return false;
	}
	info.type = item->flag;
	info.value = item->value;
	info.unit = item->unit;
	switch( item->flag ) {
	case 0:// 结算分类ID
		info.subject = data_finder::get_sett_expr()
					   [
					   data_finder::get_sett_classify(item->value)->condi_id
					   ].desc;
		break;
	case 1:// 结算条件ID
		info.subject = data_finder::get_sett_expr()[item->value].desc;
		break;
	case 2:// 固定表头显示ID
		info.subject = data_finder::get_table_head(item->value);
		break;
	case 3:// 费率组合ID
		info.subject = (*data_finder::get_group_feerate(item->value)->begin()).second.desc;
		break;
	case 4:// 当前帐期
		info.subject = SETT_PERIOD_INS->get_current();
		break;
	case 5:// 当前结算对象类型
		//sprintf(buf, "%d", (int)(var_eval(cdr_, "sett_obj_type").eval()));
		info.subject = "";
		break;
	case 6:// 当前结算对象
		//sprintf(buf, "%d", (int)(var_eval(cdr_, "sett_obj").eval()));
		info.subject = "";
		break;
	case 7:// 费率标准
		info.subject = ""; // 特殊处理，计费标题时同时计算
		break;
	case 8:// 小计
		info.subject = "小计：";
		break;
	case 9:// 总计
		info.subject = "总计：";
		break;
	}
	return true;
}

int item_eval::get_enum_item_size(int item_id) {
	const sett_item_info* item = data_finder::get_sett_item(item_id);
	if( item==0 ) {
		return 0;
	}
	switch( item->flag ) {
	case 10:// 营业区枚举
		return data_finder::get_all_section().size();
	case 11:// 局号枚举
		break;
	case 12:// 运营商枚举
		return data_finder::get_all_carrier().size();
	case 13:// SP枚举
		return data_finder::get_all_sp().size();
	case 14:// 分公司枚举
		return data_finder::get_all_province().size();
	case 15:// 统计单位枚举
		return 3;
	}
	return 0;
}

bool item_eval::get_enum_item_info(int item_id, std::list<item_info>& infos) {
	item_info info;
	const sett_item_info* item = data_finder::get_sett_item(item_id);
	if( item==0 ) {
		return false;
	}
	info.type = item->flag;

	switch( item->flag ) {
	case 10:// 营业区枚举
		{
			const std::map<int, string>& tmp = data_finder::get_all_section();
			for( std::map<int, string>::const_iterator i=tmp.begin(); i!=tmp.end(); ++i ) {
				info.subject = (*i).second;
				info.value = (*i).first;
				info.unit = -1;
				infos.push_back(info);
			}
		}
		break;
	case 11:// 局号枚举
		break;
	case 12:// 运营商枚举
		{
			const std::map<int, string>& tmp12 = data_finder::get_all_carrier();
			for( std::map<int, string>::const_iterator i=tmp12.begin(); i!=tmp12.end(); ++i ) {
				info.subject = (*i).second;
				info.value = (*i).first;
				info.unit = -1;
				infos.push_back(info);
			}
		}
		break;
	case 13:// SP枚举
		{
			const std::map<int, string>& tmp13 = data_finder::get_all_sp();
			for( std::map<int, string>::const_iterator i=tmp13.begin(); i!=tmp13.end(); ++i ) {
				info.subject = (*i).second;
				info.value = (*i).first;
				info.unit = -1;
				infos.push_back(info);
			}
		}
		break;
	case 14:// 分公司枚举
		{
			const std::map<int, string>& tmp14 = data_finder::get_all_province();
			for( std::map<int, string>::const_iterator i=tmp14.begin(); i!=tmp14.end(); ++i ) {
				info.subject = (*i).second;
				info.value = (*i).first;
				info.unit = -1;
				infos.push_back(info);
			}
		}
		break;
	case 15:// 统计单位枚举
		{
			info.subject = "跳次";
			info.unit = 4;
			info.value = -1;
			infos.push_back(info);
			info.subject = "时长";
			info.unit = 2;
			info.value = -1;
			infos.push_back(info);
			info.subject = "张数";
			info.unit = 1;
			info.value = -1;
			infos.push_back(info);
		}
		break;
	}
	return true;
}

bool item_eval::get_item_result(const item_info& info, cdr_ex& cdr, bool row) {
	switch( info.type ) {
	case 0:// 结算分类ID
		return(cdr.get<int>(F_CLS_SETT_CLSID) == info.value);
	case 1:// 结算条件ID
		return SETT_CONDI_INS->fit(info.value, cdr);
	case 2:// 固定表头显示ID
		return true;
	case 3:// 费率组合ID
		return true;
	case 4:// 当前帐期
		return true;
	case 5:// 当前结算对象类型
		return true;
	case 6:// 当前结算对象
		return true;
	case 7:// 费率标准
		return true;
	case 8:// 小计
		return true;
	case 9:// 总计
		return true;
	case 10:// 营业区枚举
		return info.value == cdr.get<int>(row?F_CLS_CALLER_SECT:F_CLS_CALLED_SECT);
	case 11:// 局号枚举
		return false;
	case 12:// 运营商枚举
		{
			return info.value == cdr.get<int>(row?F_CLS_CALLER_CARR:F_CLS_CALLED_CARR);
		}
	case 13:// SP枚举
		return info.value == cdr.get<int>(F_CLS_CALLER_SP_ID) || info.value == cdr.get<int>(F_CLS_CALLED_SP_ID);
	case 14:// 分公司枚举
		return info.value == cdr.get<int>(row?F_CLS_CALLER_AC:F_CLS_CALLED_AC);
	case 15:// 统计单位枚举
		return true;
	}
	return true;
}

