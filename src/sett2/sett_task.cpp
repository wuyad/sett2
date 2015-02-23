#include "sett_task.h"
#include "workflow.h"
#include "cdr_sett.h"
#include "data_finder.h"
#include "log.h"
#include "sett_period.h"
#include <ace/Mem_Map.h>
#include <wuya/timer.h>
#include <wuya/filestat.h>
#include <wuya/fileopt.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace wuya;
// sett_task -----------------------------------------------------------------------
bool sett_task::do_task(task_ptr task) {
	// �ж��ļ���С
	filestat fs(task->file_name.c_str());
	if(!fs.exist() || fs.length()<=0) {
		logerr << "file (" << task->file_name << ") is not exist or size equal 0" << endl;
		move_fail(task->file_name.c_str());
		return false;
	}
	// ���������Ļ���
	proc_context ctx;
	ctx.set(F_FULL_FILENAME, string(fs.get_fullname()));
	ctx.set(F_FILENAME, string(fs.get_filename()));
	ctx.set(F_FILEPATH, string(fs.get_filepath()));
	ctx.set(F_FILESIZE, (int)fs.length());
	ctx.set(F_FILETYPE, task->file_type);
	ctx.set(F_COLLECT_SOURCE, task->collect_source);
	// ��������
	workflow flow(data_finder::get_workflow(task->file_type.c_str()));
	if( !flow.valid() ) {
		logerr << "file type(" << ctx.get<int>(F_FILETYPE) 
		<< ") has not define workflow" << endl;
		return false;
	}
	pair<bool, string> file_name;
	file_name = move_working(task->file_name.c_str());
	if( !file_name.first ) {
		return false;
	}
	// ���ļ�
	ACE_Mem_Map file;
	if( file.map(file_name.second.c_str()) == -1 ) {
		logerr << file_name.second << " open error" << endl;
		return false;
	}
	// �����ļ�
	decoder_ptr decoder = data_finder::create_decoder(task->file_type);
	if( !decoder ) {
		logerr << "file type(" << task->file_type << ") is not supported" << endl;
		move_fail(file_name.second.c_str());
		return false;
	}
	int record_num = 0;
	decoder->init((const char*)file.addr(), file.size());

	for( workflow::iterator i=flow.begin(); i!=flow.end(); ++i ) {
		//���ļ�����
		(*i)->do_pre_proc_file(ctx);
	}
	// ÿ������
	wuya::timer mytimer(true);
	cdr_ex cdr;
	while( decoder->get_next_cdr(cdr) ) {
		++record_num;
		cdr.set(F_FILETYPE, task->file_type);
		cdr.set(F_SOURCE_FILE, string(fs.get_filename()));
		cdr.set(F_COLLECT_SOURCE, task->collect_source);
		//��ÿ��������
		for( workflow::iterator i=flow.begin(); i!=flow.end(); ++i ) {
			//���ļ�����
			if((*i)->get_proc_type()!=proc_base::STORAGE_PROC 
			   && (cdr.get<int>(F_REPEAT)>0 || cdr.get<int>(F_ERROR)>0)) {
				break;
			}
			cdr_ex* p_cdr = &cdr;
			do{
				(*i)->do_proc_record(*p_cdr, ctx);
			}while((p_cdr=p_cdr->next)!=0);
		}
	    cdr.clear();
	}
	double duration = mytimer.end();
	ctx.set(F_RECORD_NUM, record_num);
	loginfo << task->file_name << " process OVER, total records " << record_num << "(" << 
	duration << " s," << (int)(record_num/duration) << " recs/s)." << endl;
	for( workflow::iterator i=flow.begin(); i!=flow.end(); ++i ) {
		//���ļ�����
		(*i)->do_post_proc_file(ctx);
	}
	// ���ļ�����.bakĿ¼��
	file_name = move_bak(file_name.second.c_str());
	return file_name.first;
}
pair<bool, string> sett_task::move_working(const char* filename) {
	pair<bool, string> ret;
	ret.first = true;
	ret.second = filename;
	string::size_type p = ret.second.rfind(FILE_SEP);
	ret.second.insert(p, 1, FILE_SEP);
	ret.second.insert(p+1, ".working");
	// ���ļ�����.workingĿ¼��
	if( !move_file(filename, ret.second.c_str(), true) ) {
		logerr << filename << " move to working error" << endl;
		ret.first = false;
	}
	return ret;
}
pair<bool, string> sett_task::move_bak(const char* filename) {
	pair<bool, string> ret;
	ret.first = true;
	ret.second = filename;
	string::size_type p = ret.second.rfind("working");
	if(p==string::npos) {
		p = ret.second.rfind(FILE_SEP);
		ret.second.insert(p, 1, FILE_SEP);
		ret.second.insert(p+1, ".bak");
	}else{
		ret.second.replace(p, strlen("working"), "bak");
	}

	if( !move_file(filename, ret.second.c_str(), true) ) {
		logerr << filename << " move to bak dir error" << endl;
		ret.first = false;
	}
	return ret;
}
pair<bool, string> sett_task::move_fail(const char* filename) {
	pair<bool, string> ret;
	ret.first = true;
	ret.second = filename;
	string::size_type p = ret.second.rfind("working");
	if(p==string::npos) {
		p = ret.second.rfind(FILE_SEP);
		ret.second.insert(p, 1, FILE_SEP);
		ret.second.insert(p+1, ".fail");
	}else{
		ret.second.replace(p, strlen("working"), "fail");
	}

	if( !move_file(filename, ret.second.c_str(), true) ) {
		logerr << filename << " move to fail dir error" << endl;
		ret.first = false;
	}
	return ret;
}


