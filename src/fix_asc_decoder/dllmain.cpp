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
#include "fix_asc_decoder.h"
#include <wuya/config.h>
#include <boost/foreach.hpp>
#include <boost_hpux_bug.h>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace wuya;
using namespace boost;

typedef std::map<std::string, vector<fix_asc_field_define> > definition_t;
static definition_t definitions;
static bool good;

#if defined(WIN32)||defined(_WIN32)
	#define FIX_ASC_DECODER_CONFIG_FILE_NAME "..\\conf\\fix_asc_decoder.ini"
#else
	#define FIX_ASC_DECODER_CONFIG_FILE_NAME "../conf/fix_asc_decoder.ini"
#endif
#define FIX_ASC_DECODER_CONFIG_FILE_NAME2 "fix_asc_decoder.ini"

bool init_decoder(const std::map<std::string, int>& cdrex_index) {
	good = false;
	config conf;
	conf.open(FIX_ASC_DECODER_CONFIG_FILE_NAME);
	if( !conf.good() ) {
		conf.open(FIX_ASC_DECODER_CONFIG_FILE_NAME2);
		if( conf.good() ) {
			return good;
		}
	}
	int valid_field = 0;
	vector<const char*> decoder_names = conf.section_names();
	vector<string> split_result;
	BOOST_FOREACH(const char* decoder_name, decoder_names) {
		vector<const char*> fields = conf.key_names(decoder_name);
		vector<fix_asc_field_define> tmp_field_define;
		BOOST_FOREACH(const char* field_name, fields) {
			const char* define = conf.get<const char*>(field_name, decoder_name);
			if( define==0 || strcmp(define, "")==0 ) {
				continue;
			}
			// 这部分性能不是主要问题

			split( split_result, define, bind1st(std::equal_to<char>(), ',') );
			// 至少需要偏移量与长度
			if( split_result.size()<2 ) {
				continue;
			}
			fix_asc_field_define one_field;
			
			map<string,int>::const_iterator i = cdrex_index.find(field_name);
			if(i == cdrex_index.end()) {
				one_field.index = -1;
			}else{
				one_field.index = (*i).second;
			}
			one_field.offset = atoi(split_result[0].c_str());
			one_field.length = atoi(split_result[1].c_str());
			if( one_field.length<=0 ) {
				continue;
			}
			++valid_field;
			if( split_result.size()>2 ) {
				one_field.left_justify = (split_result[2][0]=='L');
				if( split_result.size()>3 ) {
					one_field.filler = split_result[3][0];
				}
			}
			tmp_field_define.push_back(one_field);
		}
		if( valid_field ) {
			definitions[decoder_name] = tmp_field_define;
			valid_field=0;
			good = true;
		}
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
		decoder.reset(new fix_asc_decoder(&(*it).second, name));
		return true;
	}
	return false;
}



