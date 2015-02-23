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
#include "dbf_decoder.h"
#include <wuya/config.h>
#include <boost/foreach.hpp>
#include <boost_hpux_bug.h>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace wuya;
using namespace boost;

typedef std::map<std::string,vector<int> > definition_t;
static definition_t definitions;
static bool good;

#if defined(WIN32)||defined(_WIN32)
	#define DBF_DECODER_CONFIG_FILE_NAME "..\\conf\\dbf_decoder.ini"
#else
	#define DBF_DECODER_CONFIG_FILE_NAME "../conf/dbf_decoder.ini"
#endif
#define DBF_DECODER_CONFIG_FILE_NAME2 "dbf_decoder.ini"

bool init_decoder(const std::map<std::string, int>& cdrex_index) {
	good = false;
	config conf;
	conf.open(DBF_DECODER_CONFIG_FILE_NAME);
	if( !conf.good() ) {
		conf.open(DBF_DECODER_CONFIG_FILE_NAME2);
		if( conf.good() ) {
			return good;
		}
	}
	vector<const char*> decoder_names = conf.section_names();
	vector<string> split_result;
	vector<int> all_field;
	BOOST_FOREACH(const char* decoder_name, decoder_names) {
		const char* fields = conf.get<const char*>("fields", decoder_name);
		if( fields==0 ) {
			continue;
		}
		split( split_result, fields, bind1st(std::equal_to<char>(), ',') );
		if( split_result.size()==0 ) {
			continue;
		}
		BOOST_FOREACH(const string& f, split_result) {
			map<string,int>::const_iterator i = cdrex_index.find(f);
			if(i == cdrex_index.end()) {
				all_field.push_back(-1);
			}else{
				all_field.push_back((*i).second);
			}
		}
		definitions[decoder_name] = all_field;
		good = true;
	}

	return good;
}

bool supported_decoder(std::vector<std::string>& ret) {
	if( !good ) {
		return good;
	}
	transform(definitions.begin(), definitions.end(), back_inserter(ret), 
			  bind(&definition_t::value_type::first, _1));
	return true;
}

bool create_decoder(const std::string& name, decoder_ptr& decoder) {
	if( !good ) {
		return good;
	}
	definition_t::iterator it = definitions.find(name);
	if( it != definitions.end() ) {
		decoder.reset(new dbf_decoder(&(*it).second, name));
		return true;
	}
	return false;
}



