#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <cdr_sett.h>
#include "s1240_decoder.h"
#include <boost/scoped_array.hpp>

using namespace std;
using namespace boost;

s1240_decoder_base::s1240_decoder_base():cur_record_(buf_), end_record_(buf_),
    cur_block_(0) {
}

bool s1240_decoder_base::get_next_cdr(cdr_ex& cdr) {
    if( cur_record_ >= end_record_ ) {
        cur_record_ = buf_+SIZE_PER_BLOCK*(cur_block_++);
        if(cur_record_ >= (buf_+len_)) {
            return false;
        }
        int real_len = (cur_record_[13]<<8 | (cur_record_[14]&0xff));
        end_record_ = cur_record_+real_len;
        // 越过记录头
        cur_record_ += 20;
    }
    //开始处理每条记录
    int irecode_type = bcd2int(*(cur_record_+fix_field_info_.record_type.offset), fix_field_info_.record_type.begin_half_byte);
    RECORD_TYPE record_type;
    if( irecode_type==1 ) {
        record_type = PSTN_RECORD;
    } else if( irecode_type == 3 ) {
        record_type = IN_RECORD;
    } else if( irecode_type == 4 ) {
        record_type = ISDN_RECORD;
    } else {
        record_type = OTHER_RECORD;
    }
    output_field output;
    s1240_field_info* field_info;
    if( record_type == IN_RECORD ) {
        field_info = &in_field_info_;
        output.record_type = 3;
    } else if( record_type == ISDN_RECORD ) {
        output.record_type = 4;
        field_info = &isdn_field_info_;
    } else if( record_type == PSTN_RECORD ) {
        output.record_type = 1;
        field_info = &pstn_field_info_;
    } else {
        cout << "haha" << endl;
    }

    output.callerno = bcd2str(cur_record_+fix_field_info_.callerno.offset,
                                    fix_field_info_.callerno.lenth,
                                    fix_field_info_.callerno.begin_half_byte,
                                    fix_field_info_.callerno.end_half_byte);
    output.calledno = bcd2str(cur_record_+fix_field_info_.calledno.offset,
                                   fix_field_info_.calledno.lenth,
                                   fix_field_info_.calledno.begin_half_byte,
                                   fix_field_info_.calledno.end_half_byte);
    output.incoming_trunk = bcd2str(cur_record_+fix_field_info_.incoming_trunk.offset,
                                    fix_field_info_.incoming_trunk.lenth,
                                    fix_field_info_.incoming_trunk.begin_half_byte,
                                    fix_field_info_.incoming_trunk.end_half_byte);
    output.outgoing_trunk = bcd2str(cur_record_+fix_field_info_.outgoing_trunk.offset,
                                    fix_field_info_.outgoing_trunk.lenth,
                                    fix_field_info_.outgoing_trunk.begin_half_byte,
                                    fix_field_info_.outgoing_trunk.end_half_byte);
    output.answer_datetime = bcd2str(cur_record_+fix_field_info_.answer_datetime.offset,
                                     fix_field_info_.answer_datetime.lenth,
                                     fix_field_info_.answer_datetime.begin_half_byte,
                                     fix_field_info_.answer_datetime.end_half_byte);
    output.end_datetime = bcd2str(cur_record_+fix_field_info_.end_datetime.offset,
                                     fix_field_info_.end_datetime.lenth,
                                     fix_field_info_.end_datetime.begin_half_byte,
                                     fix_field_info_.end_datetime.end_half_byte);
    output.type_of_traffic = bcd2str(cur_record_+fix_field_info_.type_of_traffic.offset,
                                     fix_field_info_.type_of_traffic.lenth,
                                     fix_field_info_.type_of_traffic.begin_half_byte,
                                     fix_field_info_.type_of_traffic.end_half_byte);
    output.duration = bcd2str(cur_record_+fix_field_info_.duration.offset,
                              fix_field_info_.duration.lenth,
                              fix_field_info_.duration.begin_half_byte,
                              fix_field_info_.duration.end_half_byte);
    output.connected_number = bcd2str(cur_record_+field_info->connected_number.offset,
                                      field_info->connected_number.lenth,
                                      field_info->connected_number.begin_half_byte,
                                      field_info->connected_number.end_half_byte);

    if( record_type == IN_RECORD || record_type == PSTN_RECORD ) {
        if( is_called_party_charge() ) {
            if( output.calledno != output.connected_number ) {
                output.callerno = output.calledno;
                output.calledno = output.connected_number;
            }
        } else {
            unsigned char ctx0 = *(cur_record_+field_info->ctx_identity.offset);
            unsigned char ctx1 = *(cur_record_+field_info->ctx_identity.offset+1);

            if( !(record_type==IN_RECORD && ctx0==0xff && ctx1==0xff) ) {
                output.connected_number = output.calledno;
            }
        }
    }
    if( record_type == IN_RECORD ) {
        output.calledno += output.connected_number;
    }
	cdr.set(F_RECORD_TYPE, output.record_type);
	cdr.set(F_CALLERNO, output.callerno);
	cdr.set(F_CALLEDNO, output.calledno);
	cdr.set(F_ANSWER_DATETIME, output.answer_datetime);
	cdr.set(F_END_DATETIME, output.end_datetime);
	cdr.set(F_TYPE_OF_TRAFFIC, output.type_of_traffic);
	cdr.set(F_DURATION, output.duration);
	cdr.set(F_INCOMING_TRUNK, output.incoming_trunk);
	cdr.set(F_OUTGOING_TRUNK, output.outgoing_trunk);
	cdr.set(F_CONNECTED_NUMBER, output.connected_number);
	cdr.set(F_SOURCE_OFFSET, cur_block_*SIZE_PER_BLOCK+(int)(cur_record_-buf_));
    cur_record_ += field_info->record_length;
    return true;
}

// half_byte only can be LOW 4bits or HIGH 4bits
int s1240_decoder_base::bcd2int(char bcd, HALF_BYTE_TYPE half_byte) {
    int ret = 0;
    switch( half_byte ) {
    case HIGH:
        ret = (bcd>>4)&0xf;
        break;
    case LOW:
        ret = bcd&0xf;
        break;
    default:
        assert(true);
    }
    return ret;
}

// begin_half_byte : LOW 4bits or ALL
// end_half_byte :  HIGH 4bits or ALL
int s1240_decoder_base::bcd2int(const char* bcd, int len, HALF_BYTE_TYPE begin_half_byte, HALF_BYTE_TYPE end_half_byte) {
    assert(len > 0);
    int value=0,pow1=0;
    if( len>1 ) {
        if( end_half_byte==ALL ) {
            value+=bcd2int(bcd[len-1], LOW)*(int)pow(10.0, pow1++);
        }
        value+=bcd2int(bcd[len-1], HIGH)*(int)pow(10.0, pow1++);
        for( int i=1; i<len-1; ++i ) {// 至少3个才开始
            value+=bcd2int(bcd[len-1-i], LOW)*(int)pow(10.0, pow1++);
            value+=bcd2int(bcd[len-1-i], HIGH)*(int)pow(10.0, pow1++);
        }
    }
    value+=bcd2int(bcd[0], LOW)*(int)pow(10.0, pow1++);
    if( begin_half_byte==ALL ) {
        value+=bcd2int(bcd[0], HIGH)*(int)pow(10.0, pow1++);
    }
    return value;
}

// half_byte only can be LOW 4bits or HIGH 4bits
char s1240_decoder_base::bcd2str(char bcd, HALF_BYTE_TYPE half_byte) {
    char ret='0';
    switch( half_byte ) {
    case HIGH:
        ret += (bcd>>4)&0xf;
        break;
    case LOW:
        ret += bcd&0xf;
        break;
    default:
        assert(true);
    }

    // 去掉填充符'e', '>'   =   '9'+0XE-0X9
    if( ret=='>' ) {
        ret = 0;
    }
    return ret;
}

// begin_half_byte : LOW 4bits or ALL
// end_half_byte :  HIGH 4bits or ALL
string s1240_decoder_base::bcd2str(const char* bcd, int len, HALF_BYTE_TYPE begin_half_byte, HALF_BYTE_TYPE end_half_byte) {
	assert(len<255 && len>0);
	scoped_array<char> sa_buf(new char[255]);
	char* ret = sa_buf.get();
	memset(ret, 0, 255);
	int index=0;
	if( begin_half_byte == ALL ) {
		ret[index++] = bcd2str(*bcd, HIGH);
	}
	ret[index++] = bcd2str(*bcd++, LOW);

	for( int i=1; i<len-1; ++i ) {// 至少3个才开始
		ret[index++] = bcd2str(*bcd, HIGH);
		ret[index++] = bcd2str(*bcd++, LOW);
	}
	if( len>1 ) {
		ret[index++] = bcd2str(*bcd, HIGH);
		if( end_half_byte == ALL ) {
			ret[index++] = bcd2str(*bcd++, LOW);
		}
	}
	return ret;
}

bool s1240_decoder_base::is_charge_free() {
    unsigned char flag = cur_record_[fix_field_info_.charged_free.offset];
    return !( (flag >> fix_field_info_.charged_free.lenth) & 0x1);
}


bool s1240_decoder_base::is_called_party_charge() {
    unsigned char flag = cur_record_[fix_field_info_.charged_free.offset];
    return (flag & 0x2)!=0;
}

string s1240_decoder_v74::name() {
    return DECODER_NAME_V74;
}

s1240_decoder_v74::s1240_decoder_v74() {
    init_config();
}

void s1240_decoder_v74::init_config() {
    fix_field_info_.record_type.offset = 0;
    fix_field_info_.record_type.lenth = 1;
    fix_field_info_.record_type.begin_half_byte = HIGH;
    fix_field_info_.callerno.offset = 6;
    fix_field_info_.callerno.lenth = 10;
    fix_field_info_.calledno.offset = 17;
    fix_field_info_.calledno.lenth = 10;
    fix_field_info_.answer_datetime.offset = 27;
    fix_field_info_.answer_datetime.lenth = 7;
    fix_field_info_.answer_datetime.end_half_byte = HIGH;
    fix_field_info_.type_of_traffic.offset = 33;
    fix_field_info_.type_of_traffic.lenth = 1;
    fix_field_info_.type_of_traffic.begin_half_byte = LOW;
    fix_field_info_.end_datetime.offset = 34;
    fix_field_info_.end_datetime.lenth = 7;
    fix_field_info_.end_datetime.end_half_byte = HIGH;
    fix_field_info_.duration.offset = 41;
    fix_field_info_.duration.lenth = 4;
    fix_field_info_.incoming_trunk.offset = 47;
    fix_field_info_.incoming_trunk.lenth = 2;
    fix_field_info_.outgoing_trunk.offset = 49;
    fix_field_info_.outgoing_trunk.lenth = 2;
    fix_field_info_.charged_party.offset = 58;// bin
    fix_field_info_.charged_party.lenth = 1;// bin
    fix_field_info_.charged_free.offset = 46; // bin
    fix_field_info_.charged_free.lenth = 1; // bin

    in_field_info_.connected_number.offset = 92;
    in_field_info_.connected_number.lenth = 10;
    isdn_field_info_.connected_number.offset = 60;
    isdn_field_info_.connected_number.lenth = 10;
    pstn_field_info_.connected_number.offset = 60;
    pstn_field_info_.connected_number.lenth = 10;

    in_field_info_.ctx_identity.offset = 77;//bin
    in_field_info_.ctx_identity.lenth = 2;//bin
    isdn_field_info_.ctx_identity.offset = 88;//bin
    isdn_field_info_.ctx_identity.lenth = 2;//bin
    pstn_field_info_.ctx_identity.offset = 0;//bin
    pstn_field_info_.ctx_identity.lenth = 0;//bin

    in_field_info_.record_length = 160;
    isdn_field_info_.record_length = 106;
    pstn_field_info_.record_length = 78;
}

string s1240_decoder_chb2::name() {
    return DECODER_NAME_CHB2;
}

s1240_decoder_chb2::s1240_decoder_chb2() {
    init_config();
}

void s1240_decoder_chb2::init_config() {
    fix_field_info_.record_type.offset = 0;
    fix_field_info_.record_type.lenth = 1;
    fix_field_info_.record_type.begin_half_byte = HIGH;
    fix_field_info_.callerno.offset = 6;
    fix_field_info_.callerno.lenth = 10;
    fix_field_info_.calledno.offset = 17;
    fix_field_info_.calledno.lenth = 14;
    fix_field_info_.answer_datetime.offset = 31;
    fix_field_info_.answer_datetime.lenth = 8;
    fix_field_info_.answer_datetime.end_half_byte = HIGH;
    fix_field_info_.type_of_traffic.offset = 38;
    fix_field_info_.type_of_traffic.lenth = 1;
    fix_field_info_.type_of_traffic.begin_half_byte = LOW;
    fix_field_info_.duration.offset = 47;
    fix_field_info_.duration.lenth = 4;
    fix_field_info_.end_datetime.offset = 39;
    fix_field_info_.end_datetime.lenth = 8;
    fix_field_info_.end_datetime.end_half_byte = HIGH;
    fix_field_info_.incoming_trunk.offset = 53;
    fix_field_info_.incoming_trunk.lenth = 2;
    fix_field_info_.outgoing_trunk.offset = 55;
    fix_field_info_.outgoing_trunk.lenth = 2;
    fix_field_info_.charged_party.offset = 64;// bin
    fix_field_info_.charged_party.lenth = 1;// bin
    fix_field_info_.charged_free.offset = 52; // bin
    fix_field_info_.charged_free.lenth = 1; // bin

    in_field_info_.connected_number.offset = 101;
    in_field_info_.connected_number.lenth = 14;
    isdn_field_info_.connected_number.offset = 66;
    isdn_field_info_.connected_number.lenth = 14;
    pstn_field_info_.connected_number.offset = 66;
    pstn_field_info_.connected_number.lenth = 14;

    in_field_info_.ctx_identity.offset = 83;//bin
    in_field_info_.ctx_identity.lenth = 2;//bin
    isdn_field_info_.ctx_identity.offset = 98;//bin
    isdn_field_info_.ctx_identity.lenth = 2;//bin
    pstn_field_info_.ctx_identity.offset = 0;//bin
    pstn_field_info_.ctx_identity.lenth = 0;//bin

    in_field_info_.record_length = 206;
    isdn_field_info_.record_length = 150;
    pstn_field_info_.record_length = 118;
}

