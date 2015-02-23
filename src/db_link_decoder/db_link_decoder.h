#ifndef __DB_LINK_DECODER_H__
#define __DB_LINK_DECODER_H__

#include <wuya/connect_pool.h>
#include <string>
#include <vector>
#include <decoder_base.h>

class db_link_decoder : public file_decoder_base{
public:
    db_link_decoder();
	db_link_decoder(std::vector<int>* field_define, const std::string& name);
    virtual bool get_next_cdr(cdr_ex& cdr);
    virtual std::string name();
private:
    std::vector<int>* definition_;
	std::string name_;
    int cur_line_;

	otl_connect db_;
	otl_stream sql_;
	int n_cols_;
	std::vector<std::string> fields_;
};

#endif // __DB_LINK_DECODER_H__
