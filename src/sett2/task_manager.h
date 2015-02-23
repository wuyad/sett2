#ifndef __TASK_MANAGER_H__
#define __TASK_MANAGER_H__

#include "task.h"
#include <string>
#include <list>

class task_manager {
public:
    task_manager();
    bool parser_command_opt(int argc, char *argv[]);
    bool init_data();
    void run();
    void finish();
    void usage();
    void shutdown();
private:
    std::list<std::string> filenames_;
	std::string file_type_;
	std::string collect_source_;
    int one_off_task_;
};

#endif // __TASK_MANAGER_H__
