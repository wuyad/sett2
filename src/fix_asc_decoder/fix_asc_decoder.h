#ifndef __FIX_ASC_DECODER_H__
#define __FIX_ASC_DECODER_H__

#include <string>
#include <vector>
#include <decoder_base.h>

struct fix_asc_field_define{
    fix_asc_field_define():left_justify(true),filler(' '){
    }
    int index;
    int offset;
    int length;
    bool left_justify;
    char filler;
};

class fix_asc_decoder : public file_decoder_base{
public:
    fix_asc_decoder();
    fix_asc_decoder(std::vector<fix_asc_field_define>* field_define, const std::string& name);
    virtual bool get_next_cdr(cdr_ex& cdr);
    virtual std::string name();
private:
    std::vector<fix_asc_field_define>* definition_;
    std::string name_;
    const char* next_begin_;
    int cur_line_;
};

#endif // __FIX_ASC_DECODER_H__
