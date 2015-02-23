#ifndef __DBF_DECODER_H__
#define __DBF_DECODER_H__

#include <string>
#include <vector>
#include <decoder_base.h>

struct dbf_item_define {
	std::string name;
	int offset;
	int length;
	char type; 	// C - 字符
				// N - 数值
				// Y - 货币(FoxPro)
				// F - 浮动
				// D - 日期
				// T - 日期时间(FoxPro)
				// B - 双精度
				// I - 整形(FoxPro)
				// L - 逻辑
				// M - 备注
				// G - 通用
};

class dbf_decoder : public file_decoder_base{
public:
    dbf_decoder();
    dbf_decoder(std::vector<int>* field_define, const std::string& name);
    virtual bool get_next_cdr(cdr_ex& cdr);
    virtual std::string name();
private:
	bool parse_dbf_head();
	
    std::vector<int>* definition_;
    std::string name_;
    int total_record_;
	int bytes_of_head_;
	int bytes_per_record_;
    int cur_record_;
	std::vector<dbf_item_define> dbf_item_define_;
};

#endif // __DBF_DECODER_H__
