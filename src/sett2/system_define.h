#ifndef __SYSTEM_DEFINE_H__
#define __SYSTEM_DEFINE_H__

#if defined(WIN32)||defined(_WIN32)
    #define CONFIG_FILENAME "..\\conf\\config.ini"
#else
    #define CONFIG_FILENAME "../conf/config.ini"
#endif

#define CONFIG_FILENAME2 "config.ini"
#if defined(WIN32)||defined(_WIN32)
    #define DEFAULT_DECODER_PATH "..\\lib"
#else
    #define DEFAULT_DECODER_PATH "../lib"
#endif

#if defined(WIN32)||defined(_WIN32)
    #define LOGFILE_NAME "..\\log\\sett_$DATE.log"
#else
    #define LOGFILE_NAME "../log/sett_$DATE.log"
#endif

#define DEFAULT_DECODER_PATH2 "."

#define MAX_DB_CONNECT 20

#define DATA_CACHE ACE_Singleton<data_cache, ACE_Thread_Mutex>::instance()
#define CONFIG DATA_CACHE->system_config_


#endif // __SYSTEM_DEFINE_H__
