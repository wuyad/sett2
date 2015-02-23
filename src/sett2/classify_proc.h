#ifndef __CLASSIFY_PROC_H__
#define __CLASSIFY_PROC_H__

#include "proc_base.h"

namespace SETT {
	enum SETT_TYPE {
		INET_SETT=1/*�������*/,
		TOLL_SETT/*���ڳ�;����*/,
		SP_SETT/*����������*/,
	};

	enum BUSI_TYPE {
		VOICE=1/*��ͨ����*/,
		IPP/*IP ҵ��*/,
		INN/*������ҵ��*/,
		IVR/*������ֵҵ��*/,
		SMSVAS/*������ֵҵ��*/,
		INTERNET/*������ҵ��*/,
		SMSP2P/*�������*/,
	};

	enum AREA_TYPE {
		LOCAL_CALL=1/*����*/,
		TOLL/*��;*/,
		INTERNATIONAL/*����*/,
	};

	enum TTYPE {
		FIX=0/*�̶��绰*/,
		PHS=2/*С��ͨ*/,
		M800/*��ͨ800��*/,
		GSM/*GSM�ֻ�*/,
		CDMA/*CDMA�ֻ�*/,
	};

	enum CALL_ORIENT {
		COME=1/*��*/,
		GO/*ȥ*/,
	};

	enum SSERV_TYPE {
		NSSERV=0/*���ط�*/,
		PAYSSERV=1/*�շ��ط�*/,
		FREESSERV=2/*����ط�*/,
		NETSSERV=3/*������*/,
		EMERGENCY=4/*�����ط�*/,
	};

	enum DEBIT_TYPE {
		DEBIT=1/*��*/,
		CREDIT/*��*/,
	};

	enum SETT_OBJECT_TYPE {
		ALL_OBJECT=0/*������Ӫ��*/,
		OTHER=1/*������Ӫ��*/,
		INNERNET/*��������ʡ��˾*/,
		SP_OBJECT/*SP*/,
	};

	enum IN_TYPE {
		IN_200=1/*200*/,
		IN_300/*300*/,
		IN_400/*400*/,
		IN_600/*600*/,
		IN_800/*800*/,
		IN_201/*201*/,
	};

	enum IP_TYPE {
		IP_CARD=0/*IP��ҵ��*/,
		IP_DIRECT/*IPֱ��*/,
		TOLL_DIRECT/*��;ֱ��*/,
	};

	enum LOCAL_TYPE {
		INNER_SECT=1/*Ӫҵ����*/,
		INTER_SECT0/*Ӫҵ����(��ͨ��������·)*/,
		INTER_SECT/*Ӫҵ����(ͨ��������·)*/,
		UNKNOW_SECT/*δ֪*/,
	};

	enum TOLL_TYPE {
		COME_USE_MINE=1/*��������;*/,
		GO_USE_MINE/*ȥ������;*/,
		COME_USE_OTHER/*����������;*/,
		GO_USE_OTHER/*ȥ��������;*/,
	};

	enum INET_TOLL_TYPE {
		COME_GAT=1/*����۰�̨*/,
		COME_NGAT/*������������*/,
		GO_GAT/*ȥ��۰�̨*/,
		GO_NGAT/*ȥ����������*/,
	};

	enum CALL_TRANS {
		CALL_TRANS_NO=1/*�Ǻ���ת��*/,
		CALL_TRANS_YES=2/*����ת��*/,
	};

	enum {
		NONE=-1
	};
}

struct call_info {
	int carr;
	int ac;
	int sect;
	int ttype;
	int sserv_type;
	int sserv_no;
	int prefix;
	int ip_type;
	int in_type;
	int trunk_carr;
	int spcode;
	int sp_id;
	int internet;
	int country;
	int is_call_nav;//�Ƿ�绰����
	int is_center_sect;
	int trunk_type;// 0����, 1��;, 2��Ⱥ

	call_info():carr(-1),ac(-1),sect(-1),ttype(-1),sserv_type(-1),sserv_no(-1),prefix(-1),
	ip_type(-1),in_type(-1),trunk_carr(-1),spcode(-1),sp_id(-1),internet(-1),country(-1),
		is_call_nav(0),is_center_sect(0), trunk_type(-1)  {
	}
};

struct feerate_info;

class classify_proc : public proc_base {
public:
	classify_proc():sett_type_(-1),sett_obj_type_(-1),sett_obj_(-1),p_cdr_(0), p_ctx_(0),
		busi_type_(-1),area_type_(-1),local_type_(-1),toll_type_(-1),inet_toll_type_(-1),
		call_trans_(-1),call_orient_(-1),sett_clsid_(-1),feerate_id_(-1),fee_(0),
		sett_hop_(0),currency_(-1),debit_(-1),sett_obj_flag_(-1){
	}
protected:
	bool proc_record(cdr_ex& cdr, proc_context& ctx);
	bool classify();
	void clear();
private:
	cdr_ex* p_cdr_; 
	proc_context* p_ctx_;
	cdr_ex& get_cdr(){
		return *p_cdr_;
	}
	proc_context& get_ctx(){
		return *p_ctx_;
	}
	void guest_collect_source();
	void set_error_n(int err_no, ...);
	void set_error(int err_no);
	void store_to_cdr();
	void store_to_cdr2();

	int fit_condition(int key);

	void parse_sett_type();
	void parse_call_info(bool is_caller);
	void parse_busi_type();
	void parse_area_type();
	void parse_sett_period();

	void compu_fee();
	long long get_group_fee(int id);
	long long get_singal_fee(const feerate_info* feerate);
	long long get_singal_group_fee(const feerate_info* feerate, int value);
	long long get_ts_fee_total(int id);
	int get_ts_duration(const wuya::datetime& begin, int duratin, int hour, int hour2);

	bool valid_201();

		
	std::string collect_source_;
	// ����Ҫ��
	int sett_type_;
	int busi_type_;
	call_info caller_;
	call_info called_;
	int area_type_;
	int local_type_;
	int toll_type_;
	int inet_toll_type_;
	int call_trans_;
	int call_orient_;
	// ����������Ϣ
	int sett_obj_;
	int sett_obj_type_;
	int sett_clsid_;
	int feerate_id_;
	long long sett_hop_;
	long long fee_;
	int currency_; 
	int debit_;
	int sett_obj_flag_;
	std::string sett_period_; 
};
#endif // __CLASSIFY_PROC_H__
