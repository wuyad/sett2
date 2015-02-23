#include "file_decoder_manager.h"
#include <wuya/filefind.h>
#include <boost/bind.hpp>
#include <ace/DLL_Manager.h>
#include <ace/Guard_T.h>
#include <boost/foreach.hpp>
#include <iostream>
#include "data_finder.h"

using namespace std;
using namespace wuya;
using namespace boost;

file_decoder_manager::file_decoder_manager() {
}

file_decoder_manager::~file_decoder_manager() {
}

int file_decoder_manager::load_decoder(const char* dir_name) {
	filestat fs(dir_name);
	if( !fs.is_dir() ) {
		return 0;
	}
	filefind finder(dir_name, "*.dll");
	finder.scan(bind(&file_decoder_manager::find_decoder_file, this, _1));
	filefind finder2(dir_name, "*.sl");
	finder2.scan(bind(&file_decoder_manager::find_decoder_file, this, _1));
	filefind finder3(dir_name, "*.so");
	finder3.scan(bind(&file_decoder_manager::find_decoder_file, this, _1));
	return(int)creators_.size();
}

decoder_ptr file_decoder_manager::create_decoder(const std::string& filetype) {
	decoder_ptr ret;
	std::map<string, fn_create_decoder>::iterator it2 = creators_.find(filetype);
	if( it2 != creators_.end() ) {
		((*it2).second)(filetype, ret);
	}
	return ret;
}

void file_decoder_manager::find_decoder_file(const char* file_name) {
	ACE_DLL_Handle* dll = ACE_DLL_Manager::instance()->open_dll(file_name, 
																ACE_DEFAULT_SHLIB_MODE,
																ACE_SHLIB_INVALID_HANDLE );
	if( !dll ) {
		return;
	}
	// 初始化dll
	fn_init_decoder pfn_init_decoder = (fn_init_decoder) dll->symbol("init_decoder");
	if( !pfn_init_decoder ) {
		return;
	}
	if( !pfn_init_decoder(data_finder::get_cdrex_all_index()) ) {
		return;
	}
	// 取支持的解码器
	fn_supported_decoder pfn_supported_decoder = (fn_supported_decoder) dll->symbol("supported_decoder");
	if( !pfn_supported_decoder ) {
		return;
	}
	vector<string> decoder_names;
	if( !pfn_supported_decoder(decoder_names) ) {
		return;
	}

	fn_create_decoder pfn_create_decoder = (fn_create_decoder) dll->symbol("create_decoder");
	if( !pfn_create_decoder ) {
		return;
	}
	dll_filenames_.push_back(file_name);
	BOOST_FOREACH(string& decoder_name, decoder_names) {
		creators_[decoder_name] = pfn_create_decoder;
	}
}

void file_decoder_manager::unload_decoder() {
	for_each(dll_filenames_.begin(), dll_filenames_.end(),
			 bind(&ACE_DLL_Manager::close_dll, ACE_DLL_Manager::instance(),
				  bind(&string::c_str, _1))
			);
}



