#include "index_task.h"
#include "log.h"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <list>
#include "data_cache.h"
#include "data_finder.h"
#include <wuya/filefind.h>
#include <wuya/file.h>
#include <wuya/fileopt.h>
#include<boost/tokenizer.hpp>

using namespace std;
using namespace wuya;
using namespace boost::lambda;


// index_task -----------------------------------------------------------------------
index_task::index_task() {
	init();
}

index_task::~index_task() {
	store_close_all();
}

bool index_task::do_task(task_ptr task) {
	store_close(task->file_type);
	return true;
}

void index_task::store_close(const std::string& file_type, bool force) {
	time_index_content* p_time_index=0;	// 第一级的index对象实例，包含时间
	list<index_content*> p_content;
	{
		ACE_GUARD(ACE_Thread_Mutex, __NOT_USE__, *index_.mutex);
		p_time_index = &index_.index[file_type];
	}
	{
		ACE_GUARD(ACE_Thread_Mutex, __NOT_USE__, *p_time_index->mutex);
		transform(p_time_index->index.begin(), p_time_index->index.end(), back_inserter(p_content),
				  &bind(&time_index::value_type::second, _1));
	}
	for( list<index_content*>::iterator i = p_content.begin();
	   i!=p_content.end(); ++i ) {
		ACE_GUARD(ACE_Thread_Mutex, __NOT_USE__, *(*i)->mutex);
		(*i)->store_close(force);
	}
}

void index_task::store_close_all() {
	list<time_index_content*> p_time_index;	// 第一级的index对象实例，包含时间
	list<index_content*> p_content;
	{
		ACE_GUARD(ACE_Thread_Mutex, __NOT_USE__, *index_.mutex);
		transform(index_.index.begin(), index_.index.end(), back_inserter(p_time_index),
				  &bind(&file_type_inx_t::value_type::second, _1));
	}
	for( list<time_index_content*>::iterator i = p_time_index.begin();
	   i!=p_time_index.end(); ++i ) {
		ACE_GUARD(ACE_Thread_Mutex, __NOT_USE__, *(*i)->mutex);
		transform((*i)->index.begin(), (*i)->index.end(), back_inserter(p_content),
				  &bind(&time_index::value_type::second, _1));
	}
	for( list<index_content*>::iterator i = p_content.begin();
	   i!=p_content.end(); ++i ) {
		ACE_GUARD(ACE_Thread_Mutex, __NOT_USE__, *(*i)->mutex);
		(*i)->store_close(true);
	}
}

bool index_task::is_unique(const index_info& info) {
	if (info.query_str==""){
		return true;
	}
	time_index_content* p_time_index=0;	// 第一级的index对象实例，包含时间
	index_content* p_content=0;
	datetime t = compute_time(info.file_type, info.time);
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, __NOT_USE__, *index_.mutex, false);
		p_time_index = &index_.index[info.file_type];
	}
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, __NOT_USE__, *p_time_index->mutex, false);
		p_content = &p_time_index->index[t];
	}
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, __NOT_USE__, *p_content->mutex, false);
		if( p_content->is_loaded==false ) {
			p_content->index_filename = compute_filename(info.file_type, info.collect_source, t);
			p_content->is_loaded = true;
			if( p_content->load() == false ) {
				return true;
			}
		}
		p_content->is_accessed = true;
		return p_content->index.insert(info.query_str).second;
	}
}

std::string index_task::compute_filename(const std::string& file_type, const std::string& collect_source, const datetime& time) {
	string temp = path_name_[file_type];
	if( temp=="" ) {
		temp=".";
	}
	filestat fs(temp.c_str(), (time.format("%Y%m%d_%H.")+file_type+"."+collect_source).c_str());
	make_dir(fs.get_filepath());
	return fs.get_fullname();
}

datetime index_task::compute_time(const std::string& file_type, const datetime& time) {
	int year=time.year();
	int month=time.month();
	int day=time.day();
	int hour=time.hour();
	int time_span = index_timespan_[file_type];
	if( time_span==0 ) {
		time_span=2;
	}
	if( time_span>=24 ) {
		hour = (((day-1)*24+hour)/time_span)*time_span;
		day = hour/24+1;
		hour %= 24;
	} else {
		hour = (hour/time_span)*time_span;
	}
	return datetime(year, month, day, hour);
}

const std::map<string, int>& index_task::get_close_span() {
	return close_timespan_;
}




void index_task::init() {
 
	const exclude_info_t &exclude_Info= data_finder::get_exclude_info();
    exclude_info_t::const_iterator it;
	for(it=exclude_Info.begin(); it!=exclude_Info.end();++it)
	{
		path_name_[(*it).first] = (*it).second.index_path;
		index_timespan_[(*it).first] = (*it).second.division_unit;
		close_timespan_[(*it).first] = (*it).second.store_unit;
	}  


}

bool index_content::store_close(bool force) {
	if( is_loaded == false ) {
		return false;
	}
	if( !force ) {
		if( is_accessed ) {
			loginfo << "attemp to close index, but index is using" << endl;
			is_accessed = false;
			return false;
		}
	}


	file fp;
	if(index.size()==0) {
	  return true;
	}
	if( fp.open(index_filename.c_str(), "a+") ) {
		set<String>::iterator it ;
		for( it=index.begin();it != index.end(); ++it ) {
			fp.write_string((*it).c_str());
		}
		index.clear();
		fp.close();
		loginfo << "index file (" << index_filename << ") closed" << endl;
		is_loaded = false;
		return true;
	} else
		return false;
}

bool index_content::load() {
	file fp;
	char buf[256];
	if( fp.open(index_filename.c_str(), "r") ) {
		while( !fp.eof() ) {
			if(fp.read_string(buf, 256) && buf[0]!='\0') {
				index.insert(buf);
			}
		}
		fp.close();
		loginfo << "load index file ok , delete indexfile "  << index_filename << endl;	
		remove_file(index_filename.c_str());
		return true;
	}
	return false;
}

