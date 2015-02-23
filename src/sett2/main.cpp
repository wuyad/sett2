#include <iostream>
#include <ace/Thread_Mutex.h>
#include <ace/Singleton.h>
#include "task_manager.h"

using namespace std;
int ACE_TMAIN (int argc, char *argv[]) {
    task_manager* mgr = ACE_Singleton<task_manager, ACE_Thread_Mutex>::instance();
    // ����������
    if( !mgr->parser_command_opt(argc, argv) ) {
        return -1;
    }
    // ��ʼ�����ݣ����ػ�������
    if( !mgr->init_data() ) {
        cout << "data init failed" << endl;
        return -1;
    }
    // ����
    mgr->run();
    // �������
    mgr->finish();

    cout << "OK" << endl;
    return 0;
}
