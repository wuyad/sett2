#ifndef __LOG_H__
#define __LOG_H__

#include <wuya/ace_log.h>
#ifdef _DEBUG
    #define log_debug(message) do{ logdbg << message << std::endl; }while(0)
#else
    #define log_debug(message) do{}while(0)
#endif // _DEBUG

#define LOGSTREAM wuya::logstream<wuya::file_output_type>
#define mylog LOGSTREAM().val()
#define logerr mylog << wuya::LOG_ERROR<< std::setw(20) << wuya::now
#define logwarn mylog << wuya::LOG_WARN<< std::setw(20) << wuya::now
#define loginfo mylog << wuya::LOG_INFO<< std::setw(20) << wuya::now
#define logdbg mylog << wuya::LOG_DEBUG<< std::setw(20) << wuya::now

#endif // __LOG_H__
