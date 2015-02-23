#ifndef __STAT_PROC_H__
#define __STAT_PROC_H__

#include "proc_base.h"
#include "task.h"
#include <list>
#include <string>
#include <map>
#include <boost/variant.hpp>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>

struct item_info {
	std::string subject;
	int type;// 0-结算分类id 1-结算条件ID 2-固定表头显示ID 3-费率组合ID 4-当前帐期 
			 // 5-当前结算对象类型 6-当前结算对象 7-费率标准 8-小计 9-总计 
			 // 10-营业区枚举 11-局号枚举 12-运营商枚举 13-SP枚举 14-分公司枚举 15-统计单位枚举
	int value;
	int unit;

	int fee_id;
	bool is_toll;

	item_info():fee_id(0), is_toll(false){
	}
	void clear(){
		subject="";
		type=0;
		value=0;
		unit=0;
		fee_id=0;
		is_toll=false;
	}
};

template <class T>
class grid {
public:
	grid();
	grid(int x, int y);
	void init(int x, int y);
	T& get(int x, int y) const;
	void set(int x, int y, const T& v);
	void set(int x, int y, int x2, int y2, const T& v);

	int get_x_len() const;
	int get_y_len() const;

	T* get_buffer();
	const T* get_buffer() const;
	~grid();
private:
	T* value_;
	int x_;
	int y_;
};

class item_eval {
public:
	static bool get_single_item_info(int item_id, item_info& info);
	static bool get_enum_item_info(int item_id, std::list<item_info>& infos);
	static int get_enum_item_size(int item_id);
	static bool get_item_result(const item_info& info, cdr_ex& cdr, bool row);
};

typedef grid<long long> grid_data;
typedef grid<item_info> grid_head;

struct table_desc {
	grid_head col_head;
	grid_head row_head;
};

class table_data {
public:
	const grid_head* col_head;
	const grid_head* row_head;
	grid_data data;

	std::string table_name;
	std::string sett_period;
	int day;
	int sett_obj_type;
	int sett_obj_id;
	bool inited;
	table_data():inited(false){
	}
	void clear(){
		data.set(0,0,data.get_x_len(), data.get_y_len(), 0);
	}
public:
	bool init();
};

struct sett_table_info;

class stat_table_def {
public:
	bool init();
	table_desc* get_table_def(const std::string& table_name, int sett_obj_type, int sett_obj_id);
private:
	bool init_one_def(int sett_obj_type, int sett_obj_id, const sett_table_info& table_info);
	int compu_rc_len(const std::map<int, std::list<int> >* rc_group, int* count);
	bool compu_head_info(const std::map<int, std::list<int> >* rc_group, 
		grid_head& head, int* count, int max_lvl, const std::string& table_name, bool is_row);
	std::map<std::string, table_desc> table_def_;
};

class stat_proc : public proc_base {
protected:
	virtual bool proc_record(cdr_ex& cdr, proc_context& ctx);
private:
	void compu_stat_value(table_data& all_grid, cdr_ex& cdr, int table_unit);

	table_data& get_table_data(const std::string& table_name, const std::string& sett_period, int day,
							   int sett_obj_type, int sett_obj_id);

};

class stat_update_task:public lf_task {
public:
	bool do_task(task_ptr task){
		flush();
		return true;
	}
	void flush();
private:
	ACE_Thread_Mutex mutex_;
	std::map<std::string, table_data> tables_;
	friend class stat_proc;
};

template<class T>
inline grid<T>::grid():value_(0), x_(0), y_(0) {
}
template<class T>
inline grid<T>::grid(int x, int y) {
	init(x, y);
}
template<class T>
inline void grid<T>::init(int x, int y) {
	x_ = x;
	y_ = y;
	value_ = new T[x*y];
	T t = T();
	set(0, 0, x, y, t);
}
template<class T>
inline T& grid<T>::get(int x, int y) const {
	return *(value_+x_*y+x);
}
template<class T>
inline void grid<T>::set(int x, int y, const T& v) {
	*(value_+x_*y+x) = v;
}
template<class T>
inline void grid<T>::set(int x, int y, int x2, int y2, const T& v) {
	for( int i=x; i<x2; ++i ) {
		for( int j=y; j<y2; ++j ) {
			*(value_+x_*j+i) = v;
		}
	}
}
template<class T>
inline int grid<T>::get_x_len() const {
	return x_;
}
template<class T>
inline int grid<T>::get_y_len() const {
	return y_;
}
template<class T>
inline grid<T>::~grid() {
	delete [] value_;
}
template<class T>
inline T* grid<T>::get_buffer() {
	return value_;
}
template<class T>
inline const T* grid<T>::get_buffer() const {
	return value_;
}

#define STAT_TABLE_DEF_INS ACE_Singleton<stat_table_def, ACE_Thread_Mutex>::instance()
#define STAT_UPDATE_TASK_INS ACE_Singleton<stat_update_task, ACE_Thread_Mutex>::instance()

#endif // __STAT_PROC_H__

