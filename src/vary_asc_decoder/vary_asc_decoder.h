#ifndef __VARY_ASC_DECODER_H__
#define __VARY_ASC_DECODER_H__

#include <string>
#include <vector>
#include <decoder_base.h>

struct vary_asc_field_define{
	std::string field_sep;
	std::string record_sep;
	std::vector<int> field_names;
};

class vary_asc_decoder : public file_decoder_base{
public:
    vary_asc_decoder();
    vary_asc_decoder(vary_asc_field_define* field_define, const std::string& name);
    virtual bool get_next_cdr(cdr_ex& cdr);
    virtual std::string name();
private:
	bool is_sep(char c, const std::string& all_sep);
    vary_asc_field_define* definition_;
    std::string name_;
    const char* next_begin_;
    int cur_line_;
};

#endif // __FIX_ASC_DECODER_H__
