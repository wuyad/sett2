#ifndef __DECODER_BASE_H__
#define __DECODER_BASE_H__

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <cdr_sett.h>

class file_decoder_base;
typedef boost::shared_ptr<file_decoder_base> decoder_ptr;

class file_decoder_base {
public:
    file_decoder_base():buf_(0), len_(0){}

    void init(const char* buf, size_t len) {
        buf_ = buf;
        len_ = len;
    }
    virtual std::string name()=0;
    virtual bool get_next_cdr(cdr_ex& cdr)=0;
protected:
    const char* buf_;
    size_t len_;
};

#endif // __DECODER_BASE_H__
