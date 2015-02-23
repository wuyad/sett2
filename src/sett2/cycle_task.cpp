#include "cycle_task.h"

using namespace std;

void cycle_task::add_cycle_task(int interval, boost::function<void ()> func) {
	tasks_.push_back(func_info(interval, func));
}

int cycle_task::svc() {
	int cur_magic_num = 0;
	while( !is_stop() ) {
		int min_num=0;
		vector<func_info>::iterator i = tasks_.begin();
		for( ; i!=tasks_.end(); ) {
			if( i->magic_num_ == cur_magic_num ) {
				i->func_();
				if(i->interval_==-1) {
					i = tasks_.erase(i);
					continue;
				}
				i->magic_num_ += i->interval_;
			}
			if( min_num==0||min_num>i->magic_num_ ) {
				min_num=i->magic_num_;
			}
			++i;
		}
		if(min_num==0) {
			break;
		}
		sleep(min_num-cur_magic_num);
		cur_magic_num=min_num;
	}
	return 0;
}

