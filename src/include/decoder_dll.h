#ifndef __DECODER_DLL_H__
#define __DECODER_DLL_H__

#include <decoder_base.h>
#include <string>
#include <map>
#include <vector>

extern "C" {
    typedef bool (*fn_create_decoder)(const std::string& name, decoder_ptr&);
	typedef bool (*fn_init_decoder)(const std::map<std::string, int>& cdrex_index);
    typedef bool (*fn_supported_decoder)(std::vector<std::string>&);
}

#endif // __DECODER_DLL_H__
