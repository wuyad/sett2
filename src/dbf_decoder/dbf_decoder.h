#ifndef __DBF_DECODER_H__
#define __DBF_DECODER_H__

#include <string>
#include <vector>
#include <decoder_base.h>

struct dbf_item_define {
	std::string name;
	int offset;
	int length;
	char type; 	// C - �ַ�
				// N - ��ֵ
				// Y - ����(FoxPro)
				// F - ����
				// D - ����
				// T - ����ʱ��(FoxPro)
				// B - ˫����
				// I - ����(FoxPro)
				// L - �߼�
				// M - ��ע
				// G - ͨ��
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
