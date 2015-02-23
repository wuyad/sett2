#ifndef __S1240_DECODER_H__
#define __S1240_DECODER_H__

#include <iosfwd>
#include <string>
#include <decoder_base.h>

#define DECODER_NAME_V74 "S1240V74"
#define DECODER_NAME_CHB2 "S1240CHB2"

const int SIZE_PER_BLOCK = 2048;
enum HALF_BYTE_TYPE {
    LOW, HIGH, ALL
};
enum RECORD_TYPE {
    IN_RECORD=3, ISDN_RECORD=4, PSTN_RECORD=1, OTHER_RECORD
};

struct position {
    int offset;
    int lenth;
    HALF_BYTE_TYPE begin_half_byte;
    HALF_BYTE_TYPE end_half_byte;
    position():begin_half_byte(ALL),end_half_byte(ALL) {
    }
};

struct s1240_fix_field_info {
    position record_type;     // 记录类型 IN_RECORD:ISDN_RECORD:PSTN_RECORD
    position callerno;  // 主叫号码
    position calledno;   // 被叫号码
    position answer_datetime; // 通话开始时间
    position type_of_traffic; // 呼叫类型，本地：长途：国际
    position duration;        // 通话时长
    position end_datetime; // 通话结束时间
    position incoming_trunk;  // 入中继
    position outgoing_trunk;  // 出中继
    position charged_party;   // 收费方
    position charged_free;    // 是否收费
};

struct s1240_field_info {
    position connected_number;// 第三方号码
    position ctx_identity;    // BCG identity of the calling party
    int record_length;
};

struct output_field {
    int record_type;
    std::string callerno;
    std::string calledno;
    std::string answer_datetime;
    std::string end_datetime;
    std::string type_of_traffic;
    std::string duration;
    std::string incoming_trunk;
    std::string outgoing_trunk;
    std::string connected_number;
};

class s1240_decoder_base : public file_decoder_base {
public:
    s1240_decoder_base();
    bool next_record(std::string& record);
    bool get_next_cdr(cdr_ex& cdr);
    virtual std::string name()=0;
protected:
    virtual void init_config()=0;
public:
    int bcd2int(const char* bcd, int len=1, HALF_BYTE_TYPE begin_half_byte = ALL, HALF_BYTE_TYPE end_half_byte = ALL);
    int bcd2int(char bcd, HALF_BYTE_TYPE half_byte = ALL);
    std::string bcd2str(const char* bcd, int len=1, HALF_BYTE_TYPE begin_half_byte = ALL, HALF_BYTE_TYPE end_half_byte = ALL);
    char bcd2str(char bcd, HALF_BYTE_TYPE half_byte = ALL);

    bool is_charge_free();
    // 是否被叫收费
    bool is_called_party_charge();
protected:
    const char* cur_record_;
    const char* end_record_;
    int cur_block_;

    s1240_fix_field_info fix_field_info_;
    s1240_field_info in_field_info_;
    s1240_field_info pstn_field_info_;
    s1240_field_info isdn_field_info_;
};

class s1240_decoder_v74 : public s1240_decoder_base {
public:
    s1240_decoder_v74();
    void init_config();
    std::string name();
};

class s1240_decoder_chb2 : public s1240_decoder_base {
public:
    s1240_decoder_chb2();
    void init_config();
    std::string name();
};

#endif // __S1240_DECODER_H__


