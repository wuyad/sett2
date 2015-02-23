#ifndef __INDEX_TASK_H__
#define __INDEX_TASK_H__
#include "task.h"
#include <my_datetime.h>
#include <set>
#include <map>
#include <ace/Thread_Mutex.h>
#include <boost/shared_ptr.hpp>

class String {
public:
	String():buf_(0) {
	}
	~String() {
		delete [] buf_;
	}
	String(const char* buf) {
		  buf_ = new char[strlen(buf)+1];
		  strcpy(buf_, buf);
	}
	String(const String& buf) {
		  buf_ = new char[strlen(buf.buf_)+1];
		  strcpy(buf_, buf.buf_);
	}
	String& operator=(const char* buf){
		buf_ = new char[strlen(buf)+1];
		strcpy(buf_, buf);
		return *this;
	}
	bool operator==(const String& buf) const{
		return strcmp(buf_, buf.buf_)==0;
	}
	bool operator<(const String& buf) const{
		return strcmp(buf_, buf.buf_)<0;
	}
	bool operator>(const String& buf) const{
		return strcmp(buf_, buf.buf_)>0;
	}
	bool operator==(const char* buf) const{
		return strcmp(buf_, buf)==0;
	}
	bool operator<(const char* buf) const{
		return strcmp(buf_, buf)<0;
	}
	bool operator>(const char* buf) const{
		return strcmp(buf_, buf)>0;
	}
	char* c_str(){
		return buf_;
	}
	const char* c_str() const{
		return buf_;
	}
private:
	char* buf_;
};

struct index_info {
	std::string file_type;
	std::string collect_source;
	wuya::datetime time;
	String query_str;
};

struct index_content {
	bool is_loaded;
	std::set<String> index;
	std::string index_filename;
	boost::shared_ptr<ACE_Thread_Mutex> mutex;
	bool is_accessed;
	
	index_content():is_loaded(false), is_accessed(false),
		mutex(new ACE_Thread_Mutex){
	}
	
	/**
	 * ������д����̣��������������
	 * �˲��������ڶ��̰߳�ȫ�������б����ã����뿼�ǻ���
	 * 
	 * @param force
	 * 
	 * @return 
	 */
	bool store_close(bool force);
	/**
	 * �Ӵ����ж�������
	 * �˲��������ڶ��̰߳�ȫ�������б����ã����뿼�ǻ���
	 * 
	 * @return 
	 */
	bool load();
	
};

typedef std::map<wuya::datetime, index_content> time_index;
struct time_index_content {
	time_index index;
	boost::shared_ptr<ACE_Thread_Mutex> mutex;
	
	time_index_content():mutex(new ACE_Thread_Mutex){
	}
};

typedef std::map<std::string, time_index_content> file_type_inx_t;
struct file_type_inx_t_content {
	file_type_inx_t index;
	boost::shared_ptr<ACE_Thread_Mutex> mutex;
	
	file_type_inx_t_content():mutex(new ACE_Thread_Mutex){
	}
};

class index_task:public lf_task {
public:
	index_task();
	~index_task();
	bool do_task(task_ptr task);
	bool is_unique(const index_info& info);
	const std::map<std::string, int>& get_close_span();
private:
    

	/**
	 * ���������ݴ����ļ���������� 
	 * ֻ�ر�is_accessed==false���ļ���is_accessed=true����Ϊfalse
	 * 
	 * @param force  false �����̲��Ա����ļ����������
	 *               true ���������ļ����������
	 */
	void store_close(const std::string&, bool force=false);
	void store_close_all();
	/**
	 * ��ʼ������
	 * ��ȷ������������������û�а취��֪
	 */
	void init();

	std::string compute_filename(const std::string&, const std::string& collect_source, const wuya::datetime& time);
	wuya::datetime compute_time(const std::string&, const wuya::datetime& time);

	file_type_inx_t_content index_;

	// read only param beside init()
	std::map<std::string, int> index_timespan_;
	std::map<std::string, int> close_timespan_;
	std::map<std::string, std::string> path_name_;

	
};

#define INDEX_TASK_INS ACE_Singleton<index_task, ACE_Thread_Mutex>::instance()

#endif // __INDEX_TASK_H__



