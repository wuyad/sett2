#ifndef __FILE_DECODER_MANAGER_H__
#define __FILE_DECODER_MANAGER_H__

#include <string>
#include <map>
#include <decoder_base.h>
#include <decoder_dll.h>
#include <ace/Thread_Mutex.h>

class file_decoder_manager {
public:
    file_decoder_manager();
    ~file_decoder_manager();
    /**
     * 加载文件解码器，必须在单线程中进行
     * 
     * @param dir_name
     * 
     * @return 
     */
    int load_decoder(const char* dir_name);
    decoder_ptr create_decoder(const std::string& filetype);
    void find_decoder_file(const char* file_name);
    void unload_decoder();
private:

    std::map<std::string, fn_create_decoder> creators_;

    std::vector<std::string> dll_filenames_;

};

#endif // __FILE_DECODER_MANAGER_H__


