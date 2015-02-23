#include <vector>
#include <string>
#include <map>
#include <decoder_base.h>

#ifdef _MSC_VER
    #include <windows.h>
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ) {
    return TRUE;
}
    #define MSDECL __declspec(dllexport)
    #pragma warning( disable : 4996 4819 )
#else
    #define MSDECL
#endif

// 公开接口
extern "C" {
    MSDECL bool create_decoder(const std::string& name, decoder_ptr&);
	MSDECL bool init_decoder(const std::map<std::string, int>& cdrex_index);
    MSDECL bool supported_decoder(std::vector<std::string>&);
}
//// 实现部分
#include "s1240_decoder.h"

using namespace std;

bool init_decoder(const std::map<std::string, int>& cdrex_index) {
    return true;
}

bool supported_decoder(std::vector<string>& names) {
    names.push_back(DECODER_NAME_V74);
    names.push_back(DECODER_NAME_CHB2);
    return true;
}

bool create_decoder(const std::string& name, decoder_ptr& ptr) {
    bool ret = false;
    if( name == DECODER_NAME_V74 ) {
        ptr.reset(new s1240_decoder_v74());
        ret = true;
    } else if( name == DECODER_NAME_CHB2 ) {
        ptr.reset(new s1240_decoder_chb2());
        ret = true;
    }
    return ret;
}



