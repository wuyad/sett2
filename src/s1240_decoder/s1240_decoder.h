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
    position record_type;     // ��¼���� IN_RECORD:ISDN_RECORD:PSTN_RECORD
    position callerno;  // ���к���
    position calledno;   // ���к���
    position answer_datetime; // ͨ����ʼʱ��
    position type_of_traffic; // �������ͣ����أ���;������
    position duration;        // ͨ��ʱ��
    position end_datetime; // ͨ������ʱ��
    position incoming_trunk;  // ���м�
    position outgoing_trunk;  // ���м�
    position charged_party;   // �շѷ�
    position charged_free;    // �Ƿ��շ�
};

struct s1240_field_info {
    position connected_number;// ����������
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
    // �Ƿ񱻽��շ�
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


